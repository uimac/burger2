#version 140
#ifdef GL_ES
precision mediump float;
#endif
in vec3 position;
in vec3 normal;
in vec2 uv;
uniform sampler2D s_texture;

uniform vec4 constant_color;
uniform vec4 mat_diffuse;
uniform vec4 mat_flags;

out vec4 position_out; 
out vec4 diffuse_out;
out vec4 normal_out;
out vec4 uv_out; 

void main()
{
    diffuse_out = mat_diffuse;
    if (mat_flags.y > 0)
    {
        diffuse_out = constant_color;
    }
    if (mat_flags.x > 0)
    {
        diffuse_out = texture2D(s_texture, uv);
    }
    position_out = vec4(position, 1.0);
    normal_out = vec4(normalize(normal), 1.0);
    uv_out = vec4(uv, 0.0, 0.0);
}
