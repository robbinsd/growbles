/*
 *  Player.cpp
 *  CS248-Final-Project
 *
 *  Created by Freeman Fan on 3/5/11.
 *  Copyright 2011 Stanford University. All rights reserved.
 *
 */

#include "Player.h"
#include "UserInput.h"

Player::Player(unsigned playerID,
               SceneNode* playerSceneNode,
               Vector initialPosition,
               Matrix initialRotation) : mPlayerID(playerID)
                                       , mPlayerNode(playerSceneNode)
                                       , mRotation()
                                       , mActiveInputs(0)
{
    this->setPosition(initialPosition);
    this->setRotation(initialRotation);
}

void
Player::updateTransform(){
    Matrix translationMatrix;
    translationMatrix.Translate(mPosition.x, mPosition.y, mPosition.z);
    mPlayerNode->LoadIdentityTransform();
    mPlayerNode->ApplyTransform(translationMatrix);
    mPlayerNode->ApplyTransform(mRotation);
}

void
Player::setPosition(Vector pos) {
    mPosition = pos;
    updateTransform();
}

Vector
Player::getPosition() {
    return mPosition;
}

void 
Player::setRotation(Matrix rotation){
    mRotation = rotation;
    updateTransform();
}

Matrix
Player::getRotation(){
    return mRotation;
}

void
Player::applyInput(UserInput& input)
{
    // Separate our inputs into those that begin an action and those
    // that end an action.
    uint32_t begins = input.inputs & 0x55555555;
    uint32_t ends = input.inputs & 0xAAAAAAAA;

    // Sanity check - We shouldn't begin and end the same input
    assert(((ends >> 1) & begins) == 0);

    // Sanity check - We shouldn't begin anything already begun
    // NOTE - this is disabled because operating systems actually send multiple
    // KeyPressed events after a certain delay when holding down a key.
    // assert((begins & activeInputs) == 0);

    // Sanity check - We shouldn't end anything not begun
    assert(((ends >> 1) | mActiveInputs) == mActiveInputs);

    // Apply our begins
    mActiveInputs |= begins;

    // Apply our ends
    mActiveInputs &= ~(ends >> 1);
}
