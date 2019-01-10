#version 430

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 position0;
out vec2 texCoord0;

void main()
{
	gl_Position = vec4(position.xy, 0.0, 1.0);
	position0 = position;
	texCoord0 = texCoord;
}
