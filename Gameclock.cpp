#include "Gameclock.h"
#include <assert.h>

Gameclock::Gameclock(unsigned tickMS) : mTimestamp(0)
                                      , mLastStep(0)
                                      , mTickDuration(tickMS / 1000.0)
                                      , mClockRemainder(0.0f)
{
}

void
Gameclock::Start()
{
    assert(mTimestamp == 0);
    mClock.Reset();
}

unsigned
Gameclock::Then() const
{
    assert(mLastStep != 0);
    assert(mTimestamp >= mLastStep);
    return mTimestamp - mLastStep;
}

float
Gameclock::GetDeltaTime() const
{
    return mLastStep*mTickDuration + mClockRemainder;
}

void
Gameclock::Tick()
{
    // Busywait until a tick has passed
    float elapsedTime;
    do {
        elapsedTime = mClock.GetElapsedTime() + mClockRemainder;
    } while (elapsedTime < mTickDuration);

    // Reset the clock
    mClock.Reset();

    // Determine how many ticks passed
    unsigned nTicks = (unsigned) (elapsedTime / mTickDuration);
    assert(nTicks > 0);

    // Increment the timestamp
    mTimestamp += nTicks;

    // Save the remainder
    mClockRemainder = elapsedTime - nTicks * mTickDuration;
    assert(mClockRemainder >= 0.0f);
    assert(mClockRemainder < mTickDuration);

    // Remember the step we took
    mLastStep = nTicks;
}
