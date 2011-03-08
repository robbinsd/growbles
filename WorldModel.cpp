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
    groundShape = new btStaticPlaneShape(btVector3(0,2,0),1);
    
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));
    
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
    groundRigidBodyCI.m_friction = 0.5;
    groundRigidBodyCI.m_restitution = 0.1;
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamicsWorld->addRigidBody(groundRigidBody);
    
    // Create the player rigidBody
    playerShape = new btSphereShape(1);
    btDefaultMotionState* playerMotionState =
    new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(-8.0, 50.0, 0.0)));
    
    btScalar playerMass = 1;
    btVector3 playerInertia(0, 0, 0);
    playerShape->calculateLocalInertia(playerMass,playerInertia);
    
    btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(playerMass, playerMotionState, playerShape, playerInertia);
    playerRigidBodyCI.m_friction = 0.5;
    playerRigidBodyCI.m_restitution = 0.1;
    playerRigidBodyCI.m_angularDamping = 0.5;
    playerRigidBody = new btRigidBody(playerRigidBodyCI);
    playerRigidBody->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(playerRigidBody);
    
    // Create the other player's rigidBody
    otherPlayerShape = new btSphereShape(1);
    btDefaultMotionState* otherPlayerMotionState =
    new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(-4.0, 30.0, 4.0)));
    otherPlayerShape->calculateLocalInertia(playerMass,playerInertia);
    
    btRigidBody::btRigidBodyConstructionInfo otherPlayerRigidBodyCI(playerMass, otherPlayerMotionState, otherPlayerShape, playerInertia);
    otherPlayerRigidBodyCI.m_friction = 0.5;
    otherPlayerRigidBodyCI.m_restitution = 0.1;
    otherPlayerRigidBodyCI.m_angularDamping = 0.5;
    otherPlayerRigidBody = new btRigidBody(otherPlayerRigidBodyCI);
    otherPlayerRigidBody->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addRigidBody(otherPlayerRigidBody);
}

WorldModel::~WorldModel()
{
    // Delete our player objects
    for (vector<Player*>::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        delete *it;
    
    // Destroy physics simulation
    dynamicsWorld->removeRigidBody(playerRigidBody);
    delete playerRigidBody->getMotionState();
    delete playerRigidBody;
    
    dynamicsWorld->removeRigidBody(otherPlayerRigidBody);
    delete otherPlayerRigidBody->getMotionState();
    delete otherPlayerRigidBody;
    
    dynamicsWorld->removeRigidBody(groundRigidBody);
    delete groundRigidBody->getMotionState();
    delete groundRigidBody;
    
    delete playerShape;
    delete otherPlayerShape;
    delete groundShape;
    
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
}

void
WorldModel::Step()
{
    // Get players
    Player* player = GetPlayer(1);
    assert(player);
    
    Player* otherPlayer = GetPlayer(2);
    assert(otherPlayer);
    
    // BOF step physics
    dynamicsWorld->stepSimulation(1/60.f, 10);
    
    btTransform trans;
    playerRigidBody->getMotionState()->getWorldTransform(trans);
    Vector playerPos(trans.getOrigin());
    player->moveTo(playerPos);
    
    btTransform otherTrans;
    otherPlayerRigidBody->getMotionState()->getWorldTransform(otherTrans);
    Vector otherPlayerPos(otherTrans.getOrigin());
    otherPlayer->moveTo(otherPlayerPos);
    
    //std::cout << "sphere x: " << trans.getOrigin().getX() << std::endl;
    // EOF step physics
}

void GetState(WorldState& stateOut)
{
}

void SetState(WorldState& stateIn)
{
}

void
WorldModel::GrowPlayer(unsigned playerID)
{
}

void
WorldModel::ShrinkPlayer(unsigned playerID)
{
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

void
WorldModel::MovePlayer(unsigned playerID, int direction)
{
    // Get the referenced player
    Player* player = GetPlayer(playerID);
    assert(player);

    switch (direction) {
        case USERINPUT_MASK_UP:
            playerRigidBody->applyForce(btVector3(1.0, 0.0, 0.0), btVector3(1.0, 1.0, 1.0));
            break;
        case USERINPUT_MASK_DOWN:
            playerRigidBody->applyForce(btVector3(-1.0, 0.0, 0.0), btVector3(1.0, 1.0, 1.0));
            break;
        case USERINPUT_MASK_LEFT:
            playerRigidBody->applyForce(btVector3(0.0, 0.0, -1.0), btVector3(1.0, 1.0, 1.0));
            break;
        case USERINPUT_MASK_RIGHT:
            playerRigidBody->applyForce(btVector3(1.0, 0.0, 1.0), btVector3(1.0, 1.0, 1.0));
            break;
        default:
            break;
    }
}

