#ifndef PLATFORM_H
#define PLATFORM_H

#include "Framework.h"
#include <math.h>

const float START_RADIUS = 15.0;    // Radius to start with
const float RADIUS_DECREASE = 3.0;  // Radius of each ring that falls
const int NUM_DROPS = 4;            // Number of drops
const int BLINK_TICKS = 10;         // Time to switch blink color
const float GRAVITY = 0.05;         // Gravity

enum {IDLE, BLINKING, FALLING};

struct platformState {
    int dropTimer, blinkTimer, dropCount, fallingRing;
    bool blinkOn;
    float curRadius, curDrawRadius;
    float dropVelocity, dropY;
    int dropState;
};

class Platform
{
public:
    Platform(int timeToDrop);
    ~Platform();
    void reset();
    void update();
    void render();
    float getRadius();
    int getFallingRing();
    float getFallingRingPos();
    platformState GetPlatformState();
    void SetPlatformState(platformState pinfo);

private:
    int dropTicks, dropTimer, blinkTimer, dropCount, fallingRing; // Timers and counters
    bool blinkOn; // Toggles blinking
    float curRadius, curDrawRadius; // Indicates the collision and actual drawn radius
    float dropVelocity, dropY; // Drop movement

    // The colors for different parts of the platform
    float regularColor[3], brightColor[3], topColor[3];

    // Quadric objects for cylinders, and disks of the platform
    GLUquadric *innerCylinder, *outerCylinder;
    GLUquadric *innerDisk, *outerDisk;

    // Drop state variable
    int dropState;
    
    sf::SoundBuffer Buffer;
    sf::Sound Sound;
    sf::SoundBuffer Buffer2;
    sf::Sound Sound2;
};

#endif