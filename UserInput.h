#ifndef USERINPUT_H
#define USERINPUT_H

#include "Framework.h"
#include <stdint.h>

class RenderContext;
class WorldModel;

#define USERINPUT_MASK_GROW_BEGIN (1 << 0)
#define USERINPUT_MASK_GROW_END (1 << 1)
#define USERINPUT_MASK_SHRINK_BEGIN (1 << 2)
#define USERINPUT_MASK_SHRINK_END (1 << 3)
#define USERINPUT_MASK_UP_BEGIN (1 << 4)
#define USERINPUT_MASK_UP_END (1 << 5)
#define USERINPUT_MASK_DOWN_BEGIN (1 << 6)
#define USERINPUT_MASK_DOWN_END (1 << 7)
#define USERINPUT_MASK_LEFT_BEGIN (1 << 8)
#define USERINPUT_MASK_LEFT_END (1 << 9)
#define USERINPUT_MASK_RIGHT_BEGIN (1 << 10)
#define USERINPUT_MASK_RIGHT_END (1 << 11)

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
    void LoadInput(RenderContext& context);

    // Bitfield of inputs
    uint32_t inputs;

    // Worldspace timestamp when the input was applied
    uint32_t timestamp;

    // ID of the player doing the input
    uint32_t playerID;
};

#endif /* USERINPUT_H */
