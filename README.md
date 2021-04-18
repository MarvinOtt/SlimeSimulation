# SlimeSimulation

## Features
* Multi-particle simulations (up to 2 particles)
* Background support
* Different colors for particles
* Changing the current configuration in realtime
* Auto rotate rules and settings

## Configuration
Configurations can be done in the **setting.txt** file\
Following configurations can be done:
1. Start application in background for the selected monitor
2. Default monitor
3. V-Sync factor
4. Background Color
5. Time between setting changes
6. Number of frames when a transition happens
7. Colors for the particles
8. All the rules and their configurations

A rule configuration can be added by incrementing the configuration number for the specific rule and adding the *configuration block*.\
The *configuration block* can be copied by pressing the Copy2Clip button.
A *configuration block* consists of the following 7 things:
* Angle of the left and right sensor
* Distance of the middle, left and right sensor
* Steering angle
* Walking distance
* Deposit strength
* Diffuse strength
* Decay strength

## Requirements

* Windows 7, 8 or 10
* GPU with DirectX 12 support
* [Visual C++ Redistributable x64](https://support.microsoft.com/de-de/topic/aktuelle-unterst%C3%BCtzte-downloads-f%C3%BCr-visual-c-2647da03-1eea-4433-9aff-95f26a218cc0)

## Implementation

This project is implemented in C++ with the DirectX 12 API and imgui for the GUI.\
In order to improve the ease of use of DirectX a small Engine has been writen. This Engine is located in the [Engine folder](SlimeSimulation/Engine).\
The main application files are located under the src folder.

## Credit
This project is inspired from:
https://www.youtube.com/watch?v=X-iSQQgOd1A&t=1s&ab_channel=SebastianLague
