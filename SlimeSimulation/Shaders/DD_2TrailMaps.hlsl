
Texture2D<float2> trailmapIN : register(t0);
RWTexture2D<float2> trailmapOUT : register(u1);

cbuffer constants : register(b0)
{
    float diffuseStrength;
    float decayStrength;
    float4 backgroundColor;
    float4 partCol1;
    float4 partCol2;
};

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
    uint2 xy = trunc(pos);
    float2 sum = 0.f;
    sum += trailmapIN[xy + uint2(-1, -1)];
    sum += trailmapIN[xy + uint2(0, -1)];
    sum += trailmapIN[xy + uint2(1, -1)];
    sum += trailmapIN[xy + uint2(1, 0)];
    sum += trailmapIN[xy + uint2(1, 1)];
    sum += trailmapIN[xy + uint2(0, 1)];
    sum += trailmapIN[xy + uint2(-1, 1)];
    sum += trailmapIN[xy + uint2(-1, 0)];
    sum += trailmapIN[xy];
    sum /= 9.f;

    float2 finalval = max(((sum * (diffuseStrength)) + (trailmapIN[xy] * (1.f - diffuseStrength))) - float2(decayStrength, decayStrength), float2(0, 0));
    finalval = min(float2(1.f, 1.f), finalval);
    trailmapOUT[xy] = finalval;

    float3 col1 = partCol1.rgb * finalval.x;
    float3 col2 = partCol2.rgb * finalval.y;

    //float3 col3 = col1 + col2;
    float3 col3 = (-pow(((col1 + col2) + float3(1, 1, 1)), -1.2) + 1) * 1.4383135f;

    return saturate(float4(col3, 1.0f) + backgroundColor);
}