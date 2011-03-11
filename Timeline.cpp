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
Timeline::Init(WorldModel& model, CommunicatorMode mode)
{
    // Store parameters
    mWorld = &model;
    mMode = mode;

    // Grab the initial world state
    WorldState initialState;
    mWorld->GetState(initialState);

    // Make our first keyframe
    assert(initialState.timestamp == 0);
    Keyframe* frame = new Keyframe(initialState);
    mKeyframes.insert(mKeyframes.begin(), frame);
}
