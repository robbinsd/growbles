#ifndef _FALCON_H
#define _FALCON_H

#include "Framework.h"
#include "Vector.h"
#include "UserInput.h"

const double FALCON_IMPULSE_STRENGTH = 1;
static const double FALCON_VERTICAL_STIFFNESS = 1000;
static const double FALCON_DAMPENING = 20;

enum FalconInputIndex{ FALCON_INPUT_FORWARD, FALCON_INPUT_RIGHT, FALCON_INPUT_UP };


class FalconDevice{

public:
    /*
     * Constructor. Attempts to connect with the USB Haptic Device.
     */
    FalconDevice();

    /*
     * Destructor
     */
    ~FalconDevice();

    /*
     * Returns whether or not the haptic device is connected
     */
    bool isConnected();
    /*
     * Gets the haptic device's offset vector from its resting position
     */
    void getPosition(float &up, float &right, float &forward);
    
    /*
     * Gets the velocity of the haptic device
     */
    void getVelocity(float &up, float &right, float &forward);

    void getForce(float &up, float &right, float &forward);

    /*
     * Set the force that the falcon experiences
     */
    void setHorizontalForce(float right, float forward);

    // Sets the vertical force, based on whether or not the falcon is
    // in freefall. If not in freefall, it will be forced toward the vertical
    // middle of its workspace.
    void setVerticalForce(bool isFalling);

    /*
     * Sets the input, given the previous tick's activeInputs.
     * Stores information indicating the current state of the haptic
     * device.
     */
    void setInputs(UserInput &input, uint32_t activeInputs);

protected:
    
    // Haptics
#ifdef FALCON
    cHapticDeviceHandler *mHapticHandler;
    cGenericHapticDevice *mHapticDevice;
    cHapticDeviceInfo mHapticDeviceInfo;
#endif 
    bool isReady;

};
#endif