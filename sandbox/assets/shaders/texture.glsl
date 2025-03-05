#type vertex
#version 410 core
layout(location=0) in vec3 a_Position;
layout(location=1) in vec2 a_TexCoord;

uniform mat4 u_view_proj;
uniform mat4 u_transform;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_view_proj * u_transform * vec4(a_Position, 1.0);
}

#type fragment
#version 410 core
layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform float u_Tile;
void main()
{
	color = texture(u_Texture, v_TexCoord*u_Tile) * u_Color;
}
