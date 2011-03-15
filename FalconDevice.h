#ifndef _FALCON_H
#define _FALCON_H

#include "Framework.h"
#include "Vector.h"
#include "UserInput.h"
using namespace std;

const double FALCON_IMPULSE_STRENGTH = 1;
const double FALCON_IMPULSE_LENGTH = 1; //length of time in seconds that the impulse lasts
static const double FALCON_VERTICAL_STIFFNESS = 1000;
static const double FALCON_HORIZONTAL_STIFFNESS = 120;
static const double FALCON_DAMPENING = 5;

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
     * Initialize haptic device. Start haptics thread.
     */
    void Init();

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

    // Sets the vertical force
    void setVerticalForce(float platformHeight);

    /*
     * Sets the input, given the previous tick's activeInputs.
     * Stores information indicating the current state of the haptic
     * device.
     */
    void setInputs(UserInput &input, uint32_t activeInputs);

    void hapticsLoop();

protected:


#ifdef FALCON
    //saves the horizontal forces being applied
    vector<cVector3d> forcesToApply; //x is back, y is right, z is time remaining
    cVector3d force;
    int forceCount;
    
    //saves whether or not the FalconDevice is currently falling.
    //used to compute the vertical force.
    //float prevHeight;
    float height;

    // Haptics
    cHapticDeviceHandler *mHapticHandler;
    cGenericHapticDevice *mHapticDevice;
    cHapticDeviceInfo mHapticDeviceInfo;
    bool mSimulationRunning;
    bool mSimulationFinished;
    bool beingUsed;
#endif 
    bool isReady;

};
#endif