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
     * Constructor.
     */
    Keyframe(WorldState& s) : state(s) {};

    // state snapshot
    WorldState state;

    // Inputs applied
    std::vector<UserInput> inputs;
};

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
     * Generates a keyframe for the current worldstate.
     */
    void GenerateCurrentKeyframe();

    // Pointer to our worldmodel
    WorldModel* mWorld;

    // Client or server?
    CommunicatorMode mMode;

    // Our set of keyframes, from newest to oldest
    std::list<Keyframe*> mKeyframes;

};

#endif /* TIMELINE_H */
