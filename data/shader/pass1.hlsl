#include "common.hlsl"
#include "BRDF.hlsl"
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

static const float3 lightPos=float3(10.0,20.0,-20.0);
static const float lightPower= 1000.0;
Texture2D g_texture : register(t0);
Texture2D position : register(t1);
Texture2D normal : register(t2);
Texture2D albedo : register(t3);
SamplerState g_sampler : register(s0);
ConstantBuffer<UniformBuffer> buf : register(b0);
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
    float3 _color = albedo.Sample(g_sampler, float2(input.uv.x,1.0-input.uv.y)).xyz;
    float3 _pos = position.Sample(g_sampler, float2(input.uv.x,1.0-input.uv.y)).xyz;
    if(length(_pos)<0.01)
    {
        p.rt0=0.0;
        return p;
    }
    float3 _nor = normal.Sample(g_sampler, float2(input.uv.x,1.0-input.uv.y)).xyz;
    float3 viewDir=normalize(buf.camPos-_pos);
    float3 lightDir=lightPos-_pos;
    float lightDis=length(lightDir);
    lightDir=lightDir/lightDis;

    float f=lightPower/(lightDis*lightDis);
    _color=f*_color;
    float3 kS;
    SkMat mat;
    mat.roughness=0.1;
    mat.metallic=0.5;
    _color= BRDF(mat,_color,lightDir,viewDir,_nor,kS);
    

    _color=pow(_color,float3(0.45,0.45,0.45));
    p.rt0=float4(_color,1.0);
    return p;
}
