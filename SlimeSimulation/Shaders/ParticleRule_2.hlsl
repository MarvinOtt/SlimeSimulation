
RWTexture3D<float4> particles : register(u0);
Texture2D<float> trailmapIN : register(t0);
RWTexture2D<float> trailmapOUT : register(u1);

SamplerState s1 : register(s0);

cbuffer cbConstants : register(b0)
{
    float sensor_angle;
    float sensor_distance;
    float steeringangle;
    float walkingdistance;
    float depositStrength;
    float resX;
    float resY;
    float curResX;
    float curResY;
};

inline float2 angle2vec(float angle)
{
    return float2(cos(angle), sin(angle));
}

uint hash(uint state)
{
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}

[numthreads(64, 1, 1)]
void main(uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_GroupID)
{
    float2 resFac = float2(1.f / resX, 1.f / resY);
    float resFacX = curResX / resX;
    float resFacY = curResY / resY;
    [fastopt]
    for (uint i = 0; i < 128; i++)
    {
        uint3 particleID = uint3(GTid.x, DTid.x, i);
        uint ID = GTid.x + DTid.x * 64 + i * 64 * 128;
        float4 partTemp = particles[particleID];

        // Sense
        float2 samplepoint_M = ((partTemp.xy + angle2vec(partTemp.z) * sensor_distance) * resFac);
        float2 samplepoint_L = ((partTemp.xy + angle2vec(partTemp.z - sensor_angle) * sensor_distance) * resFac);
        float2 samplepoint_R = ((partTemp.xy + angle2vec(partTemp.z + sensor_angle) * sensor_distance) * resFac);

        float sens_L = trailmapIN.SampleLevel(s1, samplepoint_L, 0);
        float sens_R = trailmapIN.SampleLevel(s1, samplepoint_R, 0);
        if (sens_L > 0.5f)
            sens_L = 1.0f - sens_L;
        if (sens_R > 0.5f)
            sens_R = 1.0f - sens_R;

        float dif = sens_L - sens_R;

        // Rotate
        if (dif > 0)
        {
            partTemp.z -= steeringangle;
        }
        else if (dif < 0)
        {
            partTemp.z += steeringangle;
        }

        if ((samplepoint_M.x < 0 || samplepoint_M.x > resFacX || samplepoint_M.y < 0 || samplepoint_M.y > resFacY) && resX > 1)
        {
            float2 newpos = max(float2(0.01f, 0.01f), min(float2(resFacX - 0.01f, resFacY - 0.01f), samplepoint_M));
            uint rand = hash(partTemp.x * curResX + partTemp.y + ID) % 1024;
            partTemp = float4(newpos * float2(resX, resY), rand * 0.006135923, 0);
        }

        // Move
        partTemp.xy += angle2vec(partTemp.z) * walkingdistance;
        particles[particleID] = partTemp;

        // Deposit
        uint2 mappos = trunc(partTemp);
        trailmapOUT[mappos] += depositStrength;
    }
}