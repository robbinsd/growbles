#ifndef TIMELINE_H
#define TIMELINE_H

#include "WorldModel.h"
#include "UserInput.h"
#include <list>
#include <vector>

// The number of steps between keyframes
#define KEYFRAME_STEP 30

/*
 * A keyframe is an item in our timeline. It contains a snapshot of the
 * world state at that timestamp, and any known subsequent inputs that
 * are timestamped before the next keyframe.
 */
struct Keyframe {

    /*
     * Constructor.
     */
    Keyframe(WorldState& s);

    /*
     * Inserts an input into the appropriate place in the list.
     */
    void InsertInput(UserInput& input);

    // state snapshot
    WorldState state;

    // An ordered list of inputs
    std::list<UserInput> inputs;
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
    void Init(WorldModel& model);

    protected:

    // Pointer to our worldmodel
    WorldModel* mWorld;

    // Our set of keyframes, from newest to oldest
    std::vector<Keyframe*> mKeyframes;

};

#endif /* TIMELINE_H */
