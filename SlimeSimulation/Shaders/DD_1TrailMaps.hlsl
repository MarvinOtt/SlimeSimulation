
Texture2D<float> trailmapIN : register(t0);
RWTexture2D<float> trailmapOUT : register(u1);

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
    float sum = 0.f;
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

    float finalval = max(((sum * (diffuseStrength)) + (trailmapIN[xy] * (1.f - diffuseStrength))) - decayStrength, 0);
    finalval = min(1.f, finalval);
    trailmapOUT[xy] = finalval;


    return saturate(partCol1 * finalval + backgroundColor);
}