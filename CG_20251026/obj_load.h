#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Mesh 
{
    GLuint vao = 0, vbo = 0;
    GLsizei count = 0; // number of vertices
};

static bool LoadOBJ_PosNorm_Interleaved(const char* path, Mesh& out)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[OBJ] failed to open: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;

    // interleaved: px,py,pz, nx,ny,nz (per vertex)
    std::vector<float> interleaved;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string tag; iss >> tag;
        if (tag == "v") {
            glm::vec3 p; iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vn") {
            glm::vec3 n; iss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "f") {
            // Expect "a//a b//b c//c"
            for (int i = 0; i < 3; ++i) {
                std::string tok; iss >> tok;
                // split by "//"
                size_t p = tok.find("//");
                if (p == std::string::npos) {
                    std::cerr << "[OBJ] face format must be v//vn\n";
                    return false;
                }
                int vi = std::stoi(tok.substr(0, p));
                int ni = std::stoi(tok.substr(p + 2));
                // OBJ is 1-based
                glm::vec3 P = positions[vi - 1];
                glm::vec3 N = normals[ni - 1];
                interleaved.push_back(P.x); interleaved.push_back(P.y); interleaved.push_back(P.z);
                interleaved.push_back(N.x); interleaved.push_back(N.y); interleaved.push_back(N.z);
            }
        }
    }

    // GL buffer
    glGenVertexArrays(1, &out.vao);
    glGenBuffers(1, &out.vbo);

    glBindVertexArray(out.vao);
    glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    // layout(location=0) vec3 aPos;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    // layout(location=1) vec3 aNormal;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    out.count = static_cast<GLsizei>(interleaved.size() / 6);
    return true;
}