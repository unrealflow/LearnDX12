struct PSInput
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv: TEXCOORD;
};
struct PSOutput
{
    float4 rt0;
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
    // float4 position : POSITION, float4 normal : NORMAL,float2 uv: TEXCOORD,
    uint ID :SV_VERTEXID)
{
    
    PSInput result;
    result.uv=float2(ID & 2, (ID << 1) & 2);//(0,0),(0,2),(2,0),(2,2)
    result.position=float4(result.uv * 2.0 - 1.0, 0.0, 1.0);
    // position.y+=pow((1.0-cos(position.x+0.001*buf.iTime))/2.0,3);
    // result.position=float4(position.xyz,1.0);
    // result.position=mul(result.position,buf.view);
    // result.position=mul(result.position,buf.projection);
    // result.normal=normal;
    // result.uv=uv;
    return result;
}

PSOutput PSMain(PSInput input)
{
    PSOutput p;
    p.rt0 = g_texture.Sample(g_sampler, float2(input.uv.x,1.0-input.uv.y));
    return p;
}