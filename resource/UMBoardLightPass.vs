#version 140
invariant gl_Position;
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
out vec2 uv;

void main()
{
    uv = a_uv;
    gl_Position = vec4(a_position, 1.0);
}
