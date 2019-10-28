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
Texture2D AO : register(t5);

Texture2D preImage : register(t6);
Texture2D prePosition : register(t7);
Texture2D preNormal : register(t8);

SamplerState g_sampler : register(s0);
ConstantBuffer<UniformBuffer>  buf : register(b0);
ConstantBuffer<MatBufPack> matPack :register(b1);

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
float GetAO(float2 uv)
{
    int size=0;

    float ao=0.0;
    float stride=0.001;
    float totalWeight=0.0;
    for(int i=-size;i<=size;i++)
    {
        for(int j=-size;j<=size;j++)
        {

            float weight=size- sqrt(float(i*i+j*j));
            if(weight<=0)
            {
                continue;
            }
            totalWeight+=weight;
            ao+=weight*AO.Sample(g_sampler,uv+float2(i,j)*stride).x;
        }
    }

    return ao/totalWeight;
}

float4 Render(float4 albedo,float4 normal,float2 uv)
{
    if(albedo.z<0.01)
    {
        float3 dir=UVToDir(buf.camFront,uv);
        // p.rt0=bk_texture.Sample(g_sampler,DirToUV(dir));
        return float4(bk_texture.SampleLevel(g_sampler,DirToUV(dir),0.0).xyz,0.0);
    }
    float3 _nor = normal.xyz;
    uint meshID=uint(normal.w);
    float3 _pos = position.Sample(g_sampler, uv).xyz;
    // float ao=GetAO(uv);
    float ao=AO.Sample(g_sampler,uv);
    albedo.xyz*=ao;
    float3 viewDir=normalize(buf.camPos-_pos);
    float3 lightDir=lightPos-_pos;
    float lightDis=length(lightDir);
    
    lightDir=lightDir/lightDis;

    float f=lightPower/(lightDis*lightDis);
    float3 _color=f*albedo.xyz;
    float3 kS;
    SkMat mat=GetMat(matPack.m,meshID);

    _color= BRDF(mat,_color,lightDir,viewDir,_nor,kS);
    float3 ref_dir=reflect(-viewDir,_nor);
    float2 ref_uv=DirToUV(ref_dir);
    float3 ref_color=bk_texture.SampleLevel(g_sampler,ref_uv,mat.roughness*10.0);
    float3 dif_color=albedo.xyz*0.5;
    float ref_factor=(1.0-mat.roughness)*F_Schlick(dot(_nor,viewDir) ,kS);
    _color+=lerp(ref_color*ref_factor,dif_color,mat.roughness);
    return float4(_color,1.0);
}
float ev(float4 a,float4 b)
{
    return exp(length(a-b))-1.0;
}
PSOutput PSMain(PSInput input,uint ID:SV_PrimitiveID)
{
    
    float2 uv=float2(input.uv.x,1.0-input.uv.y);
    PSOutput p;
    float4 _albedo = albedo.Sample(g_sampler, uv);
    float4 _nor=normal.Sample(g_sampler,uv);
    float4 color=Render(_albedo,_nor,uv);
    float stride=0.0005;
    float4 _preImage=preImage.Sample(g_sampler,uv);
    float4 _preImage1=preImage.Sample(g_sampler,uv+float2(stride,0.0));
    float4 _preImage2=preImage.Sample(g_sampler,uv-float2(stride,0.0));
    float4 _preImage3=preImage.Sample(g_sampler,uv+float2(0.0,stride));
    float4 _preImage4=preImage.Sample(g_sampler,uv-float2(0.0,stride));
    _preImage.w=max(_preImage.w,max(max(_preImage1.w,_preImage2.w),max(_preImage3.w,_preImage4.w)));

    float4 prePos=prePosition.Sample(g_sampler,uv);
    float4 curPos=position.Sample(g_sampler,uv);
    float4 preNor=preNormal.Sample(g_sampler,uv);

    float d_p=ev(prePos,curPos);
    float d_n=ev(preNor,_nor);


    float t=0.6/(1.0+d_p+d_n);
    t=clamp(t,0.0,0.95);
    t=t+(0.95-t)*(color.w*0.5+_preImage.w*0.5);
    
    p.rt0=lerp(color,_preImage,t);
    // p.rt0=clamp(p.rt0,_preImage-0.05,_preImage+0.05);
    // p.rt0=color;
    // p.rt0=ao;
    return p;
}
