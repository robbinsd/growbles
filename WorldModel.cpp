#include "WorldModel.h"

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
