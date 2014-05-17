
struct DirectionalLight
{
	float4 position;
	float4 color;
	float4 ambient_color;
};

struct Material
{
	float4 diffuse;
	// .w is specular_factor;
	float4 specular;
	float4 ambient;
	float4 shader_flags;
};

// world x view x projection
cbuffer ConstantBuffer : register( b0 )
{
	matrix world_view_projection;
	matrix world_view;
};

cbuffer ConstantBuffer2 : register( b1 )
{
	DirectionalLight light;
};

cbuffer ConstantBuffer3 : register( b2 )
{
	Material material;
};

Texture2D g_texture : register( t0 );
SamplerState g_sampler : register( s0 );

// vertex shader input
struct VS_IN
{
	float3 pos : POSITION;   // vertex position
	float2 uv : TEXCOORD;   // texture
};

// vertex shader output
struct VS_OUT
{
	float4 pos   : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// vertex shader input
struct VS_MODEL_IN
{
	float3 pos : POSITION;   // vertex position
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;   // texture
	//float4 color : COLOR;      // vertex color
};

// vertex shader output
struct VS_MODEL_OUT
{
	float4 pos   : SV_POSITION;
	float4 normal : NORMAL;
	float4 color : COLOR0;
	float4 light_direction: TEXCOORD0;
	float2 uv : TEXCOORD1;
};

float4 phong(Material mat, float4 diff, float3 NL, float3 R, float3 V)
{
	float4 ambient = mat.ambient * light.ambient_color * float4(1, 1, 1, 0);
	float4 diffuse = diff * float4(NL, 1);
	float4 specular = float4(mat.specular.xyz, 0) * pow( saturate( dot(R,V) ), mat.specular.w );
	return ambient + (diffuse + specular) * light.color;
}


