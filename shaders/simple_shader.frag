#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPos;
layout(location = 2) in vec3 inNormal;

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
} ubo;

void main() {


	float lightPower = 0.0f;
	if(push.fullbright == 1) 
	{
		lightPower = 1.0f;
	} 
	else 
	{
		vec3 Normal = normalize(inNormal);	

		for(int i = 0; i < MAX_LIGHT_DATA_COUNT; i++) 
		{
			if(ubo.lightDatas[i].w == 1.0f) // DIRECTIONAL LIGHT
			{
				lightPower += max(dot(Normal, vec3(ubo.lightDatas[i])), 0);
			} 
			else if(ubo.lightDatas[i].w == 2.0f)  // POINT LIGHT 
			{ 
				vec3 meToLight = vec3(ubo.lightDatas[i]) - vec3(inPos);
				float distanceSquared = meToLight.x * meToLight.x + meToLight.y * meToLight.y + meToLight.z * meToLight.z + 0.00001f;
				meToLight = meToLight / sqrt(distanceSquared);
				i++;
				lightPower += max(dot(Normal, meToLight), 0) / distanceSquared * ubo.lightDatas[i].x;
			}
			else if(ubo.lightDatas[i].w == 3.0f) // ENVIRONMENT LIGHT
			{
				lightPower += ubo.lightDatas[i].x;
			}
			else 
			{ 
				//As more light types get introduces, we will check for them here and handle them accordingly.
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