#version 430
// See https://github.com/bcrusco/Forward-Plus-Renderer/blob/master/Forward-Plus/Forward-Plus/source/shaders/light_culling.comp.glsl

struct Light
{
	vec4 positionRadius;
	vec4 colorSpec;
};

layout (std430, binding = 0) readonly buffer LightBuffer
{
	Light lights[];
} lightBuffer;

layout (std430, binding = 1) writeonly buffer VisibleLightBuffer
{
	uint indices[];
} visibleLightBuffer;

uniform sampler2D depthMap;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 screenSize;
uniform int lightCount;
uniform float zNear;
uniform float zFar;

shared uint minDepth;
shared uint maxDepth;
shared vec4 frustumPlanes[6];

shared uint visibleLights[1024];
shared uint nVisibleLights;

#ifndef WORKGROUP_SIZE
	#define WORKGROUP_SIZE 16
#endif

layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1) in;

void main()
{
	uint location = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
	uint offset = location * 1024;
	uvec2 nTiles = (uvec2(screenSize) + uvec2(WORKGROUP_SIZE - 1)) / uvec2(screenSize);

	if (gl_LocalInvocationIndex == 0)
	{
		minDepth = 0xFFFFFFFF;
		maxDepth = 0;
		nVisibleLights = 0;
	}

	barrier();

	// Calculate min and max depth
	float depth = texture2D(depthMap, gl_GlobalInvocationID.xy / screenSize).r;
	depth = (0.5 * projection[3][2]) / (depth + 0.5 * projection[2][2] - 0.5);

	atomicMin(minDepth, floatBitsToUint(depth));
	atomicMax(maxDepth, floatBitsToUint(depth));

	barrier();

	// Calculate frustum planes
	if (gl_LocalInvocationIndex == 0)
	{
		float depthMin = uintBitsToFloat(minDepth);
		float depthMax = uintBitsToFloat(maxDepth);

		// Calculate scale and bias
		vec2 tileScale = screenSize / float(2 * WORKGROUP_SIZE);
		vec2 tileBias = tileScale - vec2(gl_WorkGroupID.xy);

	    vec4 col1 = vec4(-projection[0][0] * tileScale.x, projection[0][1], tileBias.x, projection[0][3]);
	    vec4 col2 = vec4(projection[1][0], -projection[1][1] * tileScale.y, tileBias.y, projection[1][3]);
	    vec4 col4 = vec4(projection[3][0], projection[3][1],  -1.0f, projection[3][3]);

		frustumPlanes[0] = col4 + col1; // Left plane
		frustumPlanes[1] = col4 - col1; // Right plane
		frustumPlanes[2] = col4 - col2; // Top plane
		frustumPlanes[3] = col4 + col2; // Bottom plane
		frustumPlanes[4] = vec4(0.0f, 0.0f, -1.0f, -depthMin); // Near plane
		frustumPlanes[5] = vec4(0.0f, 0.0f, 1.0f, depthMax); // Far plane

		// Normalize side planes
		for(int i = 0; i < 4; i++)
		{
			frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}
	}

	barrier();

	uint threadCount = WORKGROUP_SIZE * WORKGROUP_SIZE;
	uint passCount = (lightCount + threadCount - 1) / threadCount;

	for (uint i = 0; i < passCount; i++)
	{
		uint index = i * threadCount + gl_LocalInvocationIndex;
		if (index > lightCount) break;

		vec4 position = view * vec4(lightBuffer.lights[index].positionRadius.xyz, 1.0);
		float radius = lightBuffer.lights[index].positionRadius.w;

		float distance = 0.0;
		for (int j = 0; j < 6; j++)
		{
			distance = dot(position, frustumPlanes[j]) + radius;
			if (distance < 0.0) break;
		}

		if (distance >= 0.0)
		{
			uint visibleLightIndex = atomicAdd(nVisibleLights, 1);
			if (visibleLightIndex >= 1024) break;
			visibleLights[visibleLightIndex] = index;
		}
	}

	barrier();

	if (gl_LocalInvocationIndex == 0)
	{
		for (uint i = 0; i < nVisibleLights; i++)
		{
			visibleLightBuffer.indices[offset + i] = visibleLights[i];
		}

		if (nVisibleLights < 1024)
		{
			visibleLightBuffer.indices[offset + nVisibleLights] = -1;
		}
	}
}
