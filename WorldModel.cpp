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
}

WorldModel::~WorldModel()
{
    // Delete our player objects
    for (vector<Player*>::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        delete *it;
    
    // Destroy physics simulation
	for(int i = 0; i < mPlayers.size(); ++i){
		Player *player = mPlayers[i];
		assert(player);
		btRigidBody *playerRigidBody = mPlayerRigidBodies[player];
		dynamicsWorld->removeRigidBody(playerRigidBody);
		delete playerRigidBody->getMotionState();
		delete playerRigidBody;
		delete mPlayerShapes[player];
	}
    
    dynamicsWorld->removeRigidBody(groundRigidBody);
    delete groundRigidBody->getMotionState();
    delete groundRigidBody;
    
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
    // BOF step physics
    dynamicsWorld->stepSimulation(1/60.f, 10);
	
    // Loop over players
	for(int i = 0; i < mPlayers.size(); ++i){
		Player *player = mPlayers[i];
		assert(player);
		btTransform trans;
		mPlayerRigidBodies[player]->getMotionState()->getWorldTransform(trans);
		Vector playerPos(trans.getOrigin());
		player->moveTo(playerPos);
	}
    
    //std::cout << "sphere x: " << trans.getOrigin().getX() << std::endl;
    // EOF step physics
}

void
WorldModel::GetState(WorldState& stateOut)
{
}

void
WorldModel::SetState(WorldState& stateIn)
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

static float sInitialPositions[][3] = { {-8.0, 2.0, 0.0},
                                        {-4.0, 2.0, 4.0} };

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
    btDefaultMotionState* playerMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),
		btVector3(initialPosition.x,initialPosition.y,initialPosition.z)));
    
    btScalar playerMass = 1;
    btVector3 playerInertia(0, 0, 0);
    playerShape->calculateLocalInertia(playerMass,playerInertia);
    
    btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(playerMass, playerMotionState, playerShape, playerInertia);
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

void
WorldModel::MovePlayer(unsigned playerID, int direction)
{
    // Get the referenced player
    Player* player = GetPlayer(playerID);
    assert(player);
	btRigidBody* playerRigidBody = mPlayerRigidBodies[player];

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

