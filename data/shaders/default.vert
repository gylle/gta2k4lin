#version 100

uniform mat4 u_modelView;
uniform mat4 u_projection;

attribute vec3 a_position;
//attribute vec3 a_normal;
attribute vec2 a_texcoord;

void main(void)
{
    gl_TexCoord[0].xy = a_texcoord;
    gl_Position = u_projection * u_modelView * vec4(a_position, 1.0);
}
