#version 300 es
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inVertexPosition;
uniform vec4 transformValues;
uniform mat4 viewMatrix;

void main() {
    vec2 adjustedPosition = (inVertexPosition.xy * transformValues.zw) + transformValues.xy;
    gl_Position = viewMatrix * vec4(adjustedPosition, 0.0, 1.0);
}
