#include "Game.h"

Game::Game(Timeline& tl, Communicator& comm)
{
    state = MENU;
    timeline = &tl;
    communicator = &comm;
}

Game::~Game()
{
    delete clock;
    delete renderContext;
    delete sceneGraph;
    delete world;
    delete mainMenu;
}

void
Game::Setup()
{
    // Gameclock
    clock = new Gameclock(GAMECLOCK_TICK_MS);
    
    // Declare and initialize our rendering context
    renderContext = new RenderContext;
    renderContext->Init();
    
    // Declare an empty scenegraph
    sceneGraph = new SceneGraph(*renderContext);
    
    // Declare our world model, and point it to the scene graph
    world = new WorldModel;
    world->Init(*sceneGraph);
    
    // Setup main menu
    mainMenu = new Menu(renderContext);
    
    // Top level game loop
    while (renderContext->GetWindow()->IsOpened()) {
        
        // Step the game
        Step();
        
        // Display the window
        renderContext->GetWindow()->Display();
    }
}

void
Game::Step()
{
    if (state == MENU) {
        // Update and render main menu
        int choice = mainMenu->update();
        mainMenu->render();
        
        // Render all strings
        renderContext->RenderAllElse();
        
        // Go to next state
        if (choice == 1) {
            state = START;
            glClear(GL_COLOR_BUFFER_BIT);
            if (communicator->GetMode() == COMMUNICATOR_MODE_SERVER) renderContext->RenderString("Waiting for client to connect...", 32, 40, 140, 250, 0, 255, 0);
            if (communicator->GetMode() == COMMUNICATOR_MODE_CLIENT) renderContext->RenderString("Connecting to server...", 32, 40, 140, 250, 0, 255, 0);
            renderContext->RenderAllElse();
        }
        else if (choice == 2) renderContext->GetWindow()->Close();
    } // EOF state == MENU
    
    else if (state == START) { // start the game
        
        // Connect to the server/clients
        communicator->Connect();
        
        // Put the players on the map and get people on the same page
        communicator->Bootstrap(*world);

        world->SetFalcon(renderContext->falcon);
        world->SetThisPlayer(communicator->GetPlayerID());
        
        // Start the clock
        clock->Start();
        
        renderContext->RenderString("Game Start", 1000, 40, 100, 100, 255, 0, 0);
        
        // Render all strings
        renderContext->RenderAllElse();
        
        // Go to next state
        state = PLAYING;
        
        // Get initial player location
        for(unsigned i=0; i < world->mPlayers.size(); ++i){
            Player* player = world->mPlayers[i];
            if (player->GetPlayerID() == communicator->GetPlayerID()) { // This is ourselves
                prevPlayerX = world->GetPlayerPosition(player->GetPlayerID()).x;
                prevPlayerZ = world->GetPlayerPosition(player->GetPlayerID()).z;
            }
        }
    } // EOF state == START
    
    else if (state == PLAYING) {
        
        // Handle input. Local input is applied immediately, global input
        // is recorded so that we can send it over the network.
        UserInput input(communicator->GetPlayerID(), clock->Now());
        input.LoadInput(*renderContext, *communicator, *world);
        if (input.inputs != 0 || input.falconInputs.x != 0 || 
            input.falconInputs.y != 0 || input.falconInputs.z != 0){
            communicator->ApplyInput(input);
        }
        
        // Apply any state updates that may have come in, and send off any
        // necessary updates.
        communicator->Synchronize();
        
        // Tick the clock
        clock->Tick();
        
        // Step the world
        world->Step(clock->Now() - clock->Then());
		
		// Apply Haptic Forces to Falcon
        world->ApplyHapticGravityForce();
        world->ApplyHapticCollisionForce();
        
        // Render the skybox
        renderContext->RenderSkybox();
        
        // Render the platform for debugging
        renderContext->RenderPlatform(*world);
        
        // Render the scenegraph
        //renderContext->Render(*sceneGraph);
        
        for(unsigned i=0; i < world->mPlayers.size(); ++i){
            Player* player = world->mPlayers[i];
            
            if (player->GetPlayerID() == communicator->GetPlayerID()) { // This is ourselves
                // Move the camera if necessary
                float newPlayerX = world->GetPlayerPosition(player->GetPlayerID()).x;
                float newPlayerZ = world->GetPlayerPosition(player->GetPlayerID()).z;
                renderContext->MoveCamera(newPlayerX - prevPlayerX, newPlayerZ - prevPlayerZ);
                prevPlayerX = newPlayerX;
                prevPlayerZ = newPlayerZ;
                
                // If height of player is below a certain point, mark player as lost
                if (world->GetPlayerPosition(player->GetPlayerID()).y < 1) {
                    renderContext->RenderString("You Lost", 100000);
                    // Go to next state
                    state = END;
                }
                
                
            }
            else { // The other player lost, we win
                if (world->GetPlayerPosition(player->GetPlayerID()).y < 1) {
                    renderContext->RenderString("You Win", 100000);
                    
                    // Go to next state
                    state = END;
                }
            }
            
        }
        
        // Render all strings
        renderContext->RenderAllElse();
        
    } // EOF state == Playing
    
    else if (state == END) {
        
        // Handle input. Local input is applied immediately, global input
        // is recorded so that we can send it over the network.
        UserInput input(communicator->GetPlayerID(), clock->Now());
        input.LoadInput(*renderContext, *communicator, *world);
        if (input.inputs != 0 || input.falconInputs.x != 0 || 
            input.falconInputs.y != 0 || input.falconInputs.z != 0){
            communicator->ApplyInput(input);
        }
        
        // Apply any state updates that may have come in, and send off any
        // necessary updates.
        communicator->Synchronize();
        
        // Tick the clock
        clock->Tick();
        
        // Step the world
        world->Step(clock->Now() - clock->Then());
		
		// Apply Haptic Forces to Falcon
        world->ApplyHapticGravityForce();
        world->ApplyHapticCollisionForce();
        
        // Render the skybox
        renderContext->RenderSkybox();
        
        // Render the platform for debugging
        renderContext->RenderPlatform(*world);
        
        // Render the scenegraph
        //renderContext->Render(*sceneGraph);
        
        for(unsigned i=0; i < world->mPlayers.size(); ++i){
            Player* player = world->mPlayers[i];
            
            if (player->GetPlayerID() == communicator->GetPlayerID()) { // This is ourselves
                // Move the camera if necessary
                float newPlayerX = world->GetPlayerPosition(player->GetPlayerID()).x;
                float newPlayerZ = world->GetPlayerPosition(player->GetPlayerID()).z;
                renderContext->MoveCamera(newPlayerX - prevPlayerX, newPlayerZ - prevPlayerZ);
                prevPlayerX = newPlayerX;
                prevPlayerZ = newPlayerZ;
            }
        }
        
        // Render all strings
        renderContext->RenderAllElse();
    } // EOF state == END
}
