#ifndef USERINPUT_H
#define USERINPUT_H

#include "Framework.h"
#include <stdint.h>

class RenderContext;
class WorldModel;

#define USERINPUT_MASK_GROW (1 << 0)
#define USERINPUT_MASK_SHRINK (1 << 1)
#define USERINPUT_MASK_UP (1 << 2)
#define USERINPUT_MASK_DOWN (1 << 3)
#define USERINPUT_MASK_LEFT (1 << 4)
#define USERINPUT_MASK_RIGHT (1 << 5)

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
    
    // flag to denote if a key is down
    bool keyDown;
    
    // flag to denote if a key is released
    bool keyReleased;
    
    /*
     * Reset variables associated with this input object
     * this is necessary every time through the main game loop
     */
    void resetInputState();
};

#endif /* USERINPUT_H */
