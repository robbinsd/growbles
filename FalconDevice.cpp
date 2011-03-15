#include "FalconDevice.h"

void hapticsLoopCallback();
static FalconDevice *hapticsLoopThisPtr = NULL;

//DON"T CHANGE THE ORDER OF THESE
enum SwitchIndexT{  SWITCH_INDEX_JUMP=0, SWITCH_INDEX_SHRINK, SWITCH_INDEX_DASH,
    SWITCH_INDEX_GROW };

FalconDevice::FalconDevice(){
    isReady = false;
#ifdef FALCON
    isFalling = false;
    mSimulationRunning = false;
    mSimulationFinished = false;
    mHapticHandler = new cHapticDeviceHandler();

    // get access to the first available haptic device
    mHapticHandler->getDevice(mHapticDevice, 0);

    // retrieve information about the current haptic device
    if (mHapticDevice){
        mHapticDeviceInfo = mHapticDevice->getSpecifications();
    }
    int result = 0;
    // open connection to device
    result = mHapticDevice->open();

	// initialize device
	if (result == 0){
		result = mHapticDevice->initialize();
	}

#endif
}

FalconDevice::~FalconDevice(){
#ifdef FALCON
    if(!isConnected())
        return;
    mHapticDevice->close();
    mSimulationRunning = false;
    mSimulationFinished = true;
#endif
}

void FalconDevice::Init(){
#ifdef FALCON
    cThread *hapticsThread = new cThread();
    mSimulationRunning = true;
    hapticsLoopThisPtr = this;
    hapticsThread->set(hapticsLoopCallback, CHAI_THREAD_PRIORITY_HAPTICS);
#endif
}

void FalconDevice::getPosition(float &up, float &right, float &forward){
    up = 0;
    right = 0;
    forward = 0;
#ifdef FALCON
    if(!isConnected())
        return;
    cVector3d tempPos;
    mHapticDevice->getPosition(tempPos);
    up = tempPos.z;
    right = tempPos.y;
    forward = -tempPos.x;
#endif
}
void FalconDevice::getVelocity(float &up, float &right, float &forward){
    up = 0;
    right = 0;
    forward = 0;
#ifdef FALCON
    if(!isConnected())
        return;
    cVector3d tempVel;
    mHapticDevice->getLinearVelocity(tempVel);
    up = tempVel.z;
    right = tempVel.y;
    forward = -tempVel.x;
#endif
}
void FalconDevice::getForce(float &up, float &right, float &forward){
    up = 0;
    right = 0;
    forward = 0;
#ifdef FALCON
    if(!isConnected())
        return;
    cVector3d tempForce;
    mHapticDevice->getForce(tempForce);
    up = tempForce.z;
    right = tempForce.y;
    forward = -tempForce.x;
#endif
}

void FalconDevice::setVerticalForce(bool _isFalling){
#ifdef FALCON
    isFalling = _isFalling;
#endif
}

void FalconDevice::setHorizontalForce(float right, float forward){
#ifdef FALCON
    forcesToApply.push_back(cVector3d(-forward, right, FALCON_IMPULSE_LENGTH));
#endif
}

bool FalconDevice::isConnected(){
#ifdef FALCON
    if(isReady)
        return true;
    if(mHapticDevice != NULL){
        cVector3d position;
        mHapticDevice->getPosition(position);
        //The Falcon always has position (0.01,0,0) when it is not yet ready.
        static const int INITIAL_X = 1008981770;
        if(position.x < .0099998 || position.x > .0100001 || position.y != 0 || position.z != 0){
            isReady = true;
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}

void PressInputIfNotActive(UserInputIndex inputIndex, UserInput &input, uint32_t activeInputs){
    if(!(activeInputs & GEN_INPUT_MASK(inputIndex, true))){
        input.inputs |= GEN_INPUT_MASK(inputIndex, true);
        input.inputs &= ~GEN_INPUT_MASK(inputIndex, false);
    }
}

void ReleaseInputIfActive(UserInputIndex inputIndex, UserInput &input, uint32_t activeInputs){
    if(activeInputs & GEN_INPUT_MASK(inputIndex, true)){
        input.inputs &= ~GEN_INPUT_MASK(inputIndex, true);
        input.inputs |= GEN_INPUT_MASK(inputIndex, false);
    }
}

void FalconDevice::setInputs(UserInput &input, uint32_t activeInputs){
#ifdef FALCON
    //Falcon reference frame:
    //x increases out of screen
    //y increases right
    //z increases up
    cVector3d tempPos;
    mHapticDevice->getPosition(tempPos);
    input.falconInputs[FALCON_INPUT_FORWARD] = -tempPos.x/mHapticDeviceInfo.m_workspaceRadius;
    input.falconInputs[FALCON_INPUT_RIGHT] = tempPos.y/mHapticDeviceInfo.m_workspaceRadius;
    //NOTE: input.falconInput[FALCON_INPUT_UP] is not set!
    bool buttonIsOn;
    mHapticDevice->getUserSwitch(SWITCH_INDEX_GROW, buttonIsOn);
    if(buttonIsOn)
        PressInputIfNotActive(USERINPUT_INDEX_GROW, input, activeInputs);
    else
        ReleaseInputIfActive(USERINPUT_INDEX_GROW, input, activeInputs);
    
    mHapticDevice->getUserSwitch(SWITCH_INDEX_SHRINK, buttonIsOn);
    if(buttonIsOn)
        PressInputIfNotActive(USERINPUT_INDEX_SHRINK, input, activeInputs);
    else
        ReleaseInputIfActive(USERINPUT_INDEX_SHRINK, input, activeInputs);
#endif
}

/*
float FalconDevice::getWorkspaceRadius() const{
#ifdef FALCON
    return mHapticDeviceInfo.m_workspaceRadius;
#else
    return -1;
#endif
}
float FalconDevice::getMaxStiffness() const{
#ifdef FALCON
    return mHapticDeviceInfo.m_maxForceStiffness;
#else
    return -1;
#endif
}
*/

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

float sinusoid(float t){
    t = t*(2*M_PI)*FALCON_IMPULSE_LENGTH;
    return sin(5*t)/t;
}

void FalconDevice::hapticsLoop(){
#ifdef FALCON
    cPrecisionClock pclock;
    pclock.setTimeoutPeriodSeconds(1.0);
    pclock.start(true);
    int counter = 0;

	double prevTimestepTime = pclock.getCPUTimeSeconds();

    // main haptic simulation loop
    while(mSimulationRunning){
        if(!isConnected())
            continue;
        
		double newtime = pclock.getCPUTimeSeconds();

        cVector3d pos(0,0,0);
        cVector3d vel(0,0,0);
        cVector3d totalForce(0,0,0);
        if(!isFalling){
            mHapticDevice->getPosition(pos);
            totalForce.z += -FALCON_VERTICAL_STIFFNESS*pos.z;
            mHapticDevice->getLinearVelocity(vel);
            float dampenTerm = -FALCON_DAMPENING*vel.z;
            if((totalForce.z+dampenTerm)*totalForce.z < 0)
                totalForce.z = 0;
            else
                totalForce.z += dampenTerm;
        }
        for(int i = 0; i < forcesToApply.size(); ++i){
            cVector3d currForce = forcesToApply[i];
            cVector3d currForceXY(currForce.x, currForce.y, 0);
            totalForce += currForceXY*sinusoid(currForce.z);
            //remember currForce.z is timestamp
            currForce.z -= (newtime-prevTimestepTime);
            if(currForce.z < 0)
                forcesToApply.erase(forcesToApply.begin()+i);
        }
        mHapticDevice->setForce(totalForce);

        prevTimestepTime = newtime;
    }
    mSimulationFinished = true;
#endif
}

void hapticsLoopCallback(){
    hapticsLoopThisPtr->hapticsLoop();
}
