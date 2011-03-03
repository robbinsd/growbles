#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

class WorldModel;

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

    /*
     * Sets the server IP address. Only valid for client mode.
     */
    void SetServer(const char* server);

    /*
     * Sets the number of clients we're expecting. Only valid
     * for server mode.
     */
    void SetNumClients(unsigned n);

    /*
     * Connects to the other communicator(s).
     *
     * For client mode, this establishes a connection to the server.
     *
     * For server mode, this waits until the appropriate number of clients
     * have connected.
     */
    void Connect();

    /*
     * For clients: Send any new input to the server, apply world updates.
     * For server: Handle input updates, send world updates.
     */
    void Synchronize(WorldModel& model);

    protected:

    // Client or server?
    CommunicatorMode mMode;
};

#endif /* COMMUNICATOR_H */
