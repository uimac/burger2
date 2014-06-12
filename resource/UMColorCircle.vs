#version 100
attribute vec3 a_position;
attribute vec2 a_uv;
uniform mat4 view_projection_matrix;
varying vec2 uv;
void main()
{
	vec4 pos = view_projection_matrix * vec4(a_position, 1.0);
	pos.z = 2.0 * pos.z - pos.w;
	uv = a_uv;
	gl_Position = pos;
}
