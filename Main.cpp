#include "Framework.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include "WorldModel.h"
#include "Communicator.h"
#include "UserInput.h"
#include "Player.h"
#include "Timeline.h"
#include "Gameclock.h"
#include <stdlib.h>

char* getOption(int argc, char** argv, const char* flag);
void printUsageAndExit(char* programName);

int main(int argc, char** argv) {

    // Random seed
#ifdef _WIN32
    srand(123456);
#else
    srandom(123456);
#endif

    // Gameclock
    Gameclock clock(GAMECLOCK_TICK_MS);

    // Declare and initialize our rendering context
    RenderContext renderContext;
    renderContext.Init();

    // Declare an empty scenegraph
    SceneGraph sceneGraph(renderContext);

    // Declare our world model, and point it to the scene graph
    WorldModel world;
    world.Init(sceneGraph);

    // Client or server mode?
    char* modeString = getOption(argc, argv, "-m");
    CommunicatorMode mode = COMMUNICATOR_MODE_NONE;
    if (!strcmp(modeString, "client"))
        mode = COMMUNICATOR_MODE_CLIENT;
    else if (!strcmp(modeString, "server"))
        mode = COMMUNICATOR_MODE_SERVER;
    else
        printUsageAndExit(argv[0]);

    // Declare our timeline. It will be initialized by the Communicator.
    Timeline timeline;

    // Declare our communicator
    Communicator communicator(timeline, mode);

    // If we're a client, who are we connecting to?
    if (mode == COMMUNICATOR_MODE_CLIENT)
        communicator.SetServer(getOption(argc, argv, "-s"));

    // Otherwise, how many clients are we waiting for?
    else {
        int numClients = atoi(getOption(argc, argv, "-n"));
        if (numClients < 0)
            printUsageAndExit(argv[0]);
        communicator.SetNumClientsExpected((unsigned) numClients);
    }

    // Connect to the server/clients
    communicator.Connect();

    // Put the players on the map and get people on the same page
    communicator.Bootstrap(world);

    // Start the clock
    clock.Start();

    // Top level game loop
    while (renderContext.GetWindow()->IsOpened()) {

        // Handle input. Local input is applied immediately, global input
        // is recorded so that we can send it over the network.
        UserInput input(communicator.GetPlayerID(), clock.Now());
        input.LoadInput(renderContext);
        if (input.inputs != 0)
            communicator.ApplyInput(input);

        // Apply any state updates that may have come in, and send off any
        // necessary updates.
        communicator.Synchronize();

        // Tick the clock
        clock.Tick();

        // Step the world
        world.Step(clock.Now() - clock.Then());

        // Render the scenegraph
        renderContext.Render(sceneGraph);

        // Display the window
        renderContext.GetWindow()->Display();

    }

    return 0;
}

char* getOption(int argc, char** argv, const char* flag)
{
    // Search for the flag
    for (int i = 0; i < argc - 1; ++i)
        if (!strcmp(argv[i], flag))
            return argv[i + 1];

    // If the flag wasn't found, bail out.
    printUsageAndExit(argv[0]);

    // Not reached
    return NULL;
}

void printUsageAndExit(char* programName)
{
    printf("Usage: %s -m [client,server] [-s address | -n numClients]\n", programName);
    exit(-1);
}
