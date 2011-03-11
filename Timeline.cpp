#include "Timeline.h"

using std::list;
using std::vector;

/*
 * Keyframe methods.
 */

void
Keyframe::InsertInput(UserInput& input)
{

}

/*
 * Timeline methods.
 */

Timeline::Timeline() : mWorld(NULL)
{
}

void
Timeline::Init(WorldModel& model)
{
    // Store a reference to the worldmodel
    mWorld = &model;
}
