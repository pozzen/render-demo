#version 430

uniform sampler2D screenTexture;
uniform float scale;

in vec2 texCoord0;

out vec4 color;

void main()
{
	color = vec4(abs(texture2D(screenTexture, texCoord0).rgb) * scale, 1.0);
}
