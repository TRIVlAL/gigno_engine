#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outColor;

layout(push_constant) uniform Push {
	mat4 transform;
	mat4 normalMatrix;
} push;

vec3 lightDir = normalize(vec3(-1.0, 3.0, -1.0));
float environment = 0.02;

void main() {
	gl_Position = push.transform * vec4(inPosition, 1.0);

	outColor = inColor * (environment + max(dot(mat3(push.normalMatrix) * inNormal, lightDir), 0));
	//outColor = mat3(push.normalMatrix) * inNormal;
}