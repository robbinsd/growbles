#include "Framework.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include "WorldModel.h"
#include "Communicator.h"
#include "UserInput.h"
#include <stdlib.h>
#include "Player.h"
sf::Clock clck;

char* getOption(int argc, char** argv, const char* flag);
void printUsageAndExit(char* programName);

int main(int argc, char** argv) {

    // Random seed
#ifdef _WIN32
    srand(123456);
#else
    srandom(123456);
#endif

    // Dummy timestamp. This should be replaced with our actual
    // timestamp once the game clock gets going.
    unsigned currTimestamp = 123456;

    // Declare and initialize our rendering context
    RenderContext renderContext;
    renderContext.Init();

    // Declare an empty scenegraph
    SceneGraph sceneGraph(renderContext);

    // Client or server mode?
    char* modeString = getOption(argc, argv, "-m");
    CommunicatorMode mode = COMMUNICATOR_MODE_NONE;
    if (!strcmp(modeString, "client"))
        mode = COMMUNICATOR_MODE_CLIENT;
    else if (!strcmp(modeString, "server"))
        mode = COMMUNICATOR_MODE_SERVER;
    else
        printUsageAndExit(argv[0]);

    // Declare our communicator
    Communicator communicator(mode);

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

    // Declare and initialize our world model
    //
    // The Communicator needs to initialize the world, because it knows how many
    // players there are.
    WorldModel world;
    communicator.InitWorld(world, sceneGraph);

    // Top level game loop
    while (renderContext.GetWindow()->IsOpened()) {

        // Handle input. Local input is applied immediately, global input
        // is recorded so that we can send it over the network.
        UserInput input(communicator.GetPlayerID(), currTimestamp);
        input.LoadInput(renderContext);
        world.ApplyInput(input);
        communicator.SendInput(input);

        // Apply any state updates that may have come in, and send off any
        // necessary updates.
        communicator.Synchronize(world);

        // Step the world
        world.Step(clck, renderContext.GetShaderID());
        
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
