#version 140
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
uniform mat4 view_projection_matrix;
out vec2 uv;
void main()
{
	vec4 pos = view_projection_matrix * vec4(a_position, 1.0);
	pos.z = 2.0 * pos.z - pos.w;
	uv = a_uv;
	gl_Position = pos;
}
