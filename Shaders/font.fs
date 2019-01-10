#version 430

uniform sampler2D texture;

in vec2 texCoord0;

out vec4 color;

void main()
{
	color = texture2D(texture, texCoord0);
}
