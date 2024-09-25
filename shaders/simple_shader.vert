#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outColor;

layout(push_constant) uniform Push {
	mat4 model;
	mat4 normalMatrix;
} push;

const int MAX_LIGHT_DATA_COUNT = 10; //MAX_LIGHT_DATA_COUNT hard-coded for now to reflect the one in swapchain.h

layout(binding=0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
	vec4 lightDatas[MAX_LIGHT_DATA_COUNT]; 
} ubo;

float environment = 0.02;

void main() {
	vec4 worldPos = push.model * vec4(inPosition, 1.0);
	gl_Position = ubo.projection * ubo.view * push.model * vec4(inPosition, 1.0);

	float lightPower = 0.0f;
	for(int i = 0; i < MAX_LIGHT_DATA_COUNT; i++) {
		if(ubo.lightDatas[i].w == 1.0f) { // DIRECTIONAL LIGHT
			lightPower += max(dot(mat3(push.normalMatrix) * inNormal, vec3(ubo.lightDatas[i])), 0);
		} 
		else if(ubo.lightDatas[i].w == 2.0f)  { // POINT LIGHT
			vec3 meToLight = vec3(ubo.lightDatas[i]) - vec3(worldPos);
			float distanceSquared = meToLight.x * meToLight.x + meToLight.y * meToLight.y + meToLight.z * meToLight.z + 0.00001f;
			meToLight = normalize(meToLight);
			i++;
			lightPower += max(dot(mat3(push.normalMatrix) * inNormal, meToLight), 0) / distanceSquared * ubo.lightDatas[i].x;
		}
		else{ //As more light types get introduces, we will check for them here and handle them accordingly.
			break;
		}
	}

	outColor = inColor * (environment + lightPower);
}