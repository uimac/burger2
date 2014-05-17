#ifdef GL_ES
precision mediump float;
#endif
uniform vec4 v_color;
void main()
{
	gl_FragColor = vec4(v_color);
}
