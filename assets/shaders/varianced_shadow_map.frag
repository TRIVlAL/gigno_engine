#version 450

const float SHADOW_MAP_WIDTH = 2000;
const float SHADOW_MAP_HEIGHT = 2000;

layout(location=0) out vec2 outColor;

layout(binding=0) uniform sampler2D shadowMapDepth;

void main() {
    vec2 coord = gl_FragCoord.xy;
    coord.x = coord.x/SHADOW_MAP_WIDTH;
    coord.y = coord.y/SHADOW_MAP_HEIGHT;

    float d = texture(shadowMapDepth, coord).x;

    outColor = vec2(d, d*d);
}