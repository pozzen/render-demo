#version 430
#include "light.in"

uniform vec3 cameraPosition;
uniform int lightCount;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

in vec2 texCoord0;
in vec3 fragPosition0;
in mat3 TBN;

out vec4 color;

void main()
{
	vec3 diffuseColor = texture2D(texture_diffuse1, texCoord0).xyz;
	float specularIntensity = texture2D(texture_specular1, texCoord0).r;
	vec3 normalColor = normalize(texture2D(texture_normal1, texCoord0).xyz * 2.0 - 1.0);

	vec3 viewDir = normalize(cameraPosition - fragPosition0);
	vec3 normal = normalize(TBN * normalColor);

	vec3 result = vec3(0.0);

	for (uint i = 0; i <= lightCount; i++)
	{
		result += calcLight(lightBuffer.lights[i], diffuseColor, specularIntensity, normal, viewDir, fragPosition0);
	}

	color = vec4(result, 1.0);
}
