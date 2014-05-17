#version 140
#ifdef GL_ES
precision mediump float;
#endif
out vec4 frag_color;
in vec2 uv;

uniform vec4 light_position;
uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D diffuse_texture;

//uniform sampler2D uv_texture;
//uniform sampler2D s_texture;
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const int numLights = 6;
const vec3 lightColors[numLights] = vec3[numLights](
    vec3(0.5, 0.5, 0.5),
    vec3(0.8, 0.3, 0.3),
    vec3(0.3, 0.3, 0.8),
    vec3(0.3, 0.2, 0.0),
    vec3(0.0, 0.0, 0.5),
    vec3(0.0, 0.6, 0.2)
);

const vec3 lightPositions[numLights] = vec3[numLights](
    vec3(10.0, 10.0, 10.0),
    vec3(-30.0, -20.0, 20.0),
    vec3( 40.0, -00.0, -10.0),
    vec3( 10.0, -10.0,  10.0),
    vec3( 50.0, 00.0, 60.0),
    vec3( 40.0, 0.0,  20.0)
);

void main()
{
    vec4 position = texture2D(position_texture, uv);
    vec3 normal = texture2D(normal_texture, uv).xyz;
    vec4 diffuse = texture2D(diffuse_texture, uv);
    frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < numLights; ++i)
    {
        //vec4 light_position = light_position + vec4(rand(uv), rand(uv), rand(uv), 0.0);
        vec3 L = normalize( lightPositions[i] - position.xyz);
        float nl = clamp( dot(normal, L ), 0.0, 1.0);
        float attenution = 1.0 / length( L );
        vec3 light_color = lightColors[i];
        frag_color.xyz += attenution * light_color * diffuse.xyz * nl;
    }
}
