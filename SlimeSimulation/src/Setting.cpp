#include "Setting.h"
#include <fstream>
#include <string>
#include <sstream>


bool Setting::StartInBackground = false;
int Setting::defaultMonitor = 0;
int Setting::VSyncFactor = 0;
int Setting::switchTime = 0;
int Setting::transitionFrames = 0;
Rule* Setting::rules = nullptr;
ImVec4* Setting::singleColors = nullptr;
ImVec4* Setting::dualColors_P1 = nullptr;
ImVec4* Setting::dualColors_P2 = nullptr;
int Setting::curSetting = 0;
int Setting::oldSetting = 0;
int Setting::curRule = 0;
int Setting::numRules = 0;
int Setting::numSingleColors = 0;
int Setting::numDualColors = 0;
ImVec4 Setting::partCol1 = ImVec4();
ImVec4 Setting::partCol2 = ImVec4();
float Setting::transValue = 1;
float Setting::transRef = 1;

ImVec4 Setting::backgroundColor = ImVec4(0, 0, 0, 1.0f);

vector<string> split(const std::string& s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string part;
    while (getline(ss, part, delim)) {
        elems.push_back(part);
    }
    return elems;
}

string getStringID(const std::string& in, int ID)
{
    stringstream ss(in);
    string part;
    for (int i = 0; i <= ID; ++i)
    {
        getline(ss, part, ' ');
        if (i == ID)
            return part;
    }
}

string getFirstString(std::ifstream& file, char delim)
{
    string firstString;
    string result;
    getline(file, firstString);
    stringstream ss(firstString);
    getline(ss, result, delim);
    return result;
}

bool Setting::Load()
{
    std::ifstream file("Settings.txt");
    std::string str;

    int curValidLine = 0;

    StartInBackground = stoi(getFirstString(file, ' ')) > 0;
    defaultMonitor = stoi(getFirstString(file, ' '));
    VSyncFactor = stoi(getFirstString(file, ' '));
    getline(file, str); // Color line
    backgroundColor.x = stoi(getStringID(str, 0)) / 255.0f;
    backgroundColor.y = stoi(getStringID(str, 1)) / 255.0f;
    backgroundColor.z = stoi(getStringID(str, 2)) / 255.0f;
    backgroundColor.w = stoi(getStringID(str, 3)) / 255.0f;

    getline(file, str); // Empty line
    switchTime = stoi(getFirstString(file, ' '));
    transitionFrames = stoi(getFirstString(file, ' '));
    getline(file, str); // Empty line
    numSingleColors = stoi(getFirstString(file, ' '));
    singleColors = new ImVec4[numSingleColors];
    getline(file, str); // Empty line
    for (int i = 0; i < numSingleColors; ++i)
    {
        getline(file, str); // Color line
        singleColors[i].x = stoi(getStringID(str, 0)) / 255.0f;
        singleColors[i].y = stoi(getStringID(str, 1)) / 255.0f;
        singleColors[i].z = stoi(getStringID(str, 2)) / 255.0f;
        singleColors[i].w = stoi(getStringID(str, 3)) / 255.0f;
    }
    getline(file, str); // Empty line
    numDualColors = stoi(getFirstString(file, ' '));
    dualColors_P1 = new ImVec4[numDualColors];
    dualColors_P2 = new ImVec4[numDualColors];
    getline(file, str); // Empty line
    for (int i = 0; i < numDualColors; ++i)
    {
        getline(file, str); // Color line 1
        dualColors_P1[i].x = stoi(getStringID(str, 0)) / 255.0f;
        dualColors_P1[i].y = stoi(getStringID(str, 1)) / 255.0f;
        dualColors_P1[i].z = stoi(getStringID(str, 2)) / 255.0f;
        dualColors_P1[i].w = stoi(getStringID(str, 3)) / 255.0f;

        getline(file, str); // Color line 2
        dualColors_P2[i].x = stoi(getStringID(str, 0)) / 255.0f;
        dualColors_P2[i].y = stoi(getStringID(str, 1)) / 255.0f;
        dualColors_P2[i].z = stoi(getStringID(str, 2)) / 255.0f;
        dualColors_P2[i].w = stoi(getStringID(str, 3)) / 255.0f;
        getline(file, str); // Empty line
    }

    numRules = stoi(getFirstString(file, ' '));

    if (rules != nullptr)
        delete rules;
    rules = new Rule[numRules];

    for (int i = 0; i < numRules; ++i)
    {
        getline(file, str); // Empty line
        getline(file, str); // Rule i
        int configCount = stoi(getFirstString(file, ' '));
        rules[i].particleCount = stoi(getFirstString(file, ' '));
        rules[i].settings.resize(configCount);

        for (int j = 0; j < configCount; ++j)
        {
            std::getline(file, str); // Empty line
            for (int k = 0; k < 7; ++k)
            {
                std::getline(file, str);
                float val = std::stof(str);
                SettingData* setting = &(rules[i].settings[j]);
                ((float*)setting)[k] = val;
            }
        }
    }
    return true;
}

Rule Setting::CurRule()
{
    return rules[curRule];
}

SettingData Setting::CurSett()
{
    return rules[curRule].settings[curSetting];
}

void Setting::Copy2Clip(float resFac, HWND hwnd, particleCB_DEF* particleCB, diffuseDecayCB_DEF* diffuseDecayCB)
{
    string finalString = string("");
    finalString += std::to_string(particleCB->sensor_angle) + string("\n");
    finalString += std::to_string(particleCB->sensor_distance) + string("\n");
    finalString += std::to_string(particleCB->steeringangle) + string("\n");
    finalString += std::to_string(particleCB->walkingdistance) + string("\n");
    finalString += std::to_string(particleCB->depositStrength) + string("\n");
    finalString += std::to_string(diffuseDecayCB->diffuseStrength) + string("\n");
    finalString += std::to_string(diffuseDecayCB->decayStrength / resFac);
    if (OpenClipboard(hwnd))
    {
        HGLOBAL clipbuffer;
        char* buffer;
        EmptyClipboard();
        clipbuffer = GlobalAlloc(GMEM_DDESHARE, finalString.size() + 1);
        buffer = (char*)GlobalLock(clipbuffer);
        strcpy(buffer, LPCSTR(finalString.c_str()));
        GlobalUnlock(clipbuffer);
        SetClipboardData(CF_TEXT, clipbuffer);
        CloseClipboard();
    }
}

void Setting::GenerateNewRuleSetting(float ruleProb)
{
    float randNewRule = rand() % 10000;
    if (numRules > 1)
    {
        if (randNewRule < ruleProb * 10001) // Switch to new Rule
        {
            int nextRule = 0;
            do {
                nextRule = rand() % numRules;
            } while (nextRule == curRule);
            curRule = nextRule;
        }
    }
    else
        curRule = 0;

    // Switch to new setting
    if (rules[curRule].settings.size() > 1)
    {
        int nextSetting = 0;
        do {
            nextSetting = rand() % rules[curRule].settings.size();
        } while (nextSetting == curSetting);
        curSetting = nextSetting;
    }
    else
        curSetting = 0;
    GenerateNewColor();
}

void Setting::GenerateNewColor()
{
    // Switch to new color
    int nextColor = rand() % ((rules[curRule].particleCount == 1) ? numSingleColors : numDualColors);
    if (rules[curRule].particleCount == 1)
        partCol1 = singleColors[nextColor];
    else
    {
        partCol1 = dualColors_P1[nextColor];
        partCol2 = dualColors_P2[nextColor];
    }
}

float fade(float t)
{
    return (t * t * t * (t * (t * 6 - 15) + 10));
}

void Setting::StartTransition(float val)
{
    transValue = val;
    transRef = val;
}

void Setting::SetRule(int newRule)
{

}

void Setting::Update(float resFac, particleCB_DEF* particleCB, diffuseDecayCB_DEF* diffuseDecayCB)
{
    transValue += 1.0f / (float)transitionFrames;
    if (transValue >= 1)
    {
        transValue = 1;
        //particleCB->sensor_angle = Setting::CurSett().sensor_angle;
        //particleCB->sensor_distance = Setting::CurSett().sensor_distance;
        //particleCB->steeringangle = Setting::CurSett().steeringangle;
        //particleCB->walkingdistance = Setting::CurSett().walkingdistance;
        //particleCB->depositStrength = Setting::CurSett().depositStrength;
        //diffuseDecayCB->diffuseStrength = Setting::CurSett().diffuseStrength;
        //diffuseDecayCB->decayStrength = Setting::CurSett().decayStrength;
    }
    else
    {
        float wantedvalue = fade(transValue);
        float fac = (wantedvalue - transRef) / (1.0001f - transRef);
        transRef = wantedvalue;

        particleCB->sensor_angle = Setting::CurSett().sensor_angle * fac + particleCB->sensor_angle * (1.0f - fac);
        particleCB->sensor_distance = Setting::CurSett().sensor_distance * fac + particleCB->sensor_distance * (1.0f - fac);
        particleCB->steeringangle = Setting::CurSett().steeringangle * fac + particleCB->steeringangle * (1.0f - fac);
        particleCB->walkingdistance = Setting::CurSett().walkingdistance * fac + particleCB->walkingdistance * (1.0f - fac);
        particleCB->depositStrength = Setting::CurSett().depositStrength * fac + particleCB->depositStrength * (1.0f - fac);
        diffuseDecayCB->diffuseStrength = Setting::CurSett().diffuseStrength * fac + diffuseDecayCB->diffuseStrength * (1.0f - fac);
        diffuseDecayCB->decayStrength = Setting::CurSett().decayStrength * resFac * fac + diffuseDecayCB->decayStrength * (1.0f - fac);

        diffuseDecayCB->partCol1.x = partCol1.x * fac + diffuseDecayCB->partCol1.x * (1.0f - fac);
        diffuseDecayCB->partCol1.y = partCol1.y * fac + diffuseDecayCB->partCol1.y * (1.0f - fac);
        diffuseDecayCB->partCol1.z = partCol1.z * fac + diffuseDecayCB->partCol1.z * (1.0f - fac);
        diffuseDecayCB->partCol1.w = partCol1.w * fac + diffuseDecayCB->partCol1.w * (1.0f - fac);

        diffuseDecayCB->partCol2.x = partCol2.x * fac + diffuseDecayCB->partCol2.x * (1.0f - fac);
        diffuseDecayCB->partCol2.y = partCol2.y * fac + diffuseDecayCB->partCol2.y * (1.0f - fac);
        diffuseDecayCB->partCol2.z = partCol2.z * fac + diffuseDecayCB->partCol2.z * (1.0f - fac);
        diffuseDecayCB->partCol2.w = partCol2.w * fac + diffuseDecayCB->partCol2.w * (1.0f - fac);
    }
}