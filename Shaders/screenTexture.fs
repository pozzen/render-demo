#version 430

uniform sampler2D screenTexture;

in vec2 texCoord0;

out vec4 color;

void main()
{
	color = vec4(texture2D(screenTexture, texCoord0).rgb, 1.0);
}
