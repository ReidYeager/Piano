#version 300 es
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inVertexPosition;
uniform vec4 transformValues;
uniform mat4 viewMatrix;

void main() {
    vec4 gapTransform = vec4(transformValues.x + (transformValues.z * 0.05), transformValues.y, transformValues.z * 0.9, transformValues.w);
    vec2 adjustedPosition = vec2(inVertexPosition.x * gapTransform.z, -inVertexPosition.y * gapTransform.w) + gapTransform.xy;
    vec4 transformed = viewMatrix * vec4(adjustedPosition.x, adjustedPosition.y, 0.0, 1.0);
    gl_Position = vec4(-transformed.x, -transformed.y, transformed.zw);
}
