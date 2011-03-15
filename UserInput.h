#ifndef USERINPUT_H
#define USERINPUT_H

#include "Framework.h"
#include "Vector.h"
#include <stdint.h>

class RenderContext;
class WorldModel;
class Communicator;

typedef enum {
    USERINPUT_INDEX_GROW = 0,
    USERINPUT_INDEX_SHRINK,
    USERINPUT_INDEX_UP,
    USERINPUT_INDEX_DOWN,
    USERINPUT_INDEX_LEFT,
    USERINPUT_INDEX_RIGHT,
    USERINPUT_INDEX_COUNT,
    USERINPUT_INDEX_DASH,
    USERINPUT_INDEX_JUMP,
    USERINPUT_INDEX_BRAKE
} UserInputIndex;

// The actual bitmask uses 2 bits for each item (one to signal the start
// of application and one to signal the end). Begin events always have an
// even bit index.

#define GEN_INPUT_MASK(index, isBegin) (isBegin ? 1 << (2*index) : 1 << (2*index+1))

struct UserInput {

    /*
     * Dumb constructor.
     */
    UserInput(unsigned playerID, unsigned timestamp);

    /*
     * Loads input from the user.
     *
     * Local input (that is to say, input that affects only the local
     * render client and doesn't get communicated over the network) is
     * applied immediately. All other inputs are stored as instance data
     * for later application.
     */
    void LoadInput(RenderContext& context, Communicator& communicator,
	    WorldModel &world);

#ifdef FALCON
    /*
     * Helper function. Converts the falcon's position into key presses/releases.
     * Stores these in the input bit array.
     */
    void SetFalconInputs(Vector positionOffset, Vector velocity);

#endif

    // Bitfield of inputs
    uint32_t inputs;

    // 3 floats indicating how much falcon wants to go left/right, up/down, or
    // grow/shrink
    Vector falconInputs;
    // Worldspace timestamp when the input was applied
    uint32_t timestamp;

    // ID of the player doing the input
    uint32_t playerID;
};

#endif /* USERINPUT_H */
