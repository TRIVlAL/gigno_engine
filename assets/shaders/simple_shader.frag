#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inLightSpacePos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 model;
	mat4 normalMatrix;
	int fullbright;
} push;

/*
MAX_LIGHT_DATA_COUNT hard-coded for now to reflect the one in swapchain.h
*/
const int MAX_LIGHT_DATA_COUNT = 15; 

layout(binding=0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	vec4 lightDatas[MAX_LIGHT_DATA_COUNT];

	mat4 LightView;
	mat4 LightProjection;
} ubo;

const int SHADOW_MAP_WIDTH = 2500;
const int SHADOW_MAP_HEIGHT = 2500;

layout(binding=1) uniform sampler2D shadowMap;

float ShadowMappedDirectionalLightPower(vec3 frag_normal, vec3 lightDir, vec3 lightPos, int addedSampleCount) {
	vec3 pos_light_space_ndc = inLightSpacePos.xyz / inLightSpacePos.w;

	float multiplier = 0.0f; // 0 if in shadow.

	if (abs(pos_light_space_ndc.x) > 1.0 || abs(pos_light_space_ndc.y) > 1.0 || abs(pos_light_space_ndc.z) > 1.0) {
		//Outside the shadowmap range => light it.
      	multiplier = 1.0f;
	} else {
		vec2 shadow_map_coord = pos_light_space_ndc.xy * 0.5 + 0.5;

		float shadow_map_pixel_width = 1.0f / SHADOW_MAP_WIDTH;
		float shadow_map_pixel_height = 1.0f / SHADOW_MAP_HEIGHT;

		int total_samples = (1 + 2 * addedSampleCount) * (1 + 2 * addedSampleCount);

		for(int x = -addedSampleCount; x <= addedSampleCount; x++) {
			for(int y = -addedSampleCount; y <= addedSampleCount; y++) {
				vec2 sample_coord = shadow_map_coord + vec2(x * shadow_map_pixel_width, y * shadow_map_pixel_height);
				float shadow_map_depth = texture(shadowMap, sample_coord.xy).x;
				if(pos_light_space_ndc.z <= shadow_map_depth) {
					multiplier += 1.0f;
				}
			}
		}


		multiplier = multiplier / total_samples;
	}

	multiplier = max(multiplier, 0.1f);

	return max(dot(frag_normal, vec3(lightDir)), 0) * multiplier;
}

void main() {

	float lightPower = 0.0f;
	if(push.fullbright == 1) 
	{
		lightPower = 1.0f;
	} 
	else 
	{
		vec3 Normal = normalize(inNormal);	

		int i = 0;
		while( i < MAX_LIGHT_DATA_COUNT) 
		{
			float light_type = ubo.lightDatas[i].w;

			if(light_type == 1.0f) // DIRECTIONAL LIGHT
			{
				if(i == 0) {
					lightPower += ShadowMappedDirectionalLightPower(Normal, ubo.lightDatas[i].xyz, ubo.lightDatas[i+1].xyz, 2);
				} else {
					lightPower += max(dot(Normal, vec3(ubo.lightDatas[i])), 0);
				}

				i += 2;
			} 
			else if(light_type == 2.0f)  // POINT LIGHT 
			{ 
				vec3 meToLight = vec3(ubo.lightDatas[i]) - vec3(inPos);
				float distanceSquared = meToLight.x * meToLight.x + meToLight.y * meToLight.y + meToLight.z * meToLight.z + 0.00001f;
				meToLight = meToLight / sqrt(distanceSquared);
				lightPower += max(dot(Normal, meToLight), 0) / distanceSquared * ubo.lightDatas[i].x;

				i += 2;
			}
			else if(light_type == 3.0f) // ENVIRONMENT LIGHT
			{
				lightPower += ubo.lightDatas[i].x;

				i += 1;
			} 
			else 
			{
				break;
			}
		}
	}

	if(push.fullbright == 2) {
		outColor = vec4(vec3(1.0f, 1.0f, 1.0f) * lightPower, 1.0f);
	} else {
		outColor = vec4(inColor * lightPower, 1.0f);
	}
}