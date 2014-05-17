#include "common.hlsl"

// vertex shader
VS_MODEL_OUT VS_Main( VS_MODEL_IN In )
{
	VS_MODEL_OUT Out;
	
	Out.pos = mul( float4( In.pos, 1 ), world_view_projection );
	Out.normal = float4(In.normal, 1);
	Out.uv = In.uv;
	Out.light_direction = normalize(float4( In.pos, 0 ) - light.position);
	return Out;
}
