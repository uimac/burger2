#version 100
#ifdef GL_ES
precision mediump float;
#endif
//out vec4 frag_color;
varying vec3 normal;
varying vec2 uv;
varying vec4 light_direction;
uniform sampler2D s_texture;

uniform vec4 light_color;
uniform vec4 light_ambient_color;

uniform vec4 constant_color;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;
uniform vec4 mat_ambient;
uniform vec4 mat_flags;

vec4 phong(vec4 diff, vec3 NL, vec3 R, vec3 V)
{
    vec4 ambient = mat_ambient * light_ambient_color * vec4(1.0, 1.0, 1.0, 0.0);
    vec4 diffuse = diff * vec4(NL.x, NL.y, NL.z, 1.0);
    vec4 specular = vec4(mat_specular.x, mat_specular.y, mat_specular.z, 0.0) 
                    * pow( max(0.00001, clamp( dot(R,V), 0.0, 1.0 )), mat_specular.w );
    return ambient + (diffuse + specular) * light_color;
}

void main()
{
    vec3 L = vec3(light_direction.x, light_direction.y, light_direction.z);
    float nl = clamp( dot(normal, -L ), 0.0, 1.0);
    vec3 vnl = vec3(nl, nl, nl);
    vec3 reflection = normalize(2.0 * nl * normal + L);

    // todo
    vec3 view_direction = vec3(0.0, 0.0, 0.0);

    vec4 diffuse = mat_diffuse;
    if (mat_flags.y > 0.0)
    {
        gl_FragColor = constant_color;
        return;
    }
    if (mat_flags.x > 0.0)
    {
        diffuse = texture2D(s_texture, uv);
    }
    vec4 out_color = phong(diffuse, vnl, reflection, view_direction);
    out_color.w = diffuse.w;
    gl_FragColor = out_color;
}
