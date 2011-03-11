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
    // We may be fast-forwarding and rewinding, so make sure our timeline contains
    // the newest model state.
    if (!UpToDate())
        GenerateCurrentKeyframe();

    // If the input is before our first keyframe, we can't do anything about it.
    if (input.timestamp < mKeyframes.front()->state.timestamp) {
        printf("Warning - Received input for player %u with timestamp %u, but "
               "we only have keyframes dating back to %u. Dropping.\n",
               input.playerID, input.timestamp, mKeyframes.front()->state.timestamp);
        return;
    }

    // If the input is ahead of our current worldstate...
    if (input.timestamp > mKeyframes.back()->state.timestamp) {

        // TODO - we should probably handle this better. Servers should discard
        // input, and clients should sync their game clocks.
        printf("Warning - Received input for player %u with timestamp %u, but "
               "we only have keyframes dating up to %u. Dropping.\n",
               input.playerID, input.timestamp, mKeyframes.back()->state.timestamp);
        return;
    }

    // Call the helper method
    AddInputInternal(input);
}

void
Timeline::AddInputInternal(UserInput& input)
{
    // Find the newest keyframe with a timestamp less than or equal to this one
    KeyframeIterator nearest = FindKeyframe(input.timestamp);
    assert(nearest != mKeyframes.end());

    // If there isn't a keyframe already there, we have to make one.
    // Note that we're about to Rectify(), so the state snapshot can be garbage.
    if ((*nearest)->state.timestamp < input.timestamp) {
        Keyframe* newFrame = new Keyframe();
        newFrame->inputs.push_back(input);
        KeyframeIterator pos = nearest;
        ++pos;
        mKeyframes.insert(pos, newFrame);
    }

    // If there is a keyframe, just add the input
    else
        (*nearest)->inputs.push_back(input);

    // Rectify
    Rectify(nearest);
}

void
Timeline::Rectify(KeyframeIterator lastGood)
{
    // Rewind ourselves to the state snapshot given
    mWorld->SetState((*lastGood)->state);
    KeyframeIterator curr, upcoming;
    curr = upcoming = lastGood;

    while (curr != mKeyframes.end()) {

        // We increment upcoming at the _beginning_ of the loop
        ++upcoming;

        // Dump the world model state into the timeline
        WorldState state;
        mWorld->GetState(state);
        (*curr)->state = state;

        // Apply all the inputs at this stage
        for (unsigned i = 0; i < (*curr)->inputs.size(); ++i)
            mWorld->ApplyInput((*curr)->inputs[i]);

        // Step the world, if necessary
        if (upcoming != mKeyframes.end()) {
            unsigned stepSize = (*upcoming)->state.timestamp -
                                  (*curr)->state.timestamp;
            mWorld->Step(stepSize);
        }

        // We increment curr at the _end_ of the loop
        ++curr;
    }
}

void
Timeline::GenerateCurrentKeyframe()
{
    assert(mKeyframes.size() == 0 || !UpToDate());

    // Grab the world state
    WorldState state;
    mWorld->GetState(state);

    // Append our keyframe
    Keyframe* frame = new Keyframe(state);
    mKeyframes.push_back(frame);
}

bool
Timeline::UpToDate()
{
    assert (mWorld->GetCurrentTimestamp() >= mKeyframes.back()->state.timestamp);
    return (mWorld->GetCurrentTimestamp() == mKeyframes.back()->state.timestamp);
}

KeyframeIterator
Timeline::FindKeyframe(unsigned timestamp)
{
    assert(mKeyframes.size() > 0);
    KeyframeIterator lastGood = mKeyframes.end();
    for (KeyframeIterator it = mKeyframes.begin();
         it != mKeyframes.end(); ++it) {

        if ((*it)->state.timestamp <= timestamp)
            lastGood = it;
        else
            break;
    }

    return lastGood;
}
