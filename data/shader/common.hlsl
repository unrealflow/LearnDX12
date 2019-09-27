#ifndef _SHADER_COMMON_
#define _SHADER_COMMON_
struct UniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float3 camPos;
    float iTime;
    float3 camFront;
    float upTime;
};
struct SkMat
{
    float metallic;
    float roughness;
};

static const float PI=3.1415926535898;

#endif