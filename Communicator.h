#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "UserInput.h"
#include <Sockets/SocketHandler.h>
#include <Sockets/TcpSocket.h>

#define GROWBLES_PORT 9323

class WorldModel;
class UserInput;
class GrowblesSocket;
struct SceneGraph;

typedef enum {
    PAYLOAD_TYPE_NONE = 0,
    PAYLOAD_TYPE_WORLDSTATE,
    PAYLOAD_TYPE_USERINPUT
} PayloadType;

struct Payload {

    Payload() : type(PAYLOAD_TYPE_NONE), data(NULL) {};
    Payload(PayloadType t, void* d) : type(t), data(d) {};

    // Gets the data size for a given type
    size_t GetDataSize();

    // The type of the payload
    PayloadType type;

    // Pointer to the payload data
    void* data;

};

class GrowblesSocket : public TcpSocket {

    public:

    // Constructor
    GrowblesSocket(ISocketHandler& h);

    // When we accept a client connection as server
    void OnAccept();

    // Gets the ID of the client this socket communicates with.
    //
    // Not valid to call for client-side sockets.
    unsigned GetClientID();

    // Sends a payload
    void SendPayload(Payload& payload);

    // Do we have a payload ready for reading?
    bool HasPayload();

    // Gets a payload. Caller must deallocate.
    // HasPayload() must return true first;
    void GetPayload(Payload& payload);

    protected:

    // The ID of the client this socket represents. Note that this is only
    // set, and thus should only be queried, on server-side sockets.
    unsigned mClientID;

    // Incoming payload
    Payload mIncoming;
};

class GrowblesHandler : public SocketHandler {

    public:

    // Adds each connection as a player in the world.
    //
    // Should only be called on the server.
    void AddPlayers(WorldModel& model);

    // Sends a payload to all connected sockets
    void SendToAll(Payload& payload);

    // Sends a payload to all connected sockets except
    // the one given by excluded.
    void SendToAllExcept(Payload& payload, unsigned excluded);

    // Sends a payload to a specific player
    void SendTo(Payload& payload, unsigned playerID);
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
