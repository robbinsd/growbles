#include "Framework.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include "WorldModel.h"
#include "Communicator.h"
#include "UserInput.h"
#include <stdlib.h>
#include "Player.h"
#include "Platform.h"

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

    UserInput input(communicator.GetPlayerID(), currTimestamp);
    
    bool testPlatform = false;
    //Platform platform(5);
    //platform.reset();

    // Top level game loop
    while (renderContext.GetWindow()->IsOpened()) {

        // Handle input. Local input is applied immediately, global input
        // is recorded so that we can send it over the network.
        input.resetInputState();
        input.LoadInput(renderContext);
        input.ApplyInput(world);
        communicator.SendInput(input);

        // Apply any state updates that may have come in, and send off any
        // necessary updates.
        communicator.Synchronize(world);

        // Step the world
        world.Step();
        
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (testPlatform) { // work in progress
            GL_CHECK(glMatrixMode(GL_MODELVIEW));
            GL_CHECK(glPushMatrix());
            GL_CHECK(glLoadIdentity());
            GL_CHECK(glMatrixMode(GL_PROJECTION));
            GL_CHECK(glPushMatrix());
            GL_CHECK(glLoadIdentity());
            //GL_CHECK(glOrtho(-0.1, 1.1, -0.1, 1.1, -1.0, 1.0));
            
            // Disable the shader (use the fixed-function pipeline)
            GL_CHECK(glUseProgram(0));
            
            // Draw our quad
            glBegin(GL_QUADS);
            glVertex2d(0.0,0.0);
            glVertex2d(10.0,0.0);
            glVertex2d(10.0,10.0);
            glVertex2d(0.0,10.0);
            glEnd();
            //platform.render();
            
            // Flush
            GL_CHECK(glFlush());
            
            // Reenable the shader
            GL_CHECK(glUseProgram(renderContext.GetShaderID()));
            
            // Restore our old matrices
            GL_CHECK(glMatrixMode(GL_MODELVIEW));
            GL_CHECK(glPopMatrix());
            GL_CHECK(glMatrixMode(GL_PROJECTION));
            GL_CHECK(glPopMatrix());
        }
        
        
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
