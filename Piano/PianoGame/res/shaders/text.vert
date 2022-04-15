#version 300 es
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 inVertexPosition;
uniform vec4 transformValues; // <vec2 pos, vec2 tex>
uniform mat4 viewMatrix;

out vec2 inTexCoords;

void main()
{
    vec2 finalPosition = (inVertexPosition.xy * transformValues.zw) + transformValues.xy;
    vec4 mirrored = (viewMatrix * vec4(finalPosition, 0.0, 1.0));
    mirrored.xy *= -1.0;
    gl_Position = mirrored;
    // inTexCoords = vertex.zw;
    inTexCoords = vec2(inVertexPosition.x, 1.0 - inVertexPosition.y);
}  