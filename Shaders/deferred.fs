#version 430
#include "light.in"

uniform vec3 cameraPosition;
uniform int lightCount;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D depthMap;

in vec2 texCoord0;
out vec4 color;

void main()
{
	if (texture(depthMap, texCoord0).r >= 0.99999) discard;

    vec3 fragPos = texture(gPosition, texCoord0).rgb;
    vec3 normal = texture(gNormal, texCoord0).rgb;
    vec4 albedoSpec = texture(gAlbedoSpec, texCoord0);

	vec3 viewDir = normalize(cameraPosition - fragPos);

	vec3 result = vec3(0.0);

	for (uint i = 0; i <= lightCount; i++)
	{
		result += calcLight(lightBuffer.lights[i], albedoSpec.rgb, albedoSpec.a, normal, viewDir, fragPos);
	}

	color = vec4(result, 1.0);
}
