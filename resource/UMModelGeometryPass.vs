#version 140
invariant gl_Position;
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
out vec3 position;
out vec3 normal;
out vec2 uv;
uniform mat4 view_projection_matrix;

void main()
{
	position = a_position;
	normal =  a_normal;
	uv = a_uv;
	vec4 pos = view_projection_matrix * vec4(a_position, 1.0);
	pos.z = 2.0 * pos.z - pos.w;
	gl_Position = pos;
}
