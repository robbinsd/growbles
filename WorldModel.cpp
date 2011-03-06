#include "WorldModel.h"
#include "UserInput.h"

void
WorldModel::Init(SceneGraph& sceneGraph)
{
    // Save parameters
    mSceneGraph = &sceneGraph;
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

void
WorldModel::SetPlayer(Player* p)
{
    player = p;
}

void
WorldModel::MovePlayer(unsigned playerID, int direction)
{
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

