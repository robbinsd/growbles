#include "Timeline.h"

using std::list;
using std::vector;

/*
 * Timeline methods.
 */

Timeline::Timeline() : mWorld(NULL)
{
}

Timeline::~Timeline()
{
    if (mKeyframes.size() == 0)
        return;

    Prune(mKeyframes.back()->timestamp + 1);
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
Timeline::SendUpdates(Communicator& communicator)
{
    assert(mMode == COMMUNICATOR_MODE_SERVER);

    // If we don't have any keyframes, we have nothing to do
    if (mKeyframes.size() == 0)
        return;

    // Do we have a keyframe old enough to send as a state update?
    if (mWorld->GetCurrentTimestamp() < MIN_STATEUPDATE_AGE)
        return;
    KeyframeIterator candidate = FindKeyframe(mWorld->GetCurrentTimestamp() -
                                              MIN_STATEUPDATE_AGE);
    if (candidate == mKeyframes.end())
        return;

    // Is the candidate different enough from the last statedump sent to be worth
    // sending?
    if (mKeyframes.front()->timestamp + MIN_STATEDUMP_SEPARATION >
        (*candidate)->timestamp)
        return;

    // Send
    communicator.SendAuthoritativeState((*candidate)->state);

    // Prune
    Prune((*candidate)->timestamp);
}

void
Timeline::AddInput(UserInput& input)
{
    // We may be fast-forwarding and rewinding, so make sure our timeline contains
    // the newest model state.
    if (!UpToDate())
        GenerateCurrentKeyframe();

    // If the input is before our first keyframe, we can't do anything about it.
    if (input.timestamp < mKeyframes.front()->timestamp) {
        printf("Warning - Received input for player %u with timestamp %u, but "
               "we only have keyframes dating back to %u. Dropping.\n",
               input.playerID, input.timestamp, mKeyframes.front()->timestamp);
        return;
    }

    // If the input is ahead of our current worldstate...
    if (input.timestamp > mKeyframes.back()->timestamp) {

        // TODO - we should probably handle this better. Servers should discard
        // input, and clients should sync their game clocks.
        printf("Warning - Received input for player %u with timestamp %u, but "
               "we only have keyframes dating up to %u. Dropping.\n",
               input.playerID, input.timestamp, mKeyframes.back()->timestamp);
        return;
    }

    // Call the helper method
    AddInputInternal(input);
}

void
Timeline::AddAuthoritativeState(WorldState& state)
{
    // We should be a client
    assert(mMode == COMMUNICATOR_MODE_CLIENT);

    // Prune everything before the given state
    Prune(state.timestamp);

    // If we don't have a keyframe for this timestamp, make one
    if (!mKeyframes.size() ||
        (*mKeyframes.begin())->timestamp != state.timestamp) {
        Keyframe* frame = new Keyframe(state.timestamp, state);
        mKeyframes.push_front(frame);
    }
    // Otherwise, just update the statedump on the first keyframes
    else
        (*mKeyframes.begin())->state = state;

    // Rectify, starting at the front
    Rectify(mKeyframes.begin());
}

void
Timeline::AddInputInternal(UserInput& input)
{
    // Find the newest keyframe with a timestamp less than or equal to this one
    KeyframeIterator nearest = FindKeyframe(input.timestamp);
    assert(nearest != mKeyframes.end());

    // If there isn't a keyframe already there, we have to make one.
    // Note that we're about to Rectify(), so the state snapshot can be garbage.
    if ((*nearest)->timestamp < input.timestamp) {
        Keyframe* newFrame = new Keyframe(input.timestamp);
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
            assert((*upcoming)->timestamp > (*curr)->timestamp);
            unsigned stepSize = (*upcoming)->timestamp - (*curr)->timestamp;
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
    Keyframe* frame = new Keyframe(state.timestamp, state);
    mKeyframes.push_back(frame);
}

bool
Timeline::UpToDate()
{
    assert (mWorld->GetCurrentTimestamp() >= mKeyframes.back()->timestamp);
    return (mWorld->GetCurrentTimestamp() == mKeyframes.back()->timestamp);
}

void
Timeline::Prune(unsigned timestamp)
{
    KeyframeIterator it = mKeyframes.begin();
    while (it != mKeyframes.end()) {
        if ((*it)->timestamp >= timestamp)
            return;
        mKeyframes.erase(it++);
    }
}

KeyframeIterator
Timeline::FindKeyframe(unsigned timestamp)
{
    KeyframeIterator lastGood = mKeyframes.end();
    for (KeyframeIterator it = mKeyframes.begin();
         it != mKeyframes.end(); ++it) {

        if ((*it)->timestamp <= timestamp)
            lastGood = it;
        else
            break;
    }

    return lastGood;
}
