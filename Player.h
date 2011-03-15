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
    void setTransform(const btTransform &transform);

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
    
    /*
     * Get the active falcon inputs.
     */
    Vector GetActiveFalconInputs() { return activeFalconInputs; };
    void SetActiveFalconInputs(Vector inputs) { activeFalconInputs = inputs; };
    
    int GetWinLossState() { return winLossState; };
    void SetWinLossState(int wlstate) { winLossState = wlstate; };

    protected:

    // The ID of the player
    unsigned mPlayerID;

    // the node in the scene that contains the mesh for the player
    SceneNode* mPlayerNode;

    // The current active inputs applied to this player.
    // This is a bitfield of the USERINPUT_* variety, with only begin
    // bits defined.
    uint32_t activeInputs;

    // The current inputs from the falcon. Each float is equivalent
    // to two bits of activeInputs, but has much more precision.
    Vector activeFalconInputs;
    
    // Indicates whether a player has won or lost a game
    // 0:Neither 1:Lost 2:Won
    int winLossState;
};

#endif /* PLAYER_H */
