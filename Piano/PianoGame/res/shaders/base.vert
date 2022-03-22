#version 300 es
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPosition;
uniform mat4 worldspaceTransform;
uniform mat4 viewMatrix;

void main() {
    gl_Position = viewMatrix * worldspaceTransform * vec4(inPosition, 1.0);
}
