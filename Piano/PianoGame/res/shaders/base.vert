
#version 300 es
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 aPos;
uniform mat4 mvp;

void main()
{
  gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
