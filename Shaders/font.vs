#version 430

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 texCoord0;

uniform vec2 screenSize;
uniform vec2 texSize;

void main()
{
	vec2 posNegY = vec2(position.x, -position.y);

	gl_Position = vec4(posNegY * 2.0 / screenSize - vec2(1.0, -1.0), 0.0, 1.0);
	texCoord0 = texCoord/texSize;
}
