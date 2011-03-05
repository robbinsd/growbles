#ifndef WORLDMODEL_H
#define WORLDMODEL_H
#include "SceneGraph.h"


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
     * Inputs
     */
    void GrowPlayer(unsigned playerID);
    void ShrinkPlayer(unsigned playerID);

    protected:

    // The scenegraph associated with this world
    SceneGraph* mSceneGraph;
};

#endif /* WORLDMODEL_H */
