#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;

// The first cube goes through push constants (the minimal path, no UBO/descriptor set)
// WARNING: must be literally identical to the Push block in the frag -- both stages see the same thing.
layout(push_constant) uniform Push {
    mat4 u_MVP;         //   0..127
    mat4 u_Model;
    int  u_EntityID;    // 128..131   for picking. Only the frag reads it
} pc;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_Color;

void main() {
    gl_Position = pc.u_MVP * vec4(a_Position, 1.0);
    v_Normal    = mat3(pc.u_Model) * a_Normal;
    v_Color     = a_Color;
}