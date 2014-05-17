attribute vec2 a_position;
uniform vec4 v_color;
void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
}