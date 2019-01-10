#version 430

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

uniform vec3 ambient;
uniform vec3 cameraPosition;
uniform int lightCount;
uniform int tilesX;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

in vec2 texCoord0;
in vec3 fragPosition0;
in mat3 TBN;

void main()
{
	vec3 diffuseColor = texture2D(texture_diffuse1, texCoord0).xyz;
	vec3 specularColor = texture2D(texture_specular1, texCoord0).xyz;
	vec3 normalColor = normalize(texture2D(texture_normal1, texCoord0).xyz * 2.0 - 1.0);

	vec3 viewDir = normalize(cameraPosition - fragPosition0);
	vec3 normal = normalize(TBN * normalColor);

    gPosition = fragPosition0.xyz;
    gNormal = normal;
    gAlbedoSpec.rgb = diffuseColor;
    gAlbedoSpec.a = specularColor.r;
}
