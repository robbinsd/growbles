/*
 *  Player.cpp
 *  CS248-Final-Project
 *
 *  Created by Freeman Fan on 3/5/11.
 *  Copyright 2011 Stanford University. All rights reserved.
 *
 */

#include "Player.h"

Player::Player(unsigned playerID,
               SceneNode* playerSceneNode,
               Vector initialPosition) : mPlayerID(playerID)
                                       , mPlayerNode(playerSceneNode)
                                       , position(Vector(0.0, 0.0, 0.0, 0.0))
{
    this->moveTo(initialPosition);
}

void
Player::move(Vector moveVec) {
    Matrix moveMatrix;
    moveMatrix.Translate(moveVec.x, moveVec.y, moveVec.z);
    mPlayerNode->ApplyTransform(moveMatrix);
    position = position + moveVec;
}

void
Player::moveTo(Vector pos) {
    this->move(pos - position);
}

Vector
Player::getPosition() {
    return position;
}
