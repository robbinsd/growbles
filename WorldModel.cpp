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

    // environment map
    Vector emapPos(0.0, 3.0 + ARMADILLO_BASE_Y, 0.0, 1.0);
    sceneGraph.FindMesh("Armadillo_0")->EnvironmentMap(emapPos);
}

WorldModel::~WorldModel()
{
    // Delete our player objects
    for (vector<Player*>::iterator it = mPlayers.begin();
         it != mPlayers.end(); ++it)
        delete *it;
}

void
WorldModel::Step()
{
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

void WorldModel::AddPlayer(unsigned playerID, Vector initialPosition)
{
    // Make sure we don't already have a player by this ID
    assert(GetPlayer(playerID) == NULL);

    // Add the player to the scenegraph
    Matrix playerTransform;
    playerTransform.Translate(initialPosition.x, initialPosition.y, initialPosition.z);
    stringstream numSS;
    numSS << playerID;
    string nodeName = string("PlayerNode_") + numSS.str();
    string rootName = string("PlayerRoot_") + numSS.str();
    SceneNode* playerNode = mSceneGraph->AddNode(&mSceneGraph->rootNode,
                                                 playerTransform,
                                                 nodeName.c_str());
    mSceneGraph->LoadScene(SPHERE_PATH, rootName.c_str(), playerNode);

    // Initialize the model representation of the player
    Player* player = new Player(playerID, playerNode);

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
            player->Move(0.1, 0.0, 0.0);
            break;
        case USERINPUT_MASK_DOWN:
            player->Move(-0.1, 0.0, 0.0);
            break;
        case USERINPUT_MASK_LEFT:
            player->Move(0.0, 0.0, -0.1);
            break;
        case USERINPUT_MASK_RIGHT:
            player->Move(0.0, 0.0, 0.1);
            break;
        default:
            break;
    }
}

