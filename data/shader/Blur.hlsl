#define threadBlockSize 1

Texture2D input:register(t0);
RWTexture2D<float4> output:register(u1);


static const int Size_X=3;
static const int Size_Y=3;
//Begin：为测试不同卷积核，在此计算权重，若要优化时可换成直接读取矩阵或数组的值
float GetWeight(int i,int j)
{
    int max_dis=max(Size_X,Size_Y);
    return max_dis- sqrt(float(i*i+j*j));
}

[numthreads(threadBlockSize, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
    float totalWeight=0.0;
    float4 value=0.0;
    int max_dis=max(Size_X,Size_Y);
    for(int i=-Size_X;i<=Size_X;i++)
    {
        for(int j=-Size_Y;j<=Size_Y;j++)
        {
            float weight=GetWeight(i,j);
            if(weight<=0)
            {
                continue;
            }
            value+= weight*input[groupId.xy+int2(i,j)];
            totalWeight+=weight;
        }
    }
    output[groupId.xy]=value/totalWeight;
}
