//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv: TEXCOORD;
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

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, float2(input.uv.x,1.0-input.uv.y));
    // return float4(buf.color,1.0);
}
