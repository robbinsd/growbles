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
    Player(unsigned playerID, SceneNode* playerSceneNode);
    
    /*
     * move the player
     */
    void Move(float x, float y, float z);

    /*
     * Gets the ID of this player.
     */
    unsigned GetPlayerID() { return mPlayerID; } ;

    protected:

    // The ID of the player
    unsigned mPlayerID;

    // the node in the scene that contains the mesh for the player
    SceneNode* mPlayerNode;
};

#endif /* PLAYER_H */
