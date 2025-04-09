#version 450

/*
MAX_LIGHT_DATA_COUNT hard-coded for now to reflect the one in swapchain.h
same for SHADOW_MAP_CASCADE_COUNT
*/
const int MAX_LIGHT_DATA_COUNT = 15;
const int SHADOW_MAP_CASCADE_COUNT = 3;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inLightSpacePos[SHADOW_MAP_CASCADE_COUNT];

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 model;
	mat4 normalMatrix;
} push;

layout(binding=0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	vec4 lightDatas[MAX_LIGHT_DATA_COUNT];

	mat4 LightView[SHADOW_MAP_CASCADE_COUNT];
	mat4 LightProjection[SHADOW_MAP_CASCADE_COUNT];

	int Parameters;
} ubo;

const int SHADOW_MAP_WIDTH = 2500;
const int SHADOW_MAP_HEIGHT = 2500;

layout(binding=1) uniform sampler2D variancedShadowMap[SHADOW_MAP_CASCADE_COUNT];

float ShadowMappedDirectionalLightPower(int index, vec3 positionLightSpaceNDC) {

	vec2 shadow_map_coord = positionLightSpaceNDC.xy * 0.5 + 0.5;
	
	vec2 moments = texture(variancedShadowMap[index], shadow_map_coord.xy).xy;

	float distance = moments.x - positionLightSpaceNDC.z;

	float variance = max(moments.y - moments.x * moments.x, 0.00002f);
	float pMax = variance / (variance + distance * distance);

	if(positionLightSpaceNDC.z < moments.x) {
		return 1.0f;
	} else {
		return pMax;
	}
}

float CascadedShadowMapLightPower(vec3 frag_normal, vec3 lightDir, bool doDebugRange) {

	int inside_shadow_map_index_one = -1;
	int inside_shadow_map_index_two = -1;

	float multiplier = 1.0f; // 0 if in shadow.

	for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		vec3 pos_light_space_ndc = inLightSpacePos[i].xyz / inLightSpacePos[i].w;

		if (abs(pos_light_space_ndc.x) < 0.7f && abs(pos_light_space_ndc.y) < 0.7f && abs(pos_light_space_ndc.z) < 0.7f) {

			multiplier = ShadowMappedDirectionalLightPower(i, pos_light_space_ndc);

			inside_shadow_map_index_one = i;
			inside_shadow_map_index_two = i;
			break;

		} else if( abs(pos_light_space_ndc.x) < 1.f && abs(pos_light_space_ndc.y) < 1.f && abs(pos_light_space_ndc.z) < 1.f) {
			//interpolate between this shadowmap and the next
			if(i < SHADOW_MAP_CASCADE_COUNT-1) {
				vec3 pos_light_space_ndc_second = inLightSpacePos[i+1].xyz / inLightSpacePos[i+1].w;
				
				if(abs(pos_light_space_ndc_second.x) < 1.0f && abs(pos_light_space_ndc_second.y) < 1.0f && abs(pos_light_space_ndc_second.z) < 1.0f) {
					
					float t = max(abs(pos_light_space_ndc.x), max(pos_light_space_ndc.y, pos_light_space_ndc.z));
					t = (t - 0.7f) / 0.7f;

					multiplier = (1-t)*ShadowMappedDirectionalLightPower(i, pos_light_space_ndc)
								+ t*ShadowMappedDirectionalLightPower(i+1, pos_light_space_ndc_second);

					inside_shadow_map_index_one = i;
					inside_shadow_map_index_two = i+1;
				} else {
					ShadowMappedDirectionalLightPower(i, pos_light_space_ndc);
				}
 
			} else {
				multiplier = ShadowMappedDirectionalLightPower(i, pos_light_space_ndc);

				inside_shadow_map_index_one = inside_shadow_map_index_two = i;
			}

			break;

		} else {
			continue;
		}
	}

	multiplier = max(multiplier, 0.1f);

	if(doDebugRange && inside_shadow_map_index_one != -1) {
		outColor = vec4(((inside_shadow_map_index_one + inside_shadow_map_index_two)*0.5f)/SHADOW_MAP_CASCADE_COUNT, 0.0f, 0.0f, 1.0f);
		return -1.0f;
	}

	return max(dot(frag_normal, vec3(lightDir)), 0) * multiplier;
}

void main() {

	int fullbright = 					(ubo.Parameters & 3);            //0b11
	int enable_shadow_map = 			(ubo.Parameters & 4) >> 2;      //0b100
	int shadow_map_do_debug_range = 	(ubo.Parameters & 8) >> 3;     //0b1000

	float lightPower = 0.0f;
	if(fullbright == 1) 
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
				if(i == 0 && enable_shadow_map == 1) {
					lightPower += CascadedShadowMapLightPower(Normal, ubo.lightDatas[i].xyz, shadow_map_do_debug_range == 1);
					if(shadow_map_do_debug_range == 1 && lightPower == -1.0f) {
						return;
					}
				} else {
					lightPower += max(dot(Normal, vec3(ubo.lightDatas[i])), 0);
				}

				i += 1;
			} 
			else if(light_type == 2.0f)  // POINT LIGHT 
			{ 
				vec3 meToLight = vec3(ubo.lightDatas[i]) - vec3(inPos);
				float distanceSquared = meToLight.x * meToLight.x + meToLight.y * meToLight.y + meToLight.z * meToLight.z + 0.00001f;
				meToLight = meToLight / sqrt(distanceSquared);
				lightPower += max(dot(Normal, meToLight), 0) / distanceSquared * ubo.lightDatas[i+1].x;

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

	if(fullbright == 2) {
		outColor = vec4(vec3(1.0f, 1.0f, 1.0f) * lightPower, 1.0f);
	} else {
		outColor = vec4(inColor * lightPower, 1.0f);
	}
}