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
#ifdef _WIN32
#include <stdint.h>
#endif

class UserInput;

class Player {
    
public:
    
    /*
     * constructor
     */
    Player(unsigned playerID, SceneNode* playerSceneNode, Vector initialPosition, 
        Matrix initialRotation);
    
    /*
     * move the player to a specified location
     */
    void setPosition(Vector pos);
    
    /*
     * Get the current position of the player
     */
    Vector getPosition();
    
    /*
     * move the player to a specified location
     */
    void setRotation(Matrix rotation);
    
    /*
     * Get the current position of the player
     */
    Matrix getRotation();

    /*
     * Apply an input.
     */
    void applyInput(UserInput& input);

    /*
     * Gets the ID of this player.
     */
    unsigned GetPlayerID() { return mPlayerID; } ;

    /*
     * Get the active inputs.
     */
    uint32_t GetActiveInputs() { return mActiveInputs; };

protected:

    // Updates player's transformation matrix.
    // Used after setting the player's position and/or rotation.
    void updateTransform();

    // The ID of the player
    unsigned mPlayerID;

    // the node in the scene that contains the mesh for the player
    SceneNode* mPlayerNode;

    // The current position of the player
    Vector mPosition;

    // Quaternion representing player's rotation
    Matrix mRotation;

    // The current active inputs applied to this player.
    // This is a bitfield of the USERINPUT_* variety, with only begin
    // bits defined.
    uint32_t mActiveInputs;
};

#endif /* PLAYER_H */
