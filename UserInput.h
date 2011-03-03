#ifndef USERINPUT_H
#define USERINPUT_H

#include "Framework.h"

class RenderContext;
class WorldModel;

struct UserInput {

    /*
     * Dumb constructor.
     */
    UserInput() : mInputs(0) {};

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
    uint32_t mInputs;
};

#endif /* USERINPUT_H */
