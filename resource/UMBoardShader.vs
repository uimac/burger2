#version 100
attribute vec3 a_position;
attribute vec2 a_uv;
varying vec2 uv;

void main()
{
    uv = a_uv;
    gl_Position = vec4(a_position, 1.0);
}
