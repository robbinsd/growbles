#include "Timeline.h"

using std::list;
using std::vector;

/*
 * Keyframe methods.
 */

Keyframe::Keyframe(WorldState& s)
{
    state = s;
}

void
Keyframe::InsertInput(UserInput& input)
{

}

/*
 * Timeline methods.
 */

Timeline::Timeline() : mWorld(NULL)
{
}

void
Timeline::Init(WorldModel& model)
{
    // Store a reference to the worldmodel
    mWorld = &model;

    // Grab the initial world state
    WorldState initialState;
    mWorld->GetState(initialState);

    // Make our first keyframe
    assert(initialState.timestamp == 0);
    Keyframe* frame = new Keyframe(initialState);
    mKeyframes.insert(mKeyframes.begin(), frame);
}
