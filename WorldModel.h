#ifndef WORLDMODEL_H
#define WORLDMODEL_H

#include "SceneGraph.h"
#include "Platform.h"
#include "GLDebugDrawer.h"
#include "Player.h"
#include "Gameclock.h"
#include "UserInput.h"
#include <vector>

class SceneGraph;
class UserInput;

// struct containing information about a player
struct PlayerInfo {
    unsigned playerID;
    uint32_t activeInputs;

    // physics
    btTransform transform;
    btVector3 linearVel;
    btVector3 angularVel;
};

// Struct containing all mutable world state
struct WorldState {

    // We want some values here so that this structure
    // takes up room in order to test the network code.
    WorldState(){};
    
    // The array of players, increase the size to allow more players
    PlayerInfo playerArray[2];
    
    // The number of players in play
    int numPlayers;

    // Timestamp of this worldstate
    unsigned timestamp;
};

class WorldModel {

    public:

    /*
     * Dummy constructor.
     */
    WorldModel() : mCurrentTimestamp(0) {};

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
    void Step(unsigned numTicks);

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
     * Applies inputs.
     */
    void ApplyInput(UserInput& input);

    /*
     * Gets the current timestamp.
     */
    unsigned GetCurrentTimestamp() { return mCurrentTimestamp; };
    
    /*
     * Gets the platform for debugging
     */
    Platform* GetPlatform() { return platform; };
    
    /*
     * Gets the dynamicsWorld for physics debug drawing
     */
    btDiscreteDynamicsWorld* GetDynamicsWorld() { return dynamicsWorld; };

    protected:

    /*
     * Internal-only method. Adds a player at a specified position.
     */
    void AddPlayer(unsigned playerID, Vector position);

    /*
     * Applies forces for the current inputs.
     */
    void HandleInputForPlayer(unsigned playerID);

    // The scenegraph associated with this world
    SceneGraph* mSceneGraph;

    // The players
    std::vector<Player*> mPlayers;
    
    // Physics properties of each player
    std::map<Player *, btCollisionShape*> mPlayerShapes;
    std::map<Player *, btRigidBody*> mPlayerRigidBodies;
    // Physics Simulation
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    // Physics properties of the platform
    std::vector<btCollisionShape*> platformShapes;
    std::vector<btRigidBody*> platformRigidBodies;
    
    // The platform
    Platform* platform;
    
    // Debug drawer
    GLDebugDrawer debugDrawer;

    // Current timestamp
    unsigned mCurrentTimestamp;
};

#endif /* WORLDMODEL_H */
