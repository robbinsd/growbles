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
class Communicator;
class Timeline;

typedef enum {
    PAYLOAD_TYPE_NONE = 0,
    PAYLOAD_TYPE_WORLDSTATE,
    PAYLOAD_TYPE_USERINPUT
} PayloadType;

struct Payload {

    Payload() : type(PAYLOAD_TYPE_NONE), data(NULL), ownData(false) {};
    Payload(PayloadType t, void* d) : type(t), data(d), ownData(false) {};

    ~Payload();

    // Gets the data size for a given type
    unsigned GetDataSize();

    // The type of the payload
    PayloadType type;

    // Pointer to the payload data
    void* data;

    // Is the data owned by us? Default no.
    bool ownData;

    // Disallow copy constructor and operator=, as they would mess up
    // our memory ownership model.
    private:
    Payload(const Payload& other);
    Payload& operator=(const Payload& rhs);


};

class GrowblesSocket : public TcpSocket {

    public:

    // Constructor
    GrowblesSocket(ISocketHandler& h);

    // When we accept a client connection as server
    virtual void OnAccept();

    // Gets/Sets the ID of the remote player this socket connects
    // us to.
    unsigned GetRemoteID();
    void SetRemoteID(unsigned ID);

    // Sends a payload
    void SendPayload(Payload& payload);

    // Do we have a payload ready for reading?
    bool HasPayload();

    // Gets a payload. Caller must deallocate.
    // HasPayload() must return true first;
    void GetPayload(Payload& payload);

    protected:

    // The ID of the remote player this socket connects us to.
    unsigned mRemoteID;

    // Incoming payload
    Payload mIncoming;
};

class GrowblesHandler : public SocketHandler {

    public:

    // Constructor
    GrowblesHandler(Communicator& c);

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

    // Do any of the sockets have a payload?
    bool HasPayload();

    // Gets any available payload, returning the playerID of the source
    // HasPayload() must return true.
    unsigned ReceivePayload(Payload& payload);

    // Gets our Communicator
    Communicator* GetCommunicator() { return mCommunicator; };

    // For some dumb reason SocketHandler has a staging area for sockets (m_add),
    // and includes the number of staged sockets in GetCount(). We want to know
    // how many sockets are actually active.
    unsigned GetNumActiveSockets() { return m_sockets.size(); };

    protected:

    // The Communicator possessing this handler
    Communicator* mCommunicator;
};

typedef enum {
    COMMUNICATOR_MODE_NONE = 0,
    COMMUNICATOR_MODE_CLIENT,
    COMMUNICATOR_MODE_SERVER,
} CommunicatorMode;

class Communicator {

    friend class GrowblesSocket;

    public:

    /*
     * Constructor.
     */
    Communicator(Timeline& timeline, CommunicatorMode mode);

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
     * For clients: Send any new input to the server, apply world updates.
     * For server: Handle input updates, send world updates.
     */
    void Synchronize(WorldModel& model);

    /*
     * Bootstraps the client and server and gets everyone on the same page.
     */
    void Bootstrap(WorldModel& world);

    /*
     * Gets our player ID.
     */
    unsigned GetPlayerID() { return mPlayerID; } ;

    /*
     * Applies input. This adds the input to our timeline, and forwards
     * it to all connected sockets as well.
     */
    void ApplyInput(WorldModel& model, UserInput& input);

    protected:

    /*
     * Connection routines for client and server.
     */
    void ConnectAsClient();
    void ConnectAsServer();

    // Timeline
    Timeline* mTimeline;

    // Client or server?
    CommunicatorMode mMode;

    // Our player ID
    unsigned mPlayerID;

    // The next player ID to assign
    unsigned mNextPlayerID;

    // Valid for servers
    std::string mServerAddress;

    // Valid for clients
    unsigned mNumClientsExpected;

    // Our socket handler
    GrowblesHandler mSocketHandler;
};

#endif /* COMMUNICATOR_H */
