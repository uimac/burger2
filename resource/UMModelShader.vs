#version 140
invariant gl_Position;
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
out vec3 normal;
out vec2 uv;
out vec4 light_direction;
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
