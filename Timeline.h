#ifndef TIMELINE_H
#define TIMELINE_H

#include "WorldModel.h"
#include "UserInput.h"
#include "Communicator.h"
#include <list>
#include <vector>

// The minimum number of ticks between authoritative updates
#define STATE_UPDATE_THRESHOLD 15

// The youngest state dump we're allowed to send as authoritative
#define MIN_STATEUPDATE_AGE 10

// Minimum seperation between statedumps
#define MIN_STATEDUMP_SEPARATION 5

/*
 * A keyframe is an item in our timeline. It contains a snapshot of the
 * world state at the beginning of that timestep, and the input applied
 * during that timestep.
 */
struct Keyframe {

    /*
     * Constructors.
     */
    Keyframe(unsigned t) : timestamp(t) {};
    Keyframe(unsigned t, WorldState& s) : timestamp(t), state(s) {};

    // Timestamp
    unsigned timestamp;

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
     * Destructor.
     */
    ~Timeline();

    /*
     * Initialize the timeline. Must be called at t=0.
     */
    void Init(WorldModel& model, CommunicatorMode mode);

    /*
     * Currently, this just sends authoritative state for the server
     * on occasion.
     */
    void SendUpdates(Communicator& communicator);

    /*
     * Adds an input to the timeline.
     */
    void AddInput(UserInput& input);

    /*
     * Adds authoritative server state to the timeline.
     */
    void AddAuthoritativeState(WorldState& state);

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
     * Does the newest timestamp in the timeline match the timestamp of the
     * world model?
     */
    bool UpToDate();

    /*
     * Prunes keyframes up to (but not including) the given timestamp.
     */
    void Prune(unsigned timestamp);

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
