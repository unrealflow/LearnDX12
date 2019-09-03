struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv: TEXCOORD;
};
struct PSOutput
{
    float4 rt0 : SV_TARGET0;
    // float4 rt1 : SV_TARGET1;
};
struct UniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float iTime;
    float upTime;
};
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);
ConstantBuffer<UniformBuffer> buf:register(b0);
PSInput VSMain(
    uint ID :SV_VERTEXID)
{
    
    PSInput result;
    result.uv=float2(ID & 2, (ID << 1) & 2);//(0,0),(0,2),(2,0),(2,2)
    result.position=float4(result.uv * 2.0 - 1.0, 0.0, 1.0);

    return result;
}

PSOutput PSMain(PSInput input)
{
    PSOutput p;
    p.rt0 = g_texture.Sample(g_sampler, float2(input.uv.x,1.0-input.uv.y));
    // p.rt1=p.rt0;
    return p;
}
