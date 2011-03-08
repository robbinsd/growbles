#ifndef WORLDMODEL_H
#define WORLDMODEL_H
#include "SceneGraph.h"

#include "SceneGraph.h"
#include "Player.h"
#include <vector>

class SceneGraph;

// Struct containing all mutable world state
struct WorldState {

    // We want some values here so that this structure
    // takes up room in order to test the network code.
    WorldState() : dummy1(12), dummy2(33) {};
    unsigned dummy1;
    unsigned dummy2;
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
     *
     * This places the players where it pleases, and as such should only
     * be called on the server (where the world is generated). Everybody else
     * should receive state dumps from the server.
     */
    void AddPlayer(unsigned playerID);

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

    /*
     * Internal-only method. Adds a player at a specified position.
     */
    void AddPlayer(unsigned playerID, Vector position);

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
