#version 140
#ifdef GL_ES
precision mediump float;
#endif
out vec4 frag_color;
in vec2 uv;
uniform sampler2D s_texture;

void main()
{
    frag_color = texture2D(s_texture, uv);
}
