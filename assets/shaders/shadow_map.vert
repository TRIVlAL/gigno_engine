#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(push_constant) uniform Push {
	mat4 model;
} push;

layout(binding=0) uniform UniformBufferObject {
	mat4 view;
	mat4 projection;
} ubo;

void main() {
	vec3 TEST = inColor + inNormal;
	TEST.x += 5.0f;
	gl_Position = ubo.projection * ubo.view * vec4(push.model * vec4(inPos.xyz, 1.0));
}