#include "Player.h"
#include "UserInput.h"

Player::Player(unsigned playerID,
               SceneNode* playerSceneNode,
               Vector initialPosition) : mPlayerID(playerID)
                                       , mPlayerNode(playerSceneNode)
                                       , activeInputs(0)
{
}

void
Player::setTransform(btTransform trans)
{
    Matrix transform;
    Vector origin(trans.getOrigin());
    transform.Translate(origin.x, origin.y, origin.z);
    btMatrix3x3 rotation(trans.getRotation());
    Matrix ourRotation(rotation);
    transform = transform.MMProduct(ourRotation);
    mPlayerNode->SetTransform(transform);
}

void
Player::applyInput(UserInput& input)
{
    // Separate our inputs into those that begin an action and those
    // that end an action.
    uint32_t begins = input.inputs & 0x55555555;
    uint32_t ends = input.inputs & 0xAAAAAAAA;

    // Sanity check - We shouldn't begin and end the same input
    // NOTE - apparently SFML does this sometimes. Oh well!
    //assert(((ends >> 1) & begins) == 0);

    // Sanity check - We shouldn't begin anything already begun
    // NOTE - this is disabled because operating systems actually send multiple
    // KeyPressed events after a certain delay when holding down a key.
    // assert((begins & activeInputs) == 0);

    // Sanity check - We shouldn't end anything not begun
    assert(((ends >> 1) | activeInputs) == activeInputs);

    // Apply our begins
    activeInputs |= begins;

    // Apply our ends
    activeInputs &= ~(ends >> 1);
}
