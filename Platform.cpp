
#include <math.h>
#include "Platform.h"

Platform::Platform(int timeToDrop) : dropTicks(timeToDrop)
{
	regularColor[0] = 0.886;
	regularColor[1] = 0.635;
	regularColor[2] = 0.039;
	brightColor[0] = 0.933;
	brightColor[1] = 0.913;
	brightColor[2] = 0.102;
	topColor[0] = 0;
	topColor[1] = 0.624;
	topColor[2] = 0;

	innerCylinder = gluNewQuadric();
	outerCylinder = gluNewQuadric();
	innerDisk = gluNewQuadric();
	outerDisk = gluNewQuadric();
	reset();
}

Platform::~Platform()
{
	gluDeleteQuadric(innerCylinder);
	gluDeleteQuadric(outerCylinder);
	gluDeleteQuadric(innerDisk);
	gluDeleteQuadric(outerDisk);
}

void
Platform::reset()
{
	dropTimer=blinkTimer=dropCount=fallingRing=0;
	blinkOn=false;
	curRadius=curDrawRadius=START_RADIUS;
	dropState = IDLE;
	dropY = dropVelocity = 0.0f;
}

void
Platform::update()
{
	if(dropState == IDLE || dropState == BLINKING) // If waiting to drop
    {
		if(dropCount<NUM_DROPS) // If still have cylinders to drop
        {
			dropTimer++;
            // Set state to blinking to signal that a drop is imminent
			if(dropState!=BLINKING && ((float)dropTimer/(float)dropTicks) > 0.5)
															dropState = BLINKING;
			if(dropTimer>=dropTicks)
            { // If its time to drop switch to falling state
				dropTimer = 0;
				dropCount++;
				blinkOn = true;
				curRadius -= RADIUS_DECREASE; // Decrease the radius of the platform
				dropState = FALLING;
			}
		}
	}
	if(dropState == BLINKING)
    {
		blinkTimer++;
		if(blinkTimer>BLINK_TICKS)
        {
            blinkOn = !blinkOn;
            blinkTimer=0;
        }
	}
	if(dropState == FALLING)
    {
        // Play sound here
        
        blinkOn = false;
		dropVelocity += GRAVITY;
		dropY+=dropVelocity;
		if(dropY > 30.0) // If done dropping, go back to being idle
        {
			dropY=0;
			dropVelocity = 0;
			blinkOn = false;
			curDrawRadius -= RADIUS_DECREASE; // Decrease the draw radius
			dropState = IDLE;
            fallingRing++;
		}
	}
}

void
Platform::render()
{
    // Render the warning blink
	glPushMatrix();
	if(blinkOn) {
        glColor4f(brightColor[0], brightColor[1], brightColor[2], 0.2);
        glTranslatef(0, -dropY+4.1, -0.4);
        glRotatef(-90.0, 1, 0, 0);
        gluDisk (innerDisk, curDrawRadius-RADIUS_DECREASE, curDrawRadius, 64, 1); 
    }

	glPopMatrix();
}

float
Platform::getRadius()
{
	return curRadius;
}

int
Platform::getFallingRing()
{
    return fallingRing;
}

float
Platform::getFallingRingPos()
{
    return -dropY;
}

platformState
Platform::GetPlatformState()
{
    platformState pinfo;
    pinfo.dropTimer = dropTimer;
    pinfo.blinkTimer = blinkTimer;
    pinfo.dropCount = dropCount;
    pinfo.fallingRing = fallingRing;
    pinfo.blinkOn = blinkOn;
    pinfo.curRadius = curRadius;
    pinfo.curDrawRadius = curDrawRadius;
    pinfo.dropVelocity = dropVelocity;
    pinfo.dropY = dropY;
    pinfo.dropState = dropState;
    return pinfo;
}

void
Platform::SetPlatformState(platformState pinfo)
{
    dropTimer = pinfo.dropTimer;
    blinkTimer = pinfo.blinkTimer;
    dropCount = pinfo.dropCount;
    fallingRing = pinfo.fallingRing;
    blinkOn = pinfo.blinkOn;
    curRadius = pinfo.curRadius;
    curDrawRadius = pinfo.curDrawRadius;
    dropVelocity = pinfo.dropVelocity;
    dropY = pinfo.dropY;
    dropState = pinfo.dropState;
}
