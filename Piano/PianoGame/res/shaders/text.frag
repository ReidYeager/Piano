
#version 300 es
precision mediump float;

in vec2 inTexCoords;
uniform sampler2D inTextAtlas;
uniform vec3 inTextColor;

out vec4 outFragColor;

void main()
{
   outFragColor = vec4(inTextColor, texture(inTextAtlas, inTexCoords).r);
   // outFragColor = vec4(inTexCoords, 0.0, 1.0);
   // outFragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
