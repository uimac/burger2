#version 100
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;
varying vec3 normal;
varying vec2 uv;
varying vec4 light_direction;
uniform mat4 view_projection_matrix;
uniform vec4 light_position;

void main()
{
	vec4 pos = view_projection_matrix * vec4(a_position, 1.0);
	normal =  a_normal;
	uv = a_uv;
	pos.z = 2.0 * pos.z - pos.w;
	light_direction = normalize(vec4(a_position, 0.0) - light_position);
	gl_Position = pos;
}
