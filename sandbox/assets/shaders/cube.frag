#version 450

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_Color;

layout(location = 0) out vec4 o_Color;

void main() {
    vec3  N       = normalize(v_Normal);
    vec3  L       = normalize(vec3(0.5, 1.0, 0.8));
    float diff    = max(dot(N, L), 0.0);
    vec3  ambient = vec3(0.25);
    o_Color = vec4(v_Color * (ambient + diff * 0.85), 1.0);
}