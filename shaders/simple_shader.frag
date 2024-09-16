#version 450

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 transform;
	mat4 normalMatrix;
} push;

void main() {
	outColor = vec4(inColor, 1.0);
}