#include "Communicator.h"
#include "WorldModel.h"
#include "Timeline.h"
#include "assert.h"

#include <Sockets/Lock.h>
#include <Sockets/ListenSocket.h>

const unsigned sGrowblesMagic = 0x640E8135;

/*
 * Payload Methods.
 */

Payload::~Payload()
{
    // If we don't own the data, we have nothing to do
    if (!ownData)
        return;

    // If we're marked as owning the data, we ought to have it.
    assert(data);

    // Free the data
    free(data);
}

unsigned
Payload::GetDataSize()
{
    switch(type) {
        case PAYLOAD_TYPE_WORLDSTATE:
            return (unsigned) sizeof(WorldState);
        case PAYLOAD_TYPE_USERINPUT:
            return (unsigned) sizeof(UserInput);
        default:
            assert(0); // Not reached
            return 0;
    }
}

/*
 * GrowblesSocket Methods.
 */

GrowblesSocket::GrowblesSocket(ISocketHandler& h) : TcpSocket(h)
                                                  , mRemoteID(0)
{
    // We don't want TCP to buffer things up
    SetTcpNodelay();
}

void
GrowblesSocket::OnAccept()
{
    // Superclass method
    TcpSocket::OnAccept();

    // This should only be used for server-side sockets.
    // ConnectAsClient() shouldn't run.
    assert(mRemoteID == 0);

    // We start by sending clients the magic word
    unsigned message[3];
    message[0] = sGrowblesMagic;

    // Then we send them our ID
    Communicator* comm = dynamic_cast<GrowblesHandler&>(Handler()).GetCommunicator();
    message[1] = comm->mPlayerID;

    // Then we send them their player ID
    SetRemoteID(comm->mNextPlayerID++);
    message[2] = mRemoteID;

    // Send
    SendBuf((const char *)&message, sizeof(message));
}

unsigned
GrowblesSocket::GetRemoteID()
{
    assert(mRemoteID != 0);
    return mRemoteID;
}

void
GrowblesSocket::SetRemoteID(unsigned id)
{
    // ID should only be set once
    assert(mRemoteID == 0);
    mRemoteID = id;
}

void
GrowblesSocket::SendPayload(Payload& payload)
{
    // We're using TCP_NODELAY, which sends data immediately. However, we want
    // our payload to go in a single packet. So we create a buffer here for
    // the packet.
    unsigned dataSize = payload.GetDataSize();
    unsigned buffSize = sizeof(payload.type) + sizeof(dataSize) + dataSize;
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
    SendBuf(buffer, buffSize);
}

bool
GrowblesSocket::HasPayload()
{
    // If we're waiting for the data portion of a payload
    if (mIncoming.type != PAYLOAD_TYPE_NONE)
        return GetInputLength() >= mIncoming.GetDataSize();

    // Otherwise, we're starting from scratch. See if the header's there.
    if (GetInputLength() < sizeof(PayloadType) + sizeof(PayloadType))
        return false;

    // We've received the header. Read it in and recur.
    unsigned dataSize;
    ReadInput((char*)&mIncoming.type, sizeof(mIncoming.type));
    ReadInput((char*)&dataSize, sizeof(dataSize));
    assert(dataSize == mIncoming.GetDataSize()); // Make sure compilers pack
                                                 // the structs the same way.
    return HasPayload();
}

void
GrowblesSocket::GetPayload(Payload& payload)
{
    // We must have a payload ready
    assert(HasPayload());

    // Set the type
    payload.type = mIncoming.type;

    // Allocate the data buffer
    payload.data = malloc(payload.GetDataSize());
    assert(payload.data);

    // Read the data from the socket input buffer
    ReadInput((char*)payload.data, payload.GetDataSize());

    // The given payload owns the data now
    payload.ownData = true;

    // Clear our incoming tracker
    mIncoming.type = PAYLOAD_TYPE_NONE;
}

/*
 * GrowblesHandler Methods.
 */

GrowblesHandler::GrowblesHandler(Communicator& c) : SocketHandler()
                                                  , mCommunicator(&c)
{
}

void
GrowblesHandler::AddPlayers(WorldModel& model)
{
    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it)
        model.AddPlayer(dynamic_cast<GrowblesSocket*>(it->second)->GetRemoteID());
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
        if (socket->GetRemoteID() != excluded)
            socket->SendPayload(payload);
    }
}

void
GrowblesHandler::SendTo(Payload& payload, unsigned playerID)
{
    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it) {
        GrowblesSocket* socket = dynamic_cast<GrowblesSocket*>(it->second);
        if (socket->GetRemoteID() == playerID) {
            socket->SendPayload(payload);
            return;
        }
    }
}

bool
GrowblesHandler::HasPayload()
{
    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it) {
        GrowblesSocket* socket = dynamic_cast<GrowblesSocket*>(it->second);
        if (socket->HasPayload())
            return true;
    }
    return false;
}

unsigned
GrowblesHandler::ReceivePayload(Payload& payload)
{
    // We must have a payload available
    assert(HasPayload());

    for (socket_m::iterator it = m_sockets.begin();
         it != m_sockets.end(); ++it) {
        GrowblesSocket* socket = dynamic_cast<GrowblesSocket*>(it->second);
        if (socket->HasPayload()) {
            socket->GetPayload(payload);
            return socket->GetRemoteID();
        }
    }

    // We should never get here
    assert(0);
    return 0;
}

/*
 * Communicator Methods.
 */

Communicator::Communicator(Timeline& timeline,
                           CommunicatorMode mode) : mTimeline(&timeline)
                                                  , mMode(mode)
                                                  , mPlayerID(0)
                                                  , mNextPlayerID(1)
                                                  , mNumClientsExpected(0)
                                                  , mSocketHandler(*this)
{
    // If we're a server, assign ourselves a player ID
    if (mode == COMMUNICATOR_MODE_SERVER)
        mPlayerID = mNextPlayerID++;
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
    unsigned openingMessage[3];
    while (socket->GetInputLength() < sizeof(openingMessage))
        mSocketHandler.Select();
    socket->ReadInput((char *)&openingMessage, sizeof(openingMessage));

    // verify the magic word
    if (openingMessage[0] != sGrowblesMagic) {
        printf("Received bad magic word (%x) from server!\n", openingMessage[0]);
        exit(-1);
    }

    // Set the server's ID
    socket->SetRemoteID(openingMessage[1]);

    // Save our player ID
    mPlayerID = openingMessage[2];
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
    // Note that we want GetNumActiveSockets() to be one more than our desired
    // number of clients, because the handler is holding onto the ListenSocket
    // too.
    while (mSocketHandler.GetNumActiveSockets() < mNumClientsExpected + 1)
        mSocketHandler.Select(1,0);
}

void
Communicator::Synchronize()
{
    // Queue up any input we might have, but don't wait
    mSocketHandler.Select(0, 0);

    // Read in all payloads
    while (mSocketHandler.HasPayload()) {

        // Get the payload
        Payload incoming;
        mSocketHandler.ReceivePayload(incoming);

        // Handle each type
        switch (incoming.type) {

            // Worldstate dumps should only come from the server.
            case PAYLOAD_TYPE_WORLDSTATE:
                assert(mMode == COMMUNICATOR_MODE_CLIENT);
                assert(0); // We don't send these yet
                //model.SetState(*(WorldState*)incoming.data);
                break;

            // User inputs can come from anyone. The server forwards received
            // inputs to everyone else.
            case PAYLOAD_TYPE_USERINPUT:
                mTimeline->AddInput(*(UserInput*)incoming.data);
                if (mMode == COMMUNICATOR_MODE_SERVER)
                    mSocketHandler.SendToAllExcept(incoming,
                                                   ((UserInput*)incoming.data)
                                                   ->playerID);
                break;

            default:
                assert(0);
                break;
        }
    }
}

void
Communicator::Bootstrap(WorldModel& world)
{
    // If we're the server
    if (mMode == COMMUNICATOR_MODE_SERVER) {

        // Add the server player
        world.AddPlayer(mPlayerID);

        // Add the client players
        mSocketHandler.AddPlayers(world);

        // Send the state to all clients
        WorldState state;
        world.GetState(state);
        Payload payload(PAYLOAD_TYPE_WORLDSTATE, &state);
        mSocketHandler.SendToAll(payload);
    }

    // If we're the client
    else {

        // Receive the worldstate payload
        Payload received;
        while (!mSocketHandler.HasPayload())
            mSocketHandler.Select();
        mSocketHandler.ReceivePayload(received);
        assert(received.type == PAYLOAD_TYPE_WORLDSTATE);

        // Apply it
        world.SetState(*(WorldState*)received.data);
    }

    // Start our timeline
    mTimeline->Init(world, mMode);
}

void
Communicator::ApplyInput(UserInput& input)
{
    // Apply it to our timeline
    mTimeline->AddInput(input);

    // Create the payload
    Payload outgoing;
    outgoing.type = PAYLOAD_TYPE_USERINPUT;
    outgoing.data = &input;

    // Send to all. For servers, this means all clients. For clients, this
    // means just the server.
    mSocketHandler.SendToAll(outgoing);
}
