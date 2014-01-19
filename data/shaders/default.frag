#version 100

uniform sampler2D texture1;

void main(void)
{
    gl_FragColor = texture2D(texture1, vec2(gl_TexCoord[0]));
}
