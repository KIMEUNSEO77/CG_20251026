#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Mesh
{
    GLuint vao = 0, vbo = 0;
    GLsizei count = 0; // number of vertices
};

// -------- 내부 유틸 --------
static inline void RemoveUTF8BOM(std::string& s) {
    if (s.size() >= 3 &&
        (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF)
    {
        s.erase(0, 3);
    }
}

struct Idx { int v = 0, vt = 0, vn = 0; };

// "v/vt/vn", "v//vn", "v/vt", "v" 를 파싱
static inline Idx ParseFaceToken(const std::string& tok) {
    Idx out{};
    int slash1 = (int)tok.find('/');
    if (slash1 == (int)std::string::npos) {
        out.v = std::stoi(tok);
        return out;
    }
    int slash2 = (int)tok.find('/', slash1 + 1);

    // v
    out.v = std::stoi(tok.substr(0, slash1));

    if (slash2 == (int)std::string::npos) {
        // v/vt
        std::string vt = tok.substr(slash1 + 1);
        if (!vt.empty()) out.vt = std::stoi(vt);
    }
    else {
        // v/vt/vn  또는 v//vn
        std::string vt = tok.substr(slash1 + 1, slash2 - slash1 - 1);
        std::string vn = tok.substr(slash2 + 1);
        if (!vt.empty()) out.vt = std::stoi(vt);
        if (!vn.empty()) out.vn = std::stoi(vn);
    }
    return out;
}

// 음수 인덱스 처리(OBJ 규격: 음수면 끝에서부터)
static inline int FixIndex(int idx, int n) {
    if (idx > 0) return idx;            // 1..n
    if (idx < 0) return n + idx + 1;    // -1 => n, -2 => n-1 ...
    return 0;
}

// -------- 메인 로더 --------
// - v, vn 사용 (vt는 무시)
// - face는 삼각형/다각형 모두 허용, 삼각형 팬으로 분해
// - vn이 없으면 면 법선으로 계산해서 채움(평면 셰이딩)
static bool LoadOBJ_PosNorm_Interleaved(const char* path, Mesh& out)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[OBJ] failed to open: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;  // 1-based
    std::vector<glm::vec3> normals;    // 1-based

    // 결과(확장형, 인덱스 없이 바로 interleave)
    std::vector<float> interleaved;
    interleaved.reserve(1 << 16);

    std::string line;
    bool firstLine = true;

    // 임시: 한 face 줄에서 토큰들을 모아 팬 분해
    auto emitTriangle = [&](const Idx& a, const Idx& b, const Idx& c) {
        // 인덱스 보정
        int va = FixIndex(a.v, (int)positions.size());
        int vb = FixIndex(b.v, (int)positions.size());
        int vc = FixIndex(c.v, (int)positions.size());
        int na = FixIndex(a.vn, (int)normals.size());
        int nb = FixIndex(b.vn, (int)normals.size());
        int nc = FixIndex(c.vn, (int)normals.size());

        if (va <= 0 || vb <= 0 || vc <= 0) return;

        glm::vec3 Pa = positions[va - 1];
        glm::vec3 Pb = positions[vb - 1];
        glm::vec3 Pc = positions[vc - 1];

        glm::vec3 Na, Nb, Nc;
        if (na > 0 && nb > 0 && nc > 0) {
            Na = normals[na - 1];
            Nb = normals[nb - 1];
            Nc = normals[nc - 1];
        }
        else {
            // vn 없음 → 면 법선(평면 셰이딩)
            glm::vec3 fn = glm::normalize(glm::cross(Pb - Pa, Pc - Pa));
            Na = Nb = Nc = fn;
        }

        auto push = [&](const glm::vec3& P, const glm::vec3& N) {
            interleaved.push_back(P.x); interleaved.push_back(P.y); interleaved.push_back(P.z);
            interleaved.push_back(N.x); interleaved.push_back(N.y); interleaved.push_back(N.z);
            };
        push(Pa, Na); push(Pb, Nb); push(Pc, Nc);
        };

    while (std::getline(ifs, line)) {
        if (firstLine) { RemoveUTF8BOM(line); firstLine = false; }
        if (line.empty()) continue;

        // 주석 제거
        if (line[0] == '#') continue;
        // 앞공백 스킵
        size_t s = 0; while (s < line.size() && std::isspace((unsigned char)line[s])) ++s;
        if (s >= line.size()) continue;

        std::istringstream iss(line.substr(s));
        std::string tag; iss >> tag;
        if (tag == "v") {
            glm::vec3 p; iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vn") {
            glm::vec3 n; iss >> n.x >> n.y >> n.z;
            normals.push_back(glm::normalize(n));
        }
        else if (tag == "f") {
            // f는 3개 이상 토큰 가능 → 삼각형 팬
            std::vector<Idx> vs;
            std::string tok;
            while (iss >> tok) {
                vs.push_back(ParseFaceToken(tok));
            }
            if (vs.size() < 3) continue;

            // 삼각형이면 그대로, 그 이상이면 (0,i-1,i)
            for (size_t i = 2; i < vs.size(); ++i) {
                emitTriangle(vs[0], vs[i - 1], vs[i]);
            }
        }
        // 그 외 태그는 무시 (vt, usemtl, mtllib, o, g, s ...)
    }

    // --- GL Buffer 업로드 ---
    glGenVertexArrays(1, &out.vao);
    glGenBuffers(1, &out.vbo);

    glBindVertexArray(out.vao);
    glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float),
        interleaved.data(), GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    // layout(location=1) vec3 aNormal;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6,
        (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    out.count = static_cast<GLsizei>(interleaved.size() / 6);
    if (out.count == 0) {
        std::cerr << "[OBJ] no triangles parsed: " << path << "\n";
        return false;
    }
    return true;
}