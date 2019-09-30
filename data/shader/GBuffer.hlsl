#include "common.hlsl"
struct PSInput
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv: TEXCOORD;
    float4 w_pos : COLOR;
};
struct PSOutput
{
    float4 position : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 albedo : SV_TARGET2;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
ConstantBuffer<UniformBuffer> buf:register(b0);
PSInput VSMain(
    float4 position : POSITION, float4 normal : NORMAL,float2 uv: TEXCOORD,
    uint ID :SV_VERTEXID)
{
    
    // position.y+=pow((1.0-cos(position.x+0.001*buf.iTime))/2.0,3);
    PSInput result;
    result.w_pos=position;
    result.position=float4(position.xyz,1.0);
    result.position=mul(result.position,buf.view);
    result.w_pos.w=-result.position.z;
    result.position=mul(result.position,buf.projection);
    result.normal=normal;
    result.uv=uv;
    return result;
}

PSOutput PSMain(PSInput input)
{
    PSOutput p;
    p.position=input.w_pos;
    p.normal=input.normal;
    p.albedo = g_texture.Sample(g_sampler, float2(input.uv.x,input.uv.y));
    return p;
}