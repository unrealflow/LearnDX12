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
Texture2D rt_AO : register(t3);
Texture2D rt_deferred : register(t4);

Texture2D preImage : register(t5);
Texture2D prePosition : register(t6);
Texture2D preNormal : register(t7);


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
static const float2 w=float2(0.001,0.001);
float3 RGBToYCoCg(float3 c)
{
	return float3(
		0.5*c.g+0.25*(c.r+c.b),
		0.5*(c.r-c.b),
		0.5*c.g-0.25*(c.r+c.b)
	);
}
float3 YCoCgToRGB(float3 c)
{
	return float3(
		c.x+c.y-c.z,
		c.x+c.z,
		c.x-c.y-c.z
	);
}
PSOutput PSMain(PSInput input)
{
    PSOutput p;
    float2 inUV=float2(input.uv.x,1.0-input.uv.y); 
    float4 curColor=rt_deferred.Sample(g_sampler,inUV);
    if(curColor.w<0.9||abs(inUV.x-0.5)>(0.5-w.x)||abs(inUV.y-0.5)>(0.5-w.y))
    {
        p.rt0=curColor;
        return p;
    }
    
    float4 gBufPos=position.Sample(g_sampler,inUV);
    float2 preUV=inUV;
    //仅在VP矩阵变化时计算preUV
    float deltaTime=buf.iTime-buf.upTime;
    if(length(gBufPos)>1e-5&&deltaTime<buf.delta)
    {
        preUV=GetUV(buf.preView,buf.preProj,gBufPos.xyz);
    }

   
    float4 preColor=preImage.Sample(g_sampler, preUV);

    float3 minColor=RGBToYCoCg(curColor.xyz);
	float3 maxColor=minColor;
	for(int u=-1;u<=1;u++)
	{
		for(int v=-1;v<=1;v++)
		{
			float3 data=RGBToYCoCg(rt_deferred.Sample(g_sampler,inUV+float2(u,v)*w).xyz);
			minColor=min(data,minColor);
			maxColor=max(data,maxColor);
		}
	}
    preColor.xyz=RGBToYCoCg(preColor.xyz);
	preColor.xyz=clamp(preColor.xyz,minColor,maxColor);
	preColor.xyz=YCoCgToRGB(preColor.xyz);

    p.rt0 = lerp(curColor,preColor,0.98);

    return p;
}
