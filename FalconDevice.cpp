#include "FalconDevice.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <fstream>

void hapticsLoopCallback();
static FalconDevice *hapticsLoopThisPtr = NULL;
const char *FALCON_USED_DUMMY_FILE = "falconUseData.dat";

//DON"T CHANGE THE ORDER OF THESE
enum SwitchIndexT{  SWITCH_INDEX_JUMP=0, SWITCH_INDEX_SHRINK, SWITCH_INDEX_DASH,
    SWITCH_INDEX_GROW };

bool isAlreadyInUse(){
    /*bool toReturn = true;
    ifstream fileReader(FALCON_USED_DUMMY_FILE);
    if(!fileReader.good())
        toReturn = false;
    else{
        char a = 0;
        fileReader.read(&a, 1);
        if(a != '1')
            toReturn = false;
    }
    fileReader.close();
    return toReturn;*/
    return false;
}

void setAlreadyInUse(bool isInUse){
    /*ofstream fileWriter(FALCON_USED_DUMMY_FILE);
    if(fileWriter.good())
        if(isInUse)
            fileWriter.write("1", 1);
        else
            fileWriter.write("0", 1);
    fileWriter.close();*/
}

FalconDevice::FalconDevice(){
    isReady = false;
#ifdef FALCON
    //prevHeight = 0;
    height = 0;
    mSimulationRunning = false;
    mSimulationFinished = false;
    beingUsed = false;
    forceCount = 0;
    force = cVector3d(0,0,0);
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
    bool isAvail = mHapticDevice->isSystemAvailable();
    bool isReady = mHapticDevice->isSystemReady();
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
    if(beingUsed)
        setAlreadyInUse(false);
#endif
}

void FalconDevice::Init(){
#ifdef FALCON
    if(!isAlreadyInUse()){
        beingUsed = true;
        setAlreadyInUse(true);
        cThread *hapticsThread = new cThread();
        mSimulationRunning = true;
        hapticsLoopThisPtr = this;
        hapticsThread->set(hapticsLoopCallback, CHAI_THREAD_PRIORITY_HAPTICS);
    }
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

void FalconDevice::setVerticalForce(float _height){
#ifdef FALCON
    if(_height > 1)
        _height = 1;
    else if(_height < -1)
        _height = -1;
    //prevHeight = height;
    height = _height*mHapticDeviceInfo.m_workspaceRadius;
#endif
}

void FalconDevice::setHorizontalForce(float right, float forward){
#ifdef FALCON
    //forcesToApply.push_back(cVector3d(-forward, right, FALCON_IMPULSE_LENGTH));
    force = 10*cVector3d(-forward, 0, right);
    forceCount = 0;
#endif
}

bool FalconDevice::isConnected(){
#ifdef FALCON
    if(!beingUsed)
        return false;
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
    
    mHapticDevice->getUserSwitch(SWITCH_INDEX_JUMP, buttonIsOn);
    if(buttonIsOn)
        PressInputIfNotActive(USERINPUT_INDEX_JUMP, input, activeInputs);
    else
        ReleaseInputIfActive(USERINPUT_INDEX_JUMP, input, activeInputs);
    
    mHapticDevice->getUserSwitch(SWITCH_INDEX_DASH, buttonIsOn);
    if(buttonIsOn)
        PressInputIfNotActive(USERINPUT_INDEX_DASH, input, activeInputs);
    else
        ReleaseInputIfActive(USERINPUT_INDEX_DASH, input, activeInputs);
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
        
        mHapticDevice->getPosition(pos);
        totalForce.z += -FALCON_VERTICAL_STIFFNESS*(pos.z-height);
        mHapticDevice->getLinearVelocity(vel);
        float dampenTerm = -FALCON_DAMPENING*vel.z;
        if((totalForce.z+dampenTerm)*totalForce.z < 0)
            totalForce.z = 0;
        else
            totalForce.z += dampenTerm;
        
        /*for(int i = 0; i < forcesToApply.size(); ++i){
            cVector3d currForce = forcesToApply[i];
            cVector3d currForceXY(currForce.x, currForce.y, 0);
            totalForce += currForceXY*sinusoid(currForce.z);
            //remember currForce.z is timestamp
            currForce.z -= (newtime-prevTimestepTime);
            if(currForce.z < 0)
                forcesToApply.erase(forcesToApply.begin()+i);
        }*/
        totalForce += force*(1-(forceCount/5));
        forceCount ++;
        if(forceCount >= 5)
            force = cVector3d(0,0,0);
        
        totalForce.x += -FALCON_HORIZONTAL_STIFFNESS*pos.x;
        totalForce.y += -FALCON_HORIZONTAL_STIFFNESS*pos.y;
        
        mHapticDevice->setForce(totalForce);

        prevTimestepTime = newtime;
#ifdef _WIN32
        Sleep(1);
#endif
    }
    mSimulationFinished = true;
#endif
}

void hapticsLoopCallback(){
    hapticsLoopThisPtr->hapticsLoop();
}
