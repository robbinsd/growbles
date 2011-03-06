/*
 *  Player.cpp
 *  CS248-Final-Project
 *
 *  Created by Freeman Fan on 3/5/11.
 *  Copyright 2011 Stanford University. All rights reserved.
 *
 */

#include "Player.h"

Player::Player(SceneNode* playerSceneNode) : playerNode(playerSceneNode)
{
}

void
Player::Move(float x, float y, float z) {
    Matrix moveMatrix;
    moveMatrix.Translate(x, y, z);
    playerNode->ApplyTransform(moveMatrix);
}
