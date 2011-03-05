#ifndef USERINPUT_H
#define USERINPUT_H

#include "Framework.h"
#include <stdint.h>

class RenderContext;
class WorldModel;

#define USERINPUT_MASK_GROW (1 << 0)
#define USERINPUT_MASK_SHRINK (1 << 1)

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

    /*
     * Applies recorded input to a world model.
     */
    void ApplyInput(WorldModel& model);

    // Bitfield of inputs
    uint32_t inputs;

    // Worldspace timestamp when the input was applied
    uint32_t timestamp;

    // ID of the player doing the input
    uint32_t playerID;
};

#endif /* USERINPUT_H */
