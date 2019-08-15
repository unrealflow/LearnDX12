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
    float4 color : COLOR;
    float2 uv: TEXCOORD;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR,float2 uv: TEXCOORD)
{
    PSInput result;

    result.position = position;
    float inter=uv.x*uv.y;
    result.color =float4(uv.x-inter,uv.y-inter,inter,1.0);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
