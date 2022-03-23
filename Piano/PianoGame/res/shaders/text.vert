#version 300 es
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inVertexPosition;
uniform mat4 screenspaceTransform;

void main() {
    gl_Position = screenspaceTransform * vec4(inVertexPosition, 1.0);
}
