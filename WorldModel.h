#ifndef WORLDMODEL_H
#define WORLDMODEL_H

#include "Player.h"

#define USERINPUT_MASK_UP (1 << 2)
#define USERINPUT_MASK_DOWN (1 << 3)
#define USERINPUT_MASK_LEFT (1 << 4)
#define USERINPUT_MASK_RIGHT (1 << 5)

class SceneGraph;

// Struct containing all mutable world state
struct WorldState {

};

class WorldModel {

    public:

    /*
     * Initializes the world model.
     */
    void Init(SceneGraph& sceneGraph);

    /*
     * Steps the model forward in time.
     */
    void Step();

    /*
     * Get/Set world state. Allows for rewinding.
     */
    void GetState(WorldState& stateOut);
    void SetState(WorldState& stateIn);
    
    /*
     * Set the player to the specified player object
     */
    void SetPlayer(Player* p);


    /*
     * Inputs
     */
    void GrowPlayer(unsigned playerID);
    void ShrinkPlayer(unsigned playerID);
    void MovePlayer(unsigned playerID, int direction);

    protected:

    // The scenegraph associated with this world
    SceneGraph* mSceneGraph;
    
    // player
    Player* player;
};

#endif /* WORLDMODEL_H */
