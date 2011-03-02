#include "Framework.h"
#include "Shader.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include <stdlib.h>

#define ARMADILLO_PATH "scene/armadillo.3ds"
#define CATHEDRAL_PATH "scene/cathedral.3ds"
#define SPHERE_PATH "scene/sphere.3ds"


void handleInput(RenderContext& rContext);

int main(int argc, char** argv) {

    // Random seed
    srandom(123456);

    // Declare and initialize our rendering context
    RenderContext renderContext;
    renderContext.Init();

    // Declare and initialize our scenegraph
    SceneGraph sceneGraph;
    sceneGraph.Init(renderContext);
    sceneGraph.LoadScene(CATHEDRAL_PATH, "Cathedral", &sceneGraph.rootNode);
    sceneGraph.LoadScene(ARMADILLO_PATH, "Armadillo", &sceneGraph.rootNode);
    Vector emapPos(0.0, 3.0, 0.0, 1.0);
    sceneGraph.FindMesh("Armadillo_0")->EnvironmentMap(renderContext, emapPos);

    // Put your game loop here (i.e., render with OpenGL, update animation)
    while (renderContext.GetWindow()->IsOpened()) {

        handleInput(renderContext);
        renderContext.Render(sceneGraph);
        renderContext.GetWindow()->Display();
    }

    return 0;
}

void handleInput(RenderContext& rContext) {

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

