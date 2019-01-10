#version 430

uniform sampler2D depthMap;
uniform float zNear;
uniform float zFar;

in vec2 texCoord0;

out vec4 color;

void main()
{
	float depth = texture2D(depthMap, texCoord0).x;
	depth = 2.0 * zNear / (zFar + zNear - depth * (zFar - zNear));
	color = vec4(depth);
}
