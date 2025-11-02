#version 450

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;

void main()
{
	fragColor = texture(tex,texCoords) * vec4(color,1);
}