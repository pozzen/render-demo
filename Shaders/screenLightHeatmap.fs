#version 430

uniform int lightCount;
uniform int tilesX;

layout (std430, binding = 1) readonly buffer VisibleLightBuffer
{
	uint indices[];
} visibleLightBuffer;

in vec2 texCoord0;

out vec4 color;

void main()
{
	ivec2 tileIndex = ivec2(gl_FragCoord.xy / vec2(16.0, 16.0));
	uint location = tileIndex.y * tilesX + tileIndex.x;
	uint offset = location * 1024;

	vec3 result = vec3(0.0);

	uint lastInd = offset + 1023;
	for (uint i = offset; i <= lastInd && visibleLightBuffer.indices[i] != -1; i++)
	{
		result += vec3(1.0) / float(lightCount);
	}

	color = vec4(result, 1.0);
}
