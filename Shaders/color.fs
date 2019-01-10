#version 430

uniform vec3 ambient;

out vec4 color;

void main()
{
	color = vec4(ambient, 1.0);
}
