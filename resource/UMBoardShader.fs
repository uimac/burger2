#version 100
#ifdef GL_ES
precision mediump float;
#endif
//out vec4 frag_color;
varying vec2 uv;
uniform sampler2D s_texture;

void main()
{
    gl_FragColor = texture2D(s_texture, uv);
}
