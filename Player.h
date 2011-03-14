#ifndef PLAYER_H
#define PLAYER_H

#include "Framework.h"
#include "SceneGraph.h"
#include <stdint.h>

class UserInput;

class Player {
    
public:
    
    /*
     * constructor
     */
    Player(unsigned playerID, SceneNode* playerSceneNode, Vector initialPosition);

    /*
     * Sets the player location based on a btTransform.
     */
    void setTransform(btTransform transform);

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
    uint32_t GetActiveInputs() { return activeInputs; };
    void SetActiveInputs(uint32_t inputs) { activeInputs = inputs; };

    protected:

    // The ID of the player
    unsigned mPlayerID;

    // the node in the scene that contains the mesh for the player
    SceneNode* mPlayerNode;

    // The current active inputs applied to this player.
    // This is a bitfield of the USERINPUT_* variety, with only begin
    // bits defined.
    uint32_t activeInputs;
};

#endif /* PLAYER_H */
