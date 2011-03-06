/*
 *  Player.h
 *  CS248-Final-Project
 *
 *  Created by Freeman Fan on 3/5/11.
 *  Copyright 2011 Stanford University. All rights reserved.
 *
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "Framework.h"
#include "SceneGraph.h"

class Player {
    
public:
    
    /*
     * constructor
     */
    Player(SceneNode* playerSceneNode, float xin, float yin, float zin);
    
    /*
     * move the player by a translation vector
     */
    void move(float xin, float yin, float zin);
    
    /*
     * move the player to a specified location
     */
    void moveTo(float xin, float yin, float zin);
    
    // the node in the scene that contains the mesh for the player
    SceneNode* playerNode;
    
    // The current position of the player
    float x, y, z;
    
    float getX();
    
    float getY();
    
    float getZ();
};

#endif /* PLAYER_H */