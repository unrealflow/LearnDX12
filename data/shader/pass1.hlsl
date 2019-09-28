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
static const float3 WorldUp=float3(0.0,1.0,0.0);

Texture2D g_texture : register(t0);
Texture2D bk_texture : register(t1);
Texture2D position : register(t2);
Texture2D normal : register(t3);
Texture2D albedo : register(t4);
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
float2 DirToUV(float3 dir)
{
    float2 c_angle;
    c_angle.y=asin(-dir.y);
    float t=sqrt(1.0-dir.y*dir.y);
    c_angle.x=asin(dir.x/t);
    float2 _UV= (c_angle)/PI+0.5;
    _UV.x=0.5+_UV.x*(step(0.0,dir.z)-0.5);
    return _UV*0.9999+0.00005;
}
float3 UVToDir(float3 _front,float2 uv)
{
    float3 right=normalize(cross(_front,WorldUp));
    float3 up=normalize(cross(right,_front));
    float c=buf.projection[1][1];
    float r=buf.projection[1][1]/buf.projection[0][0];
    float2 coord=uv*2.0-1.0;
    coord.y/=-r;
    coord/=c;
    float3 dir=normalize(_front+coord.x*right+coord.y*up);
    return dir;
}

PSOutput PSMain(PSInput input)
{
    float2 uv=float2(input.uv.x,1.0-input.uv.y);
    PSOutput p;
    float3 _color = albedo.Sample(g_sampler, uv).xyz;
    float3 _pos = position.Sample(g_sampler, uv).xyz;
    if(length(_pos)<0.01)
    {
        float3 dir=UVToDir(buf.camFront,uv);
        p.rt0=bk_texture.SampleLevel(g_sampler,DirToUV(dir),0.0);
        // p.rt0=bk_texture.Sample(g_sampler,DirToUV(dir));
        return p;
    }
    float3 _nor = normal.Sample(g_sampler, uv).xyz;
    float3 viewDir=normalize(buf.camPos-_pos);
    float3 lightDir=lightPos-_pos;
    float lightDis=length(lightDir);
    lightDir=lightDir/lightDis;

    float f=lightPower/(lightDis*lightDis);
    _color=f*_color;
    float3 kS;
    SkMat mat;
    mat.roughness=0.3;
    mat.metallic=0.3;
    _color= BRDF(mat,_color,lightDir,viewDir,_nor,kS);
    float3 ref_dir=reflect(-viewDir,_nor);
    float2 ref_uv=DirToUV(ref_dir);

    float stride=mat.roughness*0.001;
    int size=3;
    float _size=float(size);
    float3 ref_color=0.0;
    {
        float total_weight=0.0;
        for(int i=-size;i<=size;i++)
        {
            for(int j=-size;j<=size;j++)
            {
                float2 bias=float2(i*stride,j*stride);
                float weight=(_size-length(bias))/_size;
                // ref_color+=weight*bk_texture.Sample(g_sampler,ref_uv+bias);
                ref_color+=weight*bk_texture.SampleLevel(g_sampler,ref_uv+bias,0.0);
                
                total_weight+=weight;
            }
        }
        ref_color/=total_weight;
    }
    float3 dif_color=0.0;

    _color+=ref_color*(1.0-mat.roughness)*F_Schlick(dot(_nor,viewDir) ,kS);
    _color=pow(_color,float3(0.45,0.45,0.45));
    p.rt0=float4(_color,1.0);
    return p;
}
