#include "Framework.h"
#include "RenderContext.h"

class Game {
    
public:
    
    /*
     * Dummy constructor.
     */
    Game(RenderContext& renderContext);
    
    /*
     * Destructor.
     */
    ~Game();
    
    /*
     * Steps the model forward in time.
     */
    void Step(WorldModel& world);
    
protected:
    
    RenderContext* rc;
};