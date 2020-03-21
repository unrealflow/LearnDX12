#ifndef _SHADER_COMMON_
#define _SHADER_COMMON_
#define MAX_MESH_COUNT 4

struct UniformBuffer
{
    float4x4 projection;
    float4x4 view;
    float4x4 jitterProj;
    float4x4 preProj;
    float4x4 preView;
    float3 camPos;
    float iTime;
    float3 camFront;
    float upTime;
    float delta;
};
struct SkMat
{
    float3 baseColor;
    float roughness;
    float metallic;
    float3 _PAD_;
};
struct MatInfoPack
{
    uint4 m[MAX_MESH_COUNT/2+1];
};
uint GetPrimCount(uint4 m[MAX_MESH_COUNT/2+1],uint index)
{
    uint p_index=index/4;
    uint tag=index%4;
    switch(tag)
    {
        case 0:return m[p_index].x;break;
        case 1:return m[p_index].y;break;
        case 2:return m[p_index].z;break;
        case 3:return m[p_index].w;break;
    }
    return 0;
}
uint GetUseTex(uint4 m[MAX_MESH_COUNT/2+1],uint index)
{
    return GetPrimCount(m,index+MAX_MESH_COUNT);
}
struct MatBufPack
{
    float4 m[MAX_MESH_COUNT*2];
};
SkMat GetMat(float4 m[MAX_MESH_COUNT*2],uint index)
{
    float4 p0=m[index*2];
    float4 p1=m[index*2+1];
    SkMat mat;
    mat.baseColor=p0.xyz;
    mat.roughness=p0.w;
    mat.metallic=p1.x;
    return mat;
}
float2 GetUV(float4x4 view,float4x4 proj,float3 _pos)
{
    float4 pos=float4(_pos,1.0);
    pos=mul(pos,view);
    pos=mul(pos,proj);
    pos/=pos.w;
    pos=pos*0.5+0.5;
    pos.y=1.0-pos.y;
    return pos.xy;
}
static const float PI=3.1415926535898;

#endif