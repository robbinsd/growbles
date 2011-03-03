#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

typedef enum {
    COMMUNICATOR_MODE_CLIENT = 0,
    COMMUNICATOR_MODE_SERVER,
    COMMUNICATOR_MODE_NONE
} CommunicatorMode;

class Communicator {

    public:

    /*
     * Constructor.
     */
    Communicator(CommunicatorMode mode);


    protected:

    // Client or server?
    CommunicatorMode mMode;
};

#endif /* COMMUNICATOR_H */
