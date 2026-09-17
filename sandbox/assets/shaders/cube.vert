#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;

// 🟢 첫 큐브는 push constant로 (UBO/디스크립터셋 없이 최소 경로)
// ⚠️ frag의 Push 블록과 글자 그대로 같아야 한다 — 두 스테이지가 같은 걸 본다.
layout(push_constant) uniform Push {
    mat4 u_MVP;         //   0..127
    mat4 u_Model;
    int  u_EntityID;    // 128..131   피킹용. frag만 읽는다
} pc;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_Color;

void main() {
    gl_Position = pc.u_MVP * vec4(a_Position, 1.0);
    v_Normal    = mat3(pc.u_Model) * a_Normal;
    v_Color     = a_Color;
}