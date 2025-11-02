#version 450

layout(location = 0) in vec4 i_Position;
layout(location = 1) in vec3 i_Color;
layout(location = 2) in vec2 i_TexCoords;


layout(location = 0) out vec3 color;
layout(location = 1) out vec2 texCoords;


layout(binding = 0) uniform Matrices
{
	mat4 projMat;
	mat4 viewMat;
	mat4 modelMat;
};

void main()
{
	gl_Position = projMat * viewMat * modelMat *i_Position;
	color = i_Color;
	texCoords = i_TexCoords;
}