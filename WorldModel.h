#ifndef WORLDMODEL_H
#define WORLDMODEL_H

#include "SceneGraph.h"
#include "Platform.h"
#include "GLDebugDrawer.h"
#include "Player.h"
#include "Gameclock.h"
#include "UserInput.h"
#include "FalconDevice.h"
#include <vector>
#include <map>
using namespace std;

#define BULLET_STEP_INTERVAL (1.0/60.0)
#define BULLET_STEPS_PER_GROWBLE_STEP 6

class SceneGraph;
class UserInput;

const double PLAYER_SCALING_RATE = .01;
const double PLAYER_MAXIMUM_SCALE = 3;
const double PLAYER_MINIMUM_SCALE = 0.8;
const double PLAYER_MAX_FORCE = 2;
const double PLAYER_MASS_DENSITY = 1;

// struct containing information about a player
struct PlayerInfo {
    btTransform transform;
    btVector3 linearVel;
    btVector3 angularVel;
    Vector activeFalconInputs;
    unsigned playerID;
    uint32_t activeInputs;
    float scale;
    unsigned packingDummy;
};

// Struct containing all mutable world state
struct WorldState {

    // We want some values here so that this structure
    // takes up room in order to test the network code.
    WorldState(){};
    
    // The array of players, increase the size to allow more players
    PlayerInfo playerArray[3];
    
    // The number of players in play
    int numPlayers;
    
    // The platform state
    platformState pstate;

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
     * @param falcon: the falcon controlled by this player
     * @param playerID: this ID of the player on this computer
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
    void SingleStep();

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
     * Gets the position of a player.
     */
    Vector GetPlayerPosition(unsigned playerID);

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

    /*
     * Set the falcon controlled by this computer
     */
    void SetFalcon(FalconDevice &falcon){ mFalcon = &falcon; };

    /*
     * Set the player controlling this computer.
     */
    void SetThisPlayer(int playerID){ mPlayerID = playerID; };

    /*
     * Respond to collision between the player on this computer and
     * the player specified by otherPlayerID
     * Apply a haptic impulse force to the falcon controlled by this player.
     */
    void ApplyHapticCollisionForce();

    /*
     * Apply gravity force to  the falcon controlled by the player on this
     * computer.
     */
    void ApplyHapticGravityForce();

    protected:

    /*
     * Internal-only method. Adds a player at a specified position.
     */
    void AddPlayer(unsigned playerID, Vector position);

    /*
     * Applies forces for the current inputs.
     */
    void HandleInputForPlayer(unsigned playerID);
    void HandleKinematicInputForPlayer(unsigned playerID);

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
    
    //this player (the one running this process)
    int mPlayerID;

    //falcon controlled by this player
    FalconDevice *mFalcon;
    
    // Debug drawer
    GLDebugDrawer debugDrawer;

    // Current timestamp
    unsigned mCurrentTimestamp;
    
    friend class Game;
    
    // SceneNodes for the platform rings
    SceneNode* platformNodes[5];
    
    sf::SoundBuffer Buffer;
    sf::Sound Sound;
    sf::SoundBuffer Buffer2;
    sf::Sound Sound2;
};
// used by Falcon
static map<btCollisionDispatcher *, WorldModel *> worldModels;

#endif /* WORLDMODEL_H */
