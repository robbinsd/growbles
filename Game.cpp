#include "Game.h"

Game::Game(RenderContext& renderContext)
{
    rc = &renderContext;
}

Game::~Game()
{
}

void
Game::Step(WorldModel& world)
{
    for(unsigned i = 0; i < world.mPlayers.size(); ++i){
        Player* player = world.mPlayers[i];
        
        // if height of player is below a certain point, mark player as lost
        if (player->getPosition().y < 1) {
            rc->MakeString("You Lost");
            rc->DrawString();
        }
    }
}