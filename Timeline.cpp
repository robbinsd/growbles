#include "Timeline.h"

using std::list;
using std::vector;

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

    // Generate an initial keyframe
    GenerateCurrentKeyframe();
}

void
Timeline::AddInput(UserInput& input)
{
    // If the input is before our first keyframe, we can't do anything about it.
    if (input.timestamp < mKeyframes.front()->state.timestamp) {
        printf("Warning - Received input for player %u with timestamp %u, but "
               "we only have keyframes dating back to %u. Dropping.\n",
               input.playerID, input.timestamp, mKeyframes.front()->state.timestamp);
        return;
    }

    // If the world has progressed far enough and it's just the timeline lagging
    // behind, generate an up-to-date timestamp.
    if (input.timestamp <= mWorld->GetCurrentTimestamp())
        GenerateCurrentKeyframe();

    // If the input is ahead of our current worldstate...
    if (input.timestamp > mKeyframes.back()->state.timestamp) {

        // TODO - we should probably handle this better. Servers should discard
        // input, and clients should sync their game clocks.
        printf("Warning - Received input for player %u with timestamp %u, but "
               "we only have keyframes dating up to %u. Dropping.\n",
               input.playerID, input.timestamp, mKeyframes.back()->state.timestamp);
        return;
    }

    // TODO - finish me
}

void
Timeline::GenerateCurrentKeyframe()
{
    // Grab the world state
    WorldState state;
    mWorld->GetState(state);

    // This should be ahead of any other keyframes we have
    assert(state.timestamp > mKeyframes.back()->state.timestamp);

    // Append our keyframe
    Keyframe* frame = new Keyframe(state);
    mKeyframes.push_back(frame);
}
