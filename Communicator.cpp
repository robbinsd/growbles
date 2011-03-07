#include "Communicator.h"
#include "WorldModel.h"
#include "assert.h"

#include <Sockets/Lock.h>
#include <Sockets/ListenSocket.h>
#include <Sockets/TcpSocket.h>

const unsigned sGrowblesMagic = 0x640E8135;

// 0 should never be a player ID
static unsigned sNextPlayerID = 1;

/*
 * Payload Methods.
 */

size_t
Payload::GetDataSize()
{
    switch(type) {
        case PAYLOAD_TYPE_WORLDSTATE:
            return sizeof(WorldState);
        case PAYLOAD_TYPE_USERINPUT:
            return sizeof(UserInput);
        default:
            assert(0); // Not reached
            return 0;
    }
}

/*
 * GrowblesSocket Methods.
 */

class GrowblesSocket : public TcpSocket {

    public:
    GrowblesSocket(ISocketHandler& h) : TcpSocket(h)
                                      , mClientID(0)
    {
        // We don't want TCP to buffer things up
        SetTcpNodelay();
    }

    void OnAccept() {

        // We start by sending clients the magic word
        unsigned message[2];
        message[0] = sGrowblesMagic;

        // Then we send them their player ID
        mClientID = sNextPlayerID++;
        message[1] = mClientID;

        // Send
        SendBuf((const char *)&message, sizeof(message));
    }

    /*
     * Gets the ID of the client this socket communicates with.
     *
     * Not valid to call for client-side sockets.
     */
    unsigned GetClientID() {
        assert(mClientID != 0);
        return mClientID;
    }

    /*
     * Sends a payload.
     */
    void SendPayload(Payload& payload)
    {
        // We're using TCP_NODELAY, which sends data immediately. However, we want
        // our payload to go in a single packet. So we create a buffer here for
        // the packet.
        unsigned dataSize = payload.GetDataSize();
        unsigned buffSize = sizeof(payload.type) + sizeof(size_t) + dataSize;
        char* buffer = (char*)malloc(buffSize);
        assert(buffer);

        // Fill the buffer
        char* currBuffer = buffer;
        memcpy(currBuffer, &payload.type, sizeof(payload.type));
        currBuffer += sizeof(payload.type);
        memcpy(currBuffer, &dataSize, sizeof(dataSize));
        currBuffer += sizeof(dataSize);
        memcpy(currBuffer, payload.data, dataSize);

        // Send the buffer
        SendBuf(currBuffer, buffSize);
    }

    protected:

    // The ID of the client this socket represents. Note that this is only
    // set, and thus should only be queried, on server-side sockets.
    unsigned mClientID;

};

/*
 * GrowblesHandler Methods.
 */

void
GrowblesHandler::AddPlayers(WorldModel& model)
{
    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it)
        model.AddPlayer(dynamic_cast<GrowblesSocket*>(it->second)->GetClientID());
}

void
GrowblesHandler::SendToAll(Payload& payload)
{
    // Zero can never be a player ID
    SendToAllExcept(payload, 0);
}

void
GrowblesHandler::SendToAllExcept(Payload& payload, unsigned excluded)
{
    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it) {
        GrowblesSocket* socket = dynamic_cast<GrowblesSocket*>(it->second);
        if (socket->GetClientID() != excluded)
            socket->SendPayload(payload);
    }
}

void
GrowblesHandler::SendTo(Payload& payload, unsigned playerID)
{
    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it) {
        GrowblesSocket* socket = dynamic_cast<GrowblesSocket*>(it->second);
        if (socket->GetClientID() == playerID) {
            socket->SendPayload(payload);
            return;
        }
    }
}

/*
 * Communicator Methods.
 */

Communicator::Communicator(CommunicatorMode mode) : mMode(mode)
                                                  , mPlayerID(0)
                                                  , mNumClientsExpected(0)
{
    // If we're a server, assign ourselves a player ID
    if (mode == COMMUNICATOR_MODE_SERVER)
        mPlayerID = sNextPlayerID++;
}

void
Communicator::SetServer(const char* server)
{
    assert(mMode == COMMUNICATOR_MODE_CLIENT);
    mServerAddress = server;
}

void
Communicator::SetNumClientsExpected(unsigned n)
{
    assert(mMode == COMMUNICATOR_MODE_SERVER);
    mNumClientsExpected = n;
}

void
Communicator::Connect()
{
    if (mMode == COMMUNICATOR_MODE_CLIENT)
        ConnectAsClient();
    else {
        assert(mMode == COMMUNICATOR_MODE_SERVER);
        ConnectAsServer();
    }
}

void
Communicator::ConnectAsClient()
{
    GrowblesSocket* socket = new GrowblesSocket(mSocketHandler);
    socket->SetDeleteByHandler();
    socket->Open(mServerAddress, GROWBLES_PORT);
    mSocketHandler.Add(socket);

    // Read the first transmission from the server
    unsigned openingMessage[2];
    while (socket->GetInputLength() < sizeof(openingMessage))
        mSocketHandler.Select();
    socket->ReadInput((char *)&openingMessage, sizeof(openingMessage));

    // verify the magic word
    if (openingMessage[0] != sGrowblesMagic) {
        printf("Received bad magic word (%x) from server!\n", openingMessage[0]);
        exit(-1);
    }

    // Save our player ID
    mPlayerID = openingMessage[1];
    printf("Assigned player ID %u\n", mPlayerID);
}

void
Communicator::ConnectAsServer()
{
    // Create the ListenSocket. Once bound, this adds a new
    // TcpSocket to the handler for each accepted connection.
    ListenSocket<GrowblesSocket> listenSocket(mSocketHandler);
    if (listenSocket.Bind(GROWBLES_PORT)) {
        printf("Couldn't bind to port %u!\n", GROWBLES_PORT);
        exit(-1);
    }

    // Add the ListenSocket to the handler. When the destructor is called
    // at the end of this method, the ListenSocket will remove itself.
    mSocketHandler.Add(&listenSocket);

    // We want to wait until we've accepted the desired number of connections.
    // Note that we want GetCount() to be one more than our desired number of
    // clients, because the handler is holding onto the ListenSocket too.
    while (mSocketHandler.GetCount() < mNumClientsExpected + 1)
        mSocketHandler.Select(1,0);
}

void
Communicator::Synchronize(WorldModel& model)
{
}

void
Communicator::InitWorld(WorldModel& world, SceneGraph& sceneGraph)
{
    // If we're a client, we need to receive the initial scene data from the server.
    // We don't support that yet, so assert that we're a server.
    assert(mMode == COMMUNICATOR_MODE_SERVER);

    // Initialize the world
    world.Init(sceneGraph);

    // Add the server player
    world.AddPlayer(mPlayerID);

    // Add the client players
    mSocketHandler.AddPlayers(world);
}

void
Communicator::SendInput(UserInput& input)
{
}
