/*
 *  Player.cpp
 *  CS248-Final-Project
 *
 *  Created by Freeman Fan on 3/5/11.
 *  Copyright 2011 Stanford University. All rights reserved.
 *
 */

#include "Player.h"

Player::Player(SceneNode* playerSceneNode, float xin, float yin, float zin) : playerNode(playerSceneNode),
                                                                              x(0.0),
                                                                              y(0.0),
                                                                              z(0.0)
{
    this->moveTo(xin, yin, zin);
}

void
Player::move(float xin, float yin, float zin) {
    Matrix moveMatrix;
    moveMatrix.Translate(xin, yin, zin);
    playerNode->applyTransform(moveMatrix);
    x += xin;
    y += yin;
    z += zin;
}

void
Player::moveTo(float xin, float yin, float zin) {
    float moveX = xin - x;
    float moveY = yin - y;
    float moveZ = zin - z;
    this->move(moveX, moveY, moveZ);
}

float
Player::getX() {
    return x;
}

float
Player::getY() {
    return y;
}

float
Player::getZ() {
    return z;
}
