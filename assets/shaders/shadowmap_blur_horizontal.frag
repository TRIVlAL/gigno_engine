#version 450

layout(location=0) out vec2 outColor;

layout(binding=0) uniform sampler2D image;

const float SCALE = 0.7f;
const float SHADOW_MAP_WIDTH = 2000.0f;
const float SHADOW_MAP_HEIGHT = 2000.0f;

void main() {
    vec2 color = vec2(0.0f);

    vec2 coord = gl_FragCoord.xy;
    coord.x = coord.x / SHADOW_MAP_WIDTH;
    coord.y = coord.y / SHADOW_MAP_HEIGHT;

    //See @ https://www.youtube.com/watch?v=mb7WuTDz5jw

    float factor = (SCALE/SHADOW_MAP_WIDTH);

    color += texture(image, coord + factor * vec2(-3, 0)).xy * (1.0/64.0);
    color += texture(image, coord + factor * vec2(-2, 0)).xy * (6.0/64.0);
    color += texture(image, coord + factor * vec2(-1, 0)).xy * (15.0/64.0);
    color += texture(image, coord + factor * vec2(0, 0)).xy * (20.0/64.0);
    color += texture(image, coord + factor * vec2(1, 0)).xy * (15.0/64.0);
    color += texture(image, coord + factor * vec2(2, 0)).xy * (6.0/64.0);
    color += texture(image, coord + factor * vec2(3, 0)).xy * (1.0/64.0);

    outColor = color;
}