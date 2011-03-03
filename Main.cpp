#include "Framework.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include "WorldModel.h"
#include "Communicator.h"
#include <stdlib.h>

#define ARMADILLO_PATH "scene/armadillo.3ds"
#define CATHEDRAL_PATH "scene/cathedral.3ds"
#define SPHERE_PATH "scene/sphere.3ds"


void handleInput(WorldModel& world, RenderContext& rContext);
char* getOption(int argc, char** argv, const char* flag);
void printUsageAndExit(char* programName);

int main(int argc, char** argv) {

    // Random seed
    srandom(123456);

    // Declare and initialize our rendering context
    RenderContext renderContext;
    renderContext.Init();

    // Declare an empty scenegraph
    SceneGraph sceneGraph;

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
        communicator.SetNumClients((unsigned) numClients);
    }

    // Connect to the server/clients
    communicator.Connect();

    // Declare and initialize our world model
    WorldModel world;
    world.Init(sceneGraph);

    // New functionality: The world model creates the scenegraph based on its
    // internal model, and updates it appropriately in response to MotionState
    // callbacks.
    //
    // We preserve the old cathedral functionality for the time being:
    sceneGraph.LoadScene(renderContext, CATHEDRAL_PATH, "Cathedral",
                         &sceneGraph.rootNode);
    sceneGraph.LoadScene(renderContext, ARMADILLO_PATH, "Armadillo",
                         &sceneGraph.rootNode);
    Vector emapPos(0.0, 3.0, 0.0, 1.0);
    sceneGraph.FindMesh("Armadillo_0")->EnvironmentMap(renderContext, emapPos);

    // Top level game loop
    while (renderContext.GetWindow()->IsOpened()) {

        // Handle input, which can affect both the world and the rendering context
        handleInput(world, renderContext);

        // Apply any state updates that may have come in, and send off any
        // necessary updates.
        communicator.Synchronize(world);

        // Step the world
        world.Step();

        // Render the scenegraph
        renderContext.Render(sceneGraph);

        // Display the window
        renderContext.GetWindow()->Display();
    }

    return 0;
}

void handleInput(WorldModel& world, RenderContext& rContext) {

    static int sLastMouseX = 0;
    static int sLastMouseY = 0;
    static bool sMouseInitialized = false;

    // Event loop, for processing user input, etc.  For more info, see:
    // http://www.sfml-dev.org/tutorials/1.6/window-events.php
    sf::Event evt;
    while (rContext.GetWindow()->GetEvent(evt)) {
        switch (evt.Type) {
        case sf::Event::Closed:
            // Close the window.  This will cause the game loop to exit,
            // because the IsOpened() function will no longer return true.
            rContext.GetWindow()->Close();
            break;
        case sf::Event::Resized:
            // If the window is resized, then we need to change the perspective
            // transformation and viewport
            rContext.SetViewportAndProjection();
            break;

        case sf::Event::KeyPressed:

            switch(evt.Key.Code) {

                case sf::Key::W:
                    rContext.MoveCamera(1.0, 0.0);
                    break;
                case sf::Key::S:
                    rContext.MoveCamera(-1.0, 0.0);
                    break;
                case sf::Key::A:
                    rContext.MoveCamera(0.0, -1.0);
                    break;
                case sf::Key::D:
                    rContext.MoveCamera(0.0, 1.0);
                    break;
                case sf::Key::Left:
                    rContext.MoveLight(0.0, -0.1);
                    break;
                case sf::Key::Right:
                    rContext.MoveLight(0.0, 0.1);
                    break;
                case sf::Key::Down:
                    rContext.MoveLight(-0.1, 0.0);
                    break;
                case sf::Key::Up:
                    rContext.MoveLight(0.1, 0.0);
                    break;

                default:
                    break;
            }

            break;

        case sf::Event::MouseMoved:

            // If we're not initialized, calibrate
            if (!sMouseInitialized) {
                sLastMouseX = evt.MouseMove.X;
                sLastMouseY = evt.MouseMove.Y;
                sMouseInitialized = true;
            }

            // Pan the camera
            // The arguments are +pitch and +yaw. Positive pitch looks up, positive yaw looks right.
            // Y is relative to the top of the window, X is relative to the left of the window.
            rContext.PanCamera(-(evt.MouseMove.Y - sLastMouseY), evt.MouseMove.X - sLastMouseX);

            // Save the new current value for next time
            sLastMouseX = evt.MouseMove.X;
            sLastMouseY = evt.MouseMove.Y;
            break;

        default:
            break;
        }
    }
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

