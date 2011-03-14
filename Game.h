#ifndef GAME_H
#define GAME_H

#include "Framework.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include "Communicator.h"
#include "Gameclock.h"
#include "Timeline.h"
#include "UserInput.h"
#include "Menu.h"

class Game {
    
public:
    
    /*
     * Constructor
     */
    Game(Timeline& tl, Communicator& comm);
    
    /*
     * Destructor
     */
    ~Game();
    
    void Setup();
    
    /*
     * Steps the model forward in time.
     */
    void Step();
    
    enum STATE {
        MENU,
        START,
        PLAYING,
        END
    };
    
protected:
    
    Gameclock* clock;
    RenderContext* renderContext;
    SceneGraph* sceneGraph;
    WorldModel* world;
    Timeline* timeline;
    Communicator* communicator;
    Menu* mainMenu;
    unsigned state;
    float prevPlayerX, prevPlayerZ;
};

#endif