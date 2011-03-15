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
    worldModels[dispatcher] = this;

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
    platform = new Platform(1000);
    
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
    for(unsigned j = 0; j < mPlayers.size(); ++j) HandleKinematicInputForPlayer(mPlayers[j]->GetPlayerID());
    
    // BOF step physics
    for (unsigned i = 0; i < BULLET_STEPS_PER_GROWBLE_STEP; ++i) {
        for(unsigned j = 0; j < mPlayers.size(); ++j)
            HandleInputForPlayer(mPlayers[j]->GetPlayerID());
        dynamicsWorld->stepSimulation(BULLET_STEP_INTERVAL, 1, BULLET_STEP_INTERVAL);
    }
    
    // Determine if a collision has taken place
    int numManifolds = dispatcher->getNumManifolds();
    for (int i=0;i<numManifolds;i++)
    {
        btPersistentManifold* contactManifold =  dispatcher->getManifoldByIndexInternal(i);
        btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
        
		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance()<0.1)
			{
                // If both collision shapes are 'SPHERE'
                if (!strcmp(obA->getCollisionShape()->getName(), obB->getCollisionShape()->getName())) {
                    
                    // A collision just happened
                    //DPS("COLLISION");
                    //DPF(pt.getDistance());
                }
				//const btVector3& ptA = pt.getPositionWorldOnA();
				//const btVector3& ptB = pt.getPositionWorldOnB();
				//const btVector3& normalOnB = pt.m_normalWorldOnB;
			}
		}
        
    }
    // EOF Determine if a collision has taken place

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
        playerInfo.activeFalconInputs = mPlayers[i]->GetActiveFalconInputs();
        playerInfo.transform = mPlayerRigidBodies[mPlayers[i]]->getWorldTransform();
        playerInfo.linearVel = mPlayerRigidBodies[mPlayers[i]]->getLinearVelocity();
        playerInfo.angularVel = mPlayerRigidBodies[mPlayers[i]]->getAngularVelocity();
        playerInfo.scale = mPlayerShapes[mPlayers[i]]->getLocalScaling().x();
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

        //Scale
        float scale = playerArray[i].scale;
        mPlayerShapes[player]->setLocalScaling(btVector3(scale,scale,scale));

        // Active inputs
        player->SetActiveInputs(playerArray[i].activeInputs);

        // Active Falcon inputs
        player->SetActiveFalconInputs(playerArray[i].activeFalconInputs);
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
                                        {-4.0, 5.0, 4.0},
                                        {4.0, 5.0, -4.0}};

void
WorldModel::AddPlayer(unsigned playerID)
{
    // Generate the initial position.
    //
    // We only have enough initial positions for two players. This can be trivially
    // fixed.
    unsigned posIndex = mPlayers.size();
    assert(posIndex <= 2);
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

	btScalar playerScale = 1;
    btScalar playerMass = PLAYER_MASS_DENSITY *playerScale*playerScale;
    btVector3 playerInertia(0, 0, 0);
    playerShape->calculateLocalInertia(playerMass,playerInertia);

    btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(playerMass, NULL, playerShape, playerInertia);
    playerRigidBodyCI.m_startWorldTransform = btTransform(btQuaternion(0,0,0,1), btVector3(initialPosition.x,
                                                                                           initialPosition.y,
                                                                                           initialPosition.z));
    playerRigidBodyCI.m_friction = 0.5;
    playerRigidBodyCI.m_restitution = 0.9;
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
WorldModel::HandleKinematicInputForPlayer(unsigned playerID)
{
    // Get the referenced player
    Player* player = GetPlayer(playerID);
    assert(player);
    btRigidBody* playerRigidBody = mPlayerRigidBodies[player];
    
    // Get the inputs
    Vector activeFalconInputs = player->GetActiveFalconInputs();
    uint32_t activeInputs = player->GetActiveInputs();
    if(activeFalconInputs.x != 0 || activeFalconInputs.y != 0 || activeFalconInputs.z != 0){
        //if we have falcon input, use that. the maximum force magnitude
        //we will have is 1.        
        //forceVector.setX(activeFalconInputs[FALCON_INPUT_FORWARD]);
        //forceVector.setZ(activeFalconInputs[FALCON_INPUT_RIGHT]);
    }else{
        //otherwise, use keyboard input and make sure the resulting force
        //always has the same length (or 0 length)
        //if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_JUMP, true))
            //forceVector += btVector3(0,1.5,0);
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_DASH, true))
            playerRigidBody->setLinearVelocity(1.2*(playerRigidBody->getLinearVelocity()));
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_BRAKE, true))
            playerRigidBody->setLinearVelocity(0.6*(playerRigidBody->getLinearVelocity()));
            //forceVector += btVector3(0,1.5,0);
        //float len = forceVector.length();
        //if(len != 0)
            //forceVector /= len;
    }
    
    //playerRigidBody->applyForce(forceVector*PLAYER_MAX_FORCE, btVector3(1.0, 1.0, 1.0)); IMPORTANT
}

void
WorldModel::HandleInputForPlayer(unsigned playerID)
{
    // Get the referenced player
    Player* player = GetPlayer(playerID);
    assert(player);
    btRigidBody* playerRigidBody = mPlayerRigidBodies[player];

    // Get the inputs
    btVector3 forceVector(0,0,0);
    int growthFactor = 0;
    Vector activeFalconInputs = player->GetActiveFalconInputs();
    uint32_t activeInputs = player->GetActiveInputs();
    if(activeFalconInputs.x != 0 || activeFalconInputs.y != 0 || activeFalconInputs.z != 0){
        //if we have falcon input, use that. the maximum force magnitude
        //we will have is 1.        
        forceVector.setX(activeFalconInputs[FALCON_INPUT_FORWARD]);
        forceVector.setZ(activeFalconInputs[FALCON_INPUT_RIGHT]);
    }else{
        //otherwise, use keyboard input and make sure the resulting force
        //always has the same length (or 0 length)
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_UP, true))
            forceVector += btVector3(0.5,0,0);
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_DOWN, true))
            forceVector += btVector3(-0.5,0,0);
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_LEFT, true))
            forceVector += btVector3(0,0,-0.5);
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_RIGHT, true))
            forceVector += btVector3(0,0,0.5);
        float len = forceVector.length();
        if(len != 0)
            forceVector /= len;
        if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_JUMP, true)) { // Jump
            if (playerRigidBody->getWorldTransform().getOrigin().getY() <= 6.0) {
                forceVector += btVector3(0,20,0);
            }
        }
    }
    if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_GROW, true))
        growthFactor += 1;
    if (activeInputs & GEN_INPUT_MASK(USERINPUT_INDEX_SHRINK, true))
        growthFactor += -1;
    playerRigidBody->applyForce(forceVector*PLAYER_MAX_FORCE, btVector3(0, 1.0, 0));
    btCollisionShape *collShape = mPlayerShapes[player];
    if(growthFactor != 0){
        float newScale = collShape->getLocalScaling().x() + growthFactor*PLAYER_SCALING_RATE;
        if(newScale <= PLAYER_MAXIMUM_SCALE && newScale >= PLAYER_MINIMUM_SCALE){
            float newMass = PLAYER_MASS_DENSITY * newScale*newScale*newScale;
            btVector3 newInertia(0, 0, 0);
            collShape->calculateLocalInertia(newMass,newInertia);
            playerRigidBody->setMassProps(newMass, newInertia);
            collShape->setLocalScaling(btVector3(newScale,newScale,newScale));
        }
    }
}

void WorldModel::ApplyHapticGravityForce(){
    Player *player = GetPlayer(mPlayerID);
    assert(player);
    btRigidBody *playerRigidBody = mPlayerRigidBodies[player];
    assert(playerRigidBody);
    bool isFalling = false;
    if(playerRigidBody->getLinearVelocity().y() < -.2 || playerRigidBody->getLinearVelocity().y() > .2){
        isFalling = true;
    }
    mFalcon->setVerticalForce(isFalling);
}

void WorldModel::ApplyHapticCollisionForce(){
    Player *player = GetPlayer(mPlayerID);
    assert(player);
    btVector3 impulse(0,0,0);
    for(unsigned i = 0; i < mPlayers.size(); ++i){
        Player *otherPlayer = mPlayers[i];
        if(otherPlayer == NULL || otherPlayer == player)
            continue;

        btRigidBody *playerRigidBody = mPlayerRigidBodies[player];
        assert(playerRigidBody);
        btCollisionShape *playerShape = mPlayerShapes[player];
        assert(playerShape);
        btVector3 playerCenter = playerRigidBody->getWorldTransform().getOrigin();
        float playerRadius = 1.1*playerShape->getLocalScaling().x();
        btRigidBody *otherRigidBody = mPlayerRigidBodies[otherPlayer];
        assert(otherRigidBody);
        btCollisionShape *otherShape = mPlayerShapes[otherPlayer];
        assert(otherShape);
        btVector3 otherCenter = otherRigidBody->getWorldTransform().getOrigin();
        float otherRadius = 1.1*otherShape->getLocalScaling().y();
        if((otherCenter-playerCenter).length() <= otherRadius+playerRadius){
            //player is colliding with other. need to add impulse.
            btVector3 playerMomentum = (1/playerRigidBody->getInvMass())*playerRigidBody->getLinearVelocity();
            btVector3 otherPlayerMomentum = (1/otherRigidBody->getInvMass())*otherRigidBody->getLinearVelocity();
            float impulseMag = (playerMomentum-otherPlayerMomentum).length();
            btVector3 impulseDir = playerCenter-otherCenter;
            impulse += impulseMag*impulseDir;
        }
    }
    mFalcon->setHorizontalForce(impulse.z(), impulse.x());
}


