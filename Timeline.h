#ifndef TIMELINE_H
#define TIMELINE_H

#include "WorldModel.h"
#include "UserInput.h"
#include "Communicator.h"
#include <list>
#include <vector>

// The number of steps between keyframes
#define KEYFRAME_STEP 30

/*
 * A keyframe is an item in our timeline. It contains a snapshot of the
 * world state at the beginning of that timestep, and the input applied
 * during that timestep.
 */
struct Keyframe {

    /*
     * Constructors.
     */
    Keyframe() {};
    Keyframe(WorldState& s) : state(s) {};

    // state snapshot
    WorldState state;

    // Inputs applied
    std::vector<UserInput> inputs;
};

typedef std::list<Keyframe*>::iterator KeyframeIterator;

class Timeline {

    public:

    /*
     * Dummy Constructor.
     */
    Timeline();

    /*
     * Initialize the timeline. Must be called at t=0.
     */
    void Init(WorldModel& model, CommunicatorMode mode);

    /*
     * Adds an input to the timeline.
     */
    void AddInput(UserInput& input);

    protected:

    /*
     * Internal method to add input. The input must be in the range
     * of our keyframes.
     */
    void AddInputInternal(UserInput& input);

    /*
     * Rebuilds the state snapshots in the keyframes, starting with a
     * position with a known good snapshot.
     *
     * This mucks with WorldModel state. At the end, it leaves WorldModel
     * with the rebuilt state at the last keyframe.
     */
    void Rectify(KeyframeIterator lastGood);

    /*
     * Generates a keyframe for the current worldstate.
     */
    void GenerateCurrentKeyframe();

    /*
     * Finds the keyframe with the highest timestamp less than
     * or equal to timestamp.
     *
     * Returns NULL if timestamp is later than the newest keyframe.
     */
    KeyframeIterator FindKeyframe(unsigned timestamp);

    // Pointer to our worldmodel
    WorldModel* mWorld;

    // Client or server?
    CommunicatorMode mMode;

    // Our set of keyframes, from newest to oldest
    std::list<Keyframe*> mKeyframes;

};

#endif /* TIMELINE_H */
