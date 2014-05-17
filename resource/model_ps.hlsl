#include "common.hlsl"

// pixel shader
float4 PS_Main( VS_MODEL_OUT In ) : SV_TARGET
{
	float3 nl = saturate(dot(In.normal, -In.light_direction));
	float3 reflection = normalize(2 * nl * In.normal + In.light_direction);
	// todo
	float3 view_direction = float3 (0, 0, 0);

	float4 diffuse = material.diffuse;
	if (material.shader_flags.y > 0)
	{
		return material.diffuse;
	}
	if (material.shader_flags.x > 0)
	{
		diffuse = g_texture.Sample( g_sampler, In.uv );
	}
	return phong(material, diffuse, nl, reflection, view_direction);
}
