#version 140
invariant gl_Position;
in  vec3 a_position;
uniform mat4 view_projection_matrix;

void main()
{
	gl_Position = view_projection_matrix * vec4(a_position, 1.0);
}