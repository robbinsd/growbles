#include "WorldModel.h"
#include "UserInput.h"
#include <string>
#include <sstream>

using std::vector;
using std::string;
using std::stringstream;

#define WORLDMESH_PATH "scenefiles/worldmesh.3ds"
#define ARMADILLO_PATH "scenefiles/armadillo.3ds"
#define SPHERE_PATH "scenefiles/sphere.3ds"

#define ARMADILLO_BASE_Y 3.3

/*
 * Helper function to move a rigid body to a certain location
 */
static void MoveRigidBody(btRigidBody* body, float x, float y, float z)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(btScalar(x), btScalar(y), btScalar(z)));
    body->setWorldTransform(transform);
}

void
WorldModel::Init(SceneGraph& sceneGraph)
{
    // Save parameters
    mSceneGraph = &sceneGraph;

    // Load the static parts of the scene into the scenegraph
    sceneGraph.LoadScene(WORLDMESH_PATH, "WorldMesh", &sceneGraph.rootNode);
    Matrix armTransform;
    armTransform.Translate(0.0, ARMADILLO_BASE_Y, 0.0);
    SceneNode* armParent = sceneGraph.AddNode(&sceneGraph.rootNode, armTransform,
                                              "armadilloParent");
    sceneGraph.LoadScene(ARMADILLO_PATH, "Armadillo", armParent);

    // Environment map
    Vector emapPos(0.0, 3.0 + ARMADILLO_BASE_Y, 0.0, 1.0);
    sceneGraph.FindMesh("Armadillo_0")->EnvironmentMap(emapPos);

    // Setup physics simulation
    broadphase = new btDbvtBroadphase();

    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);

    dynamicsWorld->setGravity(btVector3(0,-10,0));

    // Create the ground rigidBody
    //groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
    // 5 rings, ring 1 is the outermost and ring 5 is in the center
    float ringRadius = 15.0;
    for (int i=0; i<5; i++) {
        btCollisionShape* platformShape = new btCylinderShape(btVector3(ringRadius, 3, ringRadius));
        platformShapes.push_back(platformShape);
        
        btDefaultMotionState* platformMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,1,0)));
        
        btRigidBody::btRigidBodyConstructionInfo platformRigidBodyCI(0, platformMotionState, platformShape, btVector3(0,0,0));
        platformRigidBodyCI.m_friction = 0.5;
        platformRigidBodyCI.m_restitution = 0.1;
        btRigidBody* platformRigidBody = new btRigidBody(platformRigidBodyCI);
        dynamicsWorld->addRigidBody(platformRigidBody);
        platformRigidBodies.push_back(platformRigidBody);
        ringRadius -= 3.0;
    }
    
    // Create the temporary platform
    platform = new Platform(200);
    
    // Enable the debug drawer
    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    dynamicsWorld->setDebugDrawer(&debugDrawer);
}

WorldModel::~WorldModel()
{
    // Delete our player objects
    for (vector<Player*>::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        delete *it;

    // Destroy physics simulation
    // Destroy players
    for(unsigned i = 0; i < mPlayers.size(); ++i) {
        Player *player = mPlayers[i];
        assert(player);
        btRigidBody *playerRigidBody = mPlayerRigidBodies[player];
        dynamicsWorld->removeRigidBody(playerRigidBody);
        delete playerRigidBody;
        delete mPlayerShapes[player];
    }

    // Destroy platform
    for (int i=0; i<5; i++) {
        dynamicsWorld->removeRigidBody(platformRigidBodies[i]);
        delete platformRigidBodies[i]->getMotionState();
        delete platformRigidBodies[i];
        delete platformShapes[i];
    }
    
    // Destroy other physics stuff
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
    
    // Delete the temporary platform
    delete platform;
}

void
WorldModel::Step(unsigned numTicks)
{
    for (unsigned i = 0; i < numTicks; ++i)
        SingleStep();
}

void
WorldModel::SingleStep()
{
    // BOF step physics
    for (unsigned i = 0; i < BULLET_STEPS_PER_GROWBLE_STEP; ++i) {
        for(unsigned j = 0; j < mPlayers.size(); ++j)
            HandleInputForPlayer(mPlayers[j]->GetPlayerID());
        dynamicsWorld->stepSimulation(BULLET_STEP_INTERVAL, 1, BULLET_STEP_INTERVAL);
    }

    // Loop over players
    for(unsigned i = 0; i < mPlayers.size(); ++i){
        Player *player = mPlayers[i];
        assert(player);
        btTransform trans = mPlayerRigidBodies[player]->getWorldTransform();
        player->setTransform(trans);
    }

    // update platform position
    platform->update();

    // move the platform rigid bodies along with the rings
    int fallingRing = platform->getFallingRing();
    //std::cout << "falling ring: " << fallingRing << "\n";
    float fallingRingPos = platform->getFallingRingPos();
    MoveRigidBody(platformRigidBodies[fallingRing], 0.0, fallingRingPos+1.0, 0.0);

    // Update the current timestamp
    mCurrentTimestamp += 1;
}

void
WorldModel::GetState(WorldState& stateOut)
{
    // Get the number of players in play
    stateOut.numPlayers = mPlayers.size();
    
    // Get the ID, inputs and position of each of the players
    for (unsigned i=0; i<mPlayers.size(); i++) {
        PlayerInfo playerInfo;
        playerInfo.playerID = mPlayers[i]->GetPlayerID();
        playerInfo.activeInputs = mPlayers[i]->GetActiveInputs();
        playerInfo.transform = mPlayerRigidBodies[mPlayers[i]]->getWorldTransform();
        playerInfo.linearVel = mPlayerRigidBodies[mPlayers[i]]->getLinearVelocity();
        playerInfo.angularVel = mPlayerRigidBodies[mPlayers[i]]->getAngularVelocity();
        stateOut.playerArray[i] = playerInfo;
    }
    
    // Get platform state
    stateOut.pstate = platform->GetPlatformState();
    
    // Get the timestamp
    stateOut.timestamp = mCurrentTimestamp;
}

void
WorldModel::SetState(WorldState& stateIn)
{
    // Set the number of players
    unsigned numPlayers = stateIn.numPlayers;
    
    // Copy the player array
    PlayerInfo *playerArray = new PlayerInfo[numPlayers];
    memcpy(playerArray, stateIn.playerArray, numPlayers*sizeof(PlayerInfo));
    
    // Set info for each of the players
    for (unsigned i=0; i<numPlayers; i++) {

        // Add player to the client if they have not yet been added
        if (!GetPlayer(playerArray[i].playerID))
            AddPlayer(playerArray[i].playerID);

        // Get our local copy of the player
        Player* player = GetPlayer(playerArray[i].playerID);

        // Clean cached broadphase state
        if (dynamicsWorld->getBroadphase()->getOverlappingPairCache())
            dynamicsWorld->getBroadphase()
                         ->getOverlappingPairCache()
                         ->cleanProxyFromPairs(mPlayerRigidBodies[player]->getBroadphaseHandle(),
                                               dynamicsWorld->getDispatcher());

        // Physics
        mPlayerRigidBodies[player]->setWorldTransform(playerArray[i].transform);
        player->setTransform(playerArray[i].transform);
        mPlayerRigidBodies[player]->setLinearVelocity(playerArray[i].linearVel);
        mPlayerRigidBodies[player]->setAngularVelocity(playerArray[i].angularVel);

        // Active inputs
        player->SetActiveInputs(playerArray[i].activeInputs);
    }

    // Reset some cached state
    dynamicsWorld->getBroadphase()->resetPool(dynamicsWorld->getDispatcher());
    dynamicsWorld->getConstraintSolver()->reset();

    // Set the platform state
    platform->SetPlatformState(stateIn.pstate);

    mCurrentTimestamp = stateIn.timestamp;
    delete playerArray;
}

static float sInitialPositions[][3] = { {-8.0, 5.0, 0.0},
                                        {-4.0, 5.0, 4.0} };

void
WorldModel::AddPlayer(unsigned playerID)
{
    // Generate the initial position.
    //
    // We only have enough initial positions for two players. This can be trivially
    // fixed.
    unsigned posIndex = mPlayers.size();
    assert(posIndex <= 1);
    Vector initialPosition(sInitialPositions[posIndex][0],
                           sInitialPositions[posIndex][1],
                           sInitialPositions[posIndex][2],
                           0.0f);

    // Call the internal helper
    AddPlayer(playerID, initialPosition);
}

void
WorldModel::AddPlayer(unsigned playerID, Vector initialPosition)
{
    // Make sure we don't already have a player by this ID
    assert(GetPlayer(playerID) == NULL);

    // Add the player to the scenegraph
    Matrix playerTransform;
    //playerTransform.Translate(initialPosition.x, initialPosition.y, initialPosition.z);
    stringstream numSS;
    numSS << playerID;
    string nodeName = string("PlayerNode_") + numSS.str();
    string rootName = string("PlayerRoot_") + numSS.str();
    SceneNode* playerNode = mSceneGraph->AddNode(&mSceneGraph->rootNode,
                                                 playerTransform,
                                                 nodeName.c_str());
    mSceneGraph->LoadScene(SPHERE_PATH, rootName.c_str(), playerNode);

    // Initialize the model representation of the player
    Player* player = new Player(playerID, playerNode, initialPosition);

    // Add it to our list of players
    mPlayers.push_back(player);

    // Create the player rigidBody
    btCollisionShape* playerShape = new btSphereShape(1);
    mPlayerShapes[player] = playerShape;

    btScalar playerMass = 1;
    btVector3 playerInertia(0, 0, 0);
    playerShape->calculateLocalInertia(playerMass,playerInertia);

    btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(playerMass, NULL, playerShape, playerInertia);
    playerRigidBodyCI.m_startWorldTransform = btTransform(btQuaternion(0,0,0,1), btVector3(initialPosition.x,
                                                                                           initialPosition.y,
                                                                                           initialPosition.z));
    playerRigidBodyCI.m_friction = 0.5;
    playerRigidBodyCI.m_restitution = 0.1;
    playerRigidBodyCI.m_angularDamping = 0.5;
    btRigidBody *playerRigidBody = new btRigidBody(playerRigidBodyCI);
    mPlayerRigidBodies[player] = playerRigidBody;
    playerRigidBody->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(playerRigidBody);
}

Player*
WorldModel::GetPlayer(unsigned playerID)
{
    // Search our list of players
    for (vector<Player*>::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        if ((*it)->GetPlayerID() == playerID)
            return *it;

    // None found. Return null.
    return NULL;
}

Vector
WorldModel::GetPlayerPosition(unsigned playerID)
{
    Player* player = GetPlayer(playerID);
    assert(player);
    btTransform trans = mPlayerRigidBodies[player]->getWorldTransform();
    Vector rv(trans.getOrigin());
    return rv;
}

void
WorldModel::ApplyInput(UserInput& input)
{
    // Get the player the input applies to
    Player* player = GetPlayer(input.playerID);
    assert(player);

    // Apply it
    player->applyInput(input);
}

void
WorldModel::HandleInputForPlayer(unsigned playerID)
{
    // Get the referenced player
    Player* player = GetPlayer(playerID);
    assert(player);
    btRigidBody* playerRigidBody = mPlayerRigidBodies[player];

    // Get the inputs
    uint32_t activeInputs = player->GetActiveInputs();

    if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_UP, true))
        playerRigidBody->applyForce(btVector3(1.0, 0.0, 0.0),
                                    btVector3(1.0, 1.0, 1.0));
    if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_DOWN, true))
        playerRigidBody->applyForce(btVector3(-1.0, 0.0, 0.0),
                                    btVector3(1.0, 1.0, 1.0));
    if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_LEFT, true))
        playerRigidBody->applyForce(btVector3(0.0, 0.0, -1.0),
                                    btVector3(1.0, 1.0, 1.0));
    if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_RIGHT, true))
        playerRigidBody->applyForce(btVector3(1.0, 0.0, 1.0),
                                    btVector3(1.0, 1.0, 1.0));
}


