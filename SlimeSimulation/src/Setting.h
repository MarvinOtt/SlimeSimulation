#pragma once
#include <vector>
#include <Windows.h>
#include "ImGui/imgui.h"
using namespace std;

class Shader;
class PipelineState;

struct particleCB_DEF
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

struct diffuseDecayCB_DEF
{
    float diffuseStrength;
    float decayStrength;
    ImVec2 padding;
    ImVec4 backgroundColor;
    ImVec4 partCol1;
    ImVec4 partCol2;
};

struct SettingData
{
public:
    float sensor_angle;
    float sensor_distance;
    float steeringangle;
    float walkingdistance;
    float depositStrength;
    float diffuseStrength;
    float decayStrength;
};

class Rule
{
public:
    Shader* particleShader;
    PipelineState* diffuseDecayPipelineState;
    PipelineState* particlePipelineState;
    vector<SettingData> settings;
    int particleCount;
};

class Setting
{
public:
    static bool StartInBackground;
    static int defaultMonitor;
    static int VSyncFactor;
    static int switchTime;
    static int transitionFrames;
    static int numRules;
    static int numSingleColors;
    static int numDualColors;

    static Rule* rules;
    static ImVec4* singleColors;
    static ImVec4* dualColors_P1;
    static ImVec4* dualColors_P2;

    static int curRule;
    static int curSetting;
    static int oldSetting;
    static ImVec4 backgroundColor;

    static ImVec4 partCol1;
    static ImVec4 partCol2;

private:
    static float transValue;
    static float transRef;

public:

    static bool Load();
    static SettingData CurSett();
    static Rule CurRule();
    static void Copy2Clip(float resFac, HWND hwnd, particleCB_DEF* particleCB, diffuseDecayCB_DEF* diffuseDecayCB);
    static void GenerateNewRuleSetting(float ruleProb);
    static void StartTransition(float val = 0.0f);
    static void SetRule(int newRule);
    static void GenerateNewColor();

    static void Update(float resFac, particleCB_DEF* particleCB, diffuseDecayCB_DEF* diffuseDecayCB);
};

