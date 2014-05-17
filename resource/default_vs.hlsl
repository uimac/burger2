#include "common.hlsl"

// vertex shader
VS_OUT VS_Main( VS_IN In )
{
   VS_OUT Out;

//   Out.pos = mul( float4( In.pos, 1 ), world_view_projection );
   Out.pos = float4( In.pos, 1 );
   Out.uv = In.uv;
   //Out.color = In.color;

   return Out;
}
