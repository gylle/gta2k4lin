#version 100

attribute vec3 a_position;
attribute vec2 a_texcoord;

void main(void)
{
    gl_TexCoord[0].xy = a_texcoord;
    gl_Position = vec4(a_position, 1.0);
}
