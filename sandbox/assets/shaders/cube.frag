#version 450

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_Color;

// ⚠️ vert의 Push 블록과 글자 그대로 같아야 한다. 여기선 u_EntityID만 읽지만
//    블록을 잘라 쓰면 두 스테이지의 레이아웃이 어긋난다.
layout(push_constant) uniform Push {
    mat4 u_MVP;
    mat4 u_Model;
    int  u_EntityID;
} pc;

layout(location = 0) out vec4 o_Color;
// 🟢 P-a — 렌더타겟의 두 번째 색 어태치먼트(R32_SINT)로 간다.
//    "실제로 그려진 픽셀이 답"이라 알파·디스카드·모디파이어 결과까지 자동으로 맞고,
//    커브/NURBS처럼 지오메트리 종류가 갈려도 같은 방식으로 잡힌다.
layout(location = 1) out int  o_EntityID;

void main() {
    vec3  N       = normalize(v_Normal);
    vec3  L       = normalize(vec3(0.5, 1.0, 0.8));
    float diff    = max(dot(N, L), 0.0);
    vec3  ambient = vec3(0.25);
    o_Color    = vec4(v_Color * (ambient + diff * 0.85), 1.0);
    o_EntityID = pc.u_EntityID;
}