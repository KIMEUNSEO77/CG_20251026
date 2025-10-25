#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;   // ← VAO와 일치

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vColor;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // 노멀로 임시 컬러 만들기(시각 확인용)
    vColor = normalize(aNormal) * 0.5 + 0.5; // [-1,1] → [0,1]
}