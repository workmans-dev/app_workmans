#version 450

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_Color;

// WARNING: must be literally identical to the Push block in the vert. Only u_EntityID is
//    read here, but trimming the block makes the two stages' layouts diverge.
layout(push_constant) uniform Push {
    mat4 u_MVP;
    mat4 u_Model;
    int  u_EntityID;
} pc;

layout(location = 0) out vec4 o_Color;
// P-a -- goes to the render target's second color attachment (R32_SINT).
//    "The pixel actually drawn is the answer", so alpha, discard and modifier results all
//    come out right automatically, and curves/NURBS get picked up the same way.
layout(location = 1) out int  o_EntityID;

void main() {
    vec3  N       = normalize(v_Normal);
    vec3  L       = normalize(vec3(0.5, 1.0, 0.8));
    float diff    = max(dot(N, L), 0.0);
    vec3  ambient = vec3(0.25);
    o_Color    = vec4(v_Color * (ambient + diff * 0.85), 1.0);
    o_EntityID = pc.u_EntityID;
}