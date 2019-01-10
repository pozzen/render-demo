#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec2 texCoord0;
out vec3 fragPosition0;
out mat3 TBN;

uniform mat4 viewProjection;
uniform mat4 model;

void main()
{
	gl_Position = viewProjection * model * vec4(position, 1.0);
	texCoord0 = texCoord;
	fragPosition0 = (model * vec4(position, 1.0)).xyz;

	vec3 tangent1 = (normalize(model * vec4(tangent, 0.0))).xyz;
	vec3 bitangent1 = (normalize(model * vec4(bitangent, 0.0))).xyz;
	vec3 normal1 = (normalize(model * vec4(normal, 0.0))).xyz;

	TBN = mat3(tangent1, bitangent1, normal1);
}
