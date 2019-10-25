#include "common.hlsl"
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

Texture2D position : register(t0);
Texture2D normal : register(t1);
Texture2D albedo : register(t2);

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

float noise(float k)
{
    return frac(sin(k*1313.11413+23.11333)*146.13973);
}
float3 noise3(int k,int size,float3 bias)
{
    float _size=size;
    float _size2=_size*_size;
    float _size3=_size2*_size;
    bias+=k;
    
    float p=frac(k/_size3+noise(bias.z))*2.0*PI;
    float z=frac(p+k/_size2+noise(bias.y))*2.0-1.0;
    float d=frac(z*PI+ k/_size+noise(bias.x));

    float r=sqrt(1.0-z*z);
    float x=r*sin(p);
    float y=r*cos(p);
    return d*float3(x,y,z);
}


PSOutput PSMain(PSInput input)
{
    float2 uv=float2(input.uv.x,1.0-input.uv.y);
    PSOutput p;
    float4 pos_depth  = position.Sample(g_sampler, uv);
    if(pos_depth.w<0.0001)
    {
        p.rt0=0.0;
        return p;
    }
    float3 _nor = normal.Sample(g_sampler,uv).xyz;
    float radius=1.5;
    uint size=3;
    uint count=size*size*size;
    float weight=0.0;
    for(uint i=0;i<count;i++)
    {
        float3 pos=pos_depth.xyz;
        int3 signs=sign(pos);
        int bias=dot(signs+2,int3(1,2,3));
        float3 _noise=noise3(i,size,pos);
        pos+=  radius*_noise*dot(_noise,_nor);
        float4 v_pos=mul(float4(pos,1.0),buf.view);
        float depth=-v_pos.z;
        v_pos=mul(v_pos,buf.projection);
        v_pos/=v_pos.w;
        v_pos=v_pos*0.5+0.5;
        v_pos.y=1.0-v_pos.y;
        float cur_depth=position.Sample(g_sampler,v_pos.xy).w;
        if(cur_depth<0.0001)
        {
            weight+=0.0;
            continue;
        }
        float rangeCheck = smoothstep(  radius ,radius*0.8, abs(depth - cur_depth));
        weight += (depth >= cur_depth ? 1.0 : 0.0) * rangeCheck;   
    }

    p.rt0=min((1.0-weight/count)*1.0,1.0);
    // p.rt0=0.5;
    return p;
}
