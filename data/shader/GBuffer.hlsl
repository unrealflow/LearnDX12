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
ConstantBuffer<MatInfoPack> matInfo :register(b1);
ConstantBuffer<MatBufPack> matBuf :register(b2);

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
int GetMeshID(uint primID)
{
    for(uint i=0;i<MAX_MESH_COUNT;i++)
    {
        uint primCount=GetPrimCount(matInfo.m,i);
        if(primID<primCount)
        {
            return i;
        }
        primID-=primCount;
    }
    return 0;
}

PSOutput PSMain(PSInput input,uint primID:SV_PrimitiveID)
{
    PSOutput p;
    uint meshID=GetMeshID(primID);
    p.position=input.w_pos;

    p.normal=float4(input.normal.xyz,float(meshID));
    // p.albedo = g_texture.Sample(g_sampler, float2(input.uv.x,input.uv.y));
    // p.albedo=float(matInfo.m[0].z)/24412.0;
    uint useTex=GetUseTex(matInfo.m,meshID);
    float4 baseColor=float4(GetMat(matBuf.m,meshID).baseColor,1.0);
    switch(useTex)
    {
        case 0:p.albedo=baseColor;break;
        case 1:p.albedo=baseColor*g_texture.Sample(g_sampler, float2(input.uv.x,input.uv.y));break;
    }
    return p;
}