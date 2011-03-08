#ifndef WORLDMODEL_H
#define WORLDMODEL_H

#include "SceneGraph.h"
#include "Player.h"
#include <vector>

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
     * Destructor.
     */
    ~WorldModel();

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
     * Adds a player to the world.
     */
    void AddPlayer(unsigned playerID, Vector initialPosition);

    /*
     * Gets a player by ID.
     *
     * Returns NULL if none is found.
     */
    Player* GetPlayer(unsigned playerID);

    /*
     * Inputs
     */
    void GrowPlayer(unsigned playerID);
    void ShrinkPlayer(unsigned playerID);
    void MovePlayer(unsigned playerID, int direction);

    protected:

    // The scenegraph associated with this world
    SceneGraph* mSceneGraph;

    // The players
    std::vector<Player*> mPlayers;
    
    // Physics Simulation
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    // Physics properties of the platform
    btCollisionShape* groundShape;
    btRigidBody* groundRigidBody;
    // Physics properties of the player
    btCollisionShape* playerShape;
    btRigidBody* playerRigidBody;
    // Physics properties of the other player
    btCollisionShape* otherPlayerShape;
    btRigidBody* otherPlayerRigidBody;
};

#endif /* WORLDMODEL_H */
