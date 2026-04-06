#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

layout(set = 1, binding = 0) uniform PushConstants {
    float x_scale;
    float y_scale;
    float x_offset;
    float y_offset;
} pc;

void main() {
    gl_Position = vec4(inPos.x * pc.x_scale + pc.x_offset, inPos.y * pc.y_scale + pc.y_offset, 0.0, 1.0);
    outUV = inUV;
    outColor = inColor;
}
