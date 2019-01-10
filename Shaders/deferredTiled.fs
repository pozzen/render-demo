#version 430
#include "light.in"

uniform vec3 cameraPosition;
uniform int lightCount;
uniform int tilesX;
uniform vec2 screenSize;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D depthMap;

layout (std430, binding = 1) readonly buffer VisibleLightBuffer
{
	uint indices[];
} visibleLightBuffer;

in vec2 position0;
in vec2 texCoord0;
out vec4 color;

void main()
{
	if (texture(depthMap, texCoord0).r >= 0.99999) discard;

	// We can't use gl_FragCoord because it will mess up tile positions
	// when using glViewport to render only to part of the screen
	ivec2 tileIndex = ivec2((position0 * 0.5 + 0.5) * screenSize / vec2(16.0, 16.0));
	uint location = tileIndex.y * tilesX + tileIndex.x;
	uint offset = location * 1024;

    vec3 fragPos = texture(gPosition, texCoord0).rgb;
    vec3 normal = texture(gNormal, texCoord0).rgb;
    vec4 albedoSpec = texture(gAlbedoSpec, texCoord0);

	vec3 viewDir = normalize(cameraPosition - fragPos);

	vec3 result = vec3(0.0);

	uint lastInd = offset + 1023;
	for (uint i = offset; i <= lastInd && visibleLightBuffer.indices[i] != -1; i++)
	{
		result += calcLight(lightBuffer.lights[visibleLightBuffer.indices[i]], albedoSpec.rgb, albedoSpec.a, normal, viewDir, fragPos);
	}

	color = vec4(result, 1.0);
}
