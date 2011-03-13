#include "UserInput.h"
#include "RenderContext.h"
#include "WorldModel.h"

UserInput::UserInput(unsigned playerID_, unsigned timestamp_) : inputs(0)
                                                              , timestamp(timestamp_)
                                                              , playerID(playerID_)
{
}


void
UserInput::LoadInput(RenderContext& context)
{
    static int sLastMouseX = 0;
    static int sLastMouseY = 0;
    static bool sMouseInitialized = false;

    sf::Event evt;
    while (context.GetWindow()->GetEvent(evt)) {

        /*
         * Local inputs. These get applied directly to the rendering context
         * and aren't communicated over the network.
         */
        switch (evt.Type) {
            case sf::Event::Closed:
                context.GetWindow()->Close();
                break;
            case sf::Event::Resized:
                context.SetViewportAndProjection();
                break;
            case sf::Event::KeyPressed:
                switch(evt.Key.Code) {
                    case sf::Key::W:
                        context.MoveCamera(1.0, 0.0);
                        break;
                    case sf::Key::S:
                        context.MoveCamera(-1.0, 0.0);
                        break;
                    case sf::Key::A:
                        context.MoveCamera(0.0, -1.0);
                        break;
                    case sf::Key::D:
                        context.MoveCamera(0.0, 1.0);
                        break;
                    case sf::Key::Left:
                        context.MoveLight(0.0, -0.1);
                        break;
                    case sf::Key::Right:
                        context.MoveLight(0.0, 0.1);
                        break;
                    case sf::Key::Down:
                        context.MoveLight(-0.1, 0.0);
                        break;
                    case sf::Key::Up:
                        context.MoveLight(0.1, 0.0);
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
                // The arguments are +pitch and +yaw. Positive pitch looks up,
                // positive yaw looks right.
                // Y is relative to the top of the window, X is relative to the
                // left of the window.
                /*
                context.PanCamera(-(evt.MouseMove.Y - sLastMouseY),
                                  evt.MouseMove.X - sLastMouseX);
                                  */

                // Save the new current value for next time
                sLastMouseX = evt.MouseMove.X;
                sLastMouseY = evt.MouseMove.Y;
                break;

            default:
                break;
        }

        /*
         * Global inputs. These affect the state of other players. As such,
         * we record them, and then apply them later.
         */

        // Is it a keypress?
        bool isPress = evt.Type == sf::Event::KeyPressed;

        switch (evt.Type) {
            case sf::Event::KeyPressed:
            case sf::Event::KeyReleased:

                // Handle each key
                switch(evt.Key.Code) {
                    case sf::Key::I:
                        inputs |= GEN_INPUT_MASK(USERINPUT_INDEX_GROW, isPress);
                        break;
                    case sf::Key::K:
                        inputs |= GEN_INPUT_MASK(USERINPUT_INDEX_SHRINK, isPress);
                        break;
                    case sf::Key::T:
                        inputs |= GEN_INPUT_MASK(USERINPUT_INDEX_UP, isPress);
                        break;
                    case sf::Key::G:
                        inputs |= GEN_INPUT_MASK(USERINPUT_INDEX_DOWN, isPress);
                        break;
                    case sf::Key::F:
                        inputs |= GEN_INPUT_MASK(USERINPUT_INDEX_LEFT, isPress);
                        break;
                    case sf::Key::H:
                        inputs |= GEN_INPUT_MASK(USERINPUT_INDEX_RIGHT, isPress);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}
