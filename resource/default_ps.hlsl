#include "common.hlsl"

// pixel shader
float4 PS_Main( VS_OUT In ) : SV_TARGET
{
	return g_texture.Sample( g_sampler, In.uv );
}
