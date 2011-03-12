#ifndef GAMECLOCK_H
#define GAMECLOCK_H

#include "Framework.h"

#define GAMECLOCK_TICK_MS 32

class Gameclock {

    public:
    /*
     * Constructor.
     *
     * We start at t=0.
     */
    Gameclock(unsigned tickMS);

    /*
     * Starts the clock.
     */
    void Start();

    /*
     * Sets the clock to a specified timestamp. Use with caution.
     */
    void Set(unsigned timestamp);

    /*
     * Ticks the clock forward as closed to 1 tick as we can.
     *
     * If the machine is very fast, Tick() will busywait until
     * at least one tick has occurred. if the machine is slow,
     * Tick() may jump the value of Now() by more than one.
     */
    void Tick();

    /*
     * Gets the current timestamp.
     */
    unsigned Now() const { return mTimestamp; };

    /*
     * Gets the timestamp before the most recent Tick() call.
     */
    unsigned Then() const;

    /*
     * Gets the exact number of seconds elapsed between the
     * current tick and the previous tick. This includes 
     * leftovers.
     */
    float GetDeltaTime() const;


    protected:

    // Timestamp
    unsigned mTimestamp;

    // Size of the last step we took
    unsigned mLastStep;

    // Number of seconds per tick
    float mTickDuration;

    // Our internal clock
    sf::Clock mClock;

    // The remainder on the clock after the last tick
    float mClockRemainder;
};

#endif /* GAMECLOCK_H */
