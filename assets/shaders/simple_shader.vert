#version 450

/*
MAX_LIGHT_DATA_COUNT hard-coded for now to reflect the one in swapchain.h
same for SHADOW_MAP_CASCADE_COUNT
*/
const int MAX_LIGHT_DATA_COUNT = 15;
const int SHADOW_MAP_CASCADE_COUNT = 3;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outFragPos;
layout(location = 2) out vec3 outFragNormal;
layout(location = 3) out vec4 outFragLightSpacePos[SHADOW_MAP_CASCADE_COUNT];

layout(push_constant) uniform Push {
	mat4 model;
	mat4 normalMatrix;
	int fullbright;
} push;


layout(binding=0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;

	vec4 lightDatas[MAX_LIGHT_DATA_COUNT];

	//SHadow Map Transformations
	mat4 LightView[SHADOW_MAP_CASCADE_COUNT];
	mat4 LightProjection[SHADOW_MAP_CASCADE_COUNT];
} ubo;

void main() {
	vec4 worldPos = push.model * vec4(inPosition, 1.0);
	gl_Position = ubo.projection * ubo.view * worldPos;

	outFragNormal = normalize(mat3(push.normalMatrix) * inNormal);
	outFragPos = worldPos.xyz;
	outColor = inColor;

	for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		outFragLightSpacePos[i] = ubo.LightProjection[i] * ubo.LightView[i] * worldPos;
	}

	gl_PointSize = 5.0f;
}