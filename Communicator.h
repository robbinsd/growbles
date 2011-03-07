#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "UserInput.h"
#include <Sockets/SocketHandler.h>

#define GROWBLES_PORT 9323

class WorldModel;
class UserInput;
class GrowblesSocket;
struct SceneGraph;

class GrowblesHandler : public SocketHandler {

    public:

    // Adds each connection as a player in the world.
    //
    // Should only be called on the server.
    void AddPlayers(WorldModel& model);
};

typedef enum {
    COMMUNICATOR_MODE_NONE = 0,
    COMMUNICATOR_MODE_CLIENT,
    COMMUNICATOR_MODE_SERVER,
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
    void SetNumClientsExpected(unsigned n);

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
     * Sends client input to the server.
     *
     * No-op if we're the server.
     */
    void SendInput(UserInput& input);

    /*
     * For clients: Send any new input to the server, apply world updates.
     * For server: Handle input updates, send world updates.
     */
    void Synchronize(WorldModel& model);

    /*
     * Initializes a world.
     */
    void InitWorld(WorldModel& world, SceneGraph& sceneGraph);

    /*
     * Gets our player ID.
     */
    unsigned GetPlayerID() { return mPlayerID; } ;

    protected:

    /*
     * Connection routines for client and server.
     */
    void ConnectAsClient();
    void ConnectAsServer();

    // Client or server?
    CommunicatorMode mMode;

    // Our player ID
    unsigned mPlayerID;

    // Valid for servers
    std::string mServerAddress;

    // Valid for clients
    unsigned mNumClientsExpected;

    // Our socket handler
    GrowblesHandler mSocketHandler;
};

#endif /* COMMUNICATOR_H */
