#include "Framework.h"
#include "RenderContext.h"
#include "SceneGraph.h"
#include "WorldModel.h"
#include "Communicator.h"
#include "UserInput.h"
#include "Player.h"
#include "Timeline.h"
#include "Gameclock.h"
#include "Game.h"
#include <stdlib.h>


char* getOption(int argc, char** argv, const char* flag);
void printUsageAndExit(char* programName);

int main(int argc, char** argv) {

    printf("world state size: %u\n", sizeof(WorldState));
    printf("playerinfo size: %u\n", sizeof(PlayerInfo));

    // Random seed
#ifdef _WIN32
    srand(123456);
#else
    srandom(123456);
#endif
    
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
    
    // Load sound file into sound buffer, sound cannot be used for music
    /*
    sf::SoundBuffer Buffer;
    if (!Buffer.LoadFromFile("scenefiles/bgm.ogg"))
    {
        std::cout << "Error loading sound file\n";
    }
    // Bind sound buffer to sound
    sf::Sound Sound;
    Sound.SetBuffer(Buffer);
    //Sound.SetLoop(true);
    //Sound.SetPitch(1.5f);
    //Sound.SetVolume(75.f);
    Sound.Play();
     */
    
    sf::Music Music;
    if (!Music.OpenFromFile("scenefiles/bgm.ogg"))
    {
        std::cout << "Error loading music file\n";
    }    
    Music.Play();

    

    Game growblesGame(timeline, communicator);
    growblesGame.Setup();

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
