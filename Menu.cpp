#include "Menu.h"

Menu::Menu(RenderContext* rc)
{
    renderContext = rc;
    currentMenu = 0;
    currentSelected = 0;
    
    // The main menu
    menus[0].numStrings=1;
    menus[0].strings[0].text="Growbles!";
    menus[0].strings[0].x= 100;
    menus[0].strings[0].y= 100;
    menus[0].strings[0].size = 100;
    menus[0].strings[0].r = 230;
    menus[0].strings[0].g = 230;
    menus[0].strings[0].b = 230;
    
    menus[0].numItems=4;
    menus[0].items[0].text="Start Game";
    menus[0].items[0].x= 150;
    menus[0].items[0].y= 250;
    menus[0].items[0].size = 30;
    menus[0].items[0].r = 128;
    menus[0].items[0].g = 0;
    menus[0].items[0].b = 0;
    //menus[0].items[0].newState = PLAYING;
    
    menus[0].items[1].text="Exit";
    menus[0].items[1].x= 150;
    menus[0].items[1].y= 300;
    menus[0].items[1].size = 30;
    menus[0].items[1].r = 128;
    menus[0].items[1].g = 0;
    menus[0].items[1].b = 0;
    //menus[0].items[1].newState = EXIT;
}

int
Menu::update()
{
    sf::Event evt;
    while (renderContext->GetWindow()->GetEvent(evt)) {
        switch (evt.Type) {
            case sf::Event::Closed:
                renderContext->GetWindow()->Close();
                break;
            case sf::Event::Resized:
                renderContext->SetViewportAndProjection();
                break;
            case sf::Event::KeyPressed:
                switch(evt.Key.Code) {
                    case sf::Key::Up:
                        //renderContext->RenderString("Up pressed", 1000);
                        currentSelected = 1 - currentSelected;
                        break;
                    case sf::Key::Down:
                        //renderContext->RenderString("Down pressed", 1000);
                        currentSelected = 1 - currentSelected;
                        break;
                    case sf::Key::Return:
                        //renderContext->RenderString("Return pressed", 1000);
                        if (currentSelected == 0) return 1; // Start game
                        else return 2; // Exit
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    
    // Update color of highlighted item
    for (int i=0; i<menus[0].numItems; i++) {
        if (i == currentSelected) menus[0].items[i].r = 255;
        else menus[0].items[i].r = 128;
    }
    
    return 0; // No selection made
}

void
Menu::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor( 0.7, 0.5, 0.1, 1.0);
    
    /*
    // Disable the shader so we can draw the platform using the fixed pipeline
    GL_CHECK(glUseProgram(0));
    
    glDisable(GL_DEPTH_TEST);

    
    glNormal3f(0,0,1);
    
    // Draw box around selected item
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);

    glColor4f(1.0, 1.0, 1.0, 1.0);

    //position the box under the selected item
    //glTranslatef(2.0, menus[0].items[currentSelected].y, 4.0);

    glBegin(GL_QUADS);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 100, 0);
        glVertex3f(400, 100, 0);
        glVertex3f(400, 0, 0);
    glEnd();

    glPopMatrix();
     */
     
    //RenderString function signature:
    //void RenderString(std::string str, unsigned duration=32, unsigned size=30, float x=100.0, float y=100.0, unsigned r=128, unsigned g=0, unsigned b=128);

    // Render title
    for (int i=0; i<menus[0].numStrings; i++)
    {
        renderContext->RenderString(menus[0].strings[i].text, 32,
                                    menus[0].strings[i].size, 
                                    menus[0].strings[i].x, menus[0].strings[i].y,
                                    menus[0].strings[i].r, menus[0].strings[i].g, menus[0].strings[i].b);        
    }

    // Render menu items
    for (int i=0; i<menus[0].numItems; i++)
    {
        renderContext->RenderString(menus[0].items[i].text, 32,
                                    menus[0].items[i].size, 
                                    menus[0].items[i].x, menus[0].items[i].y,
                                    menus[0].items[i].r, menus[0].items[i].g, menus[0].items[i].b);
    }

    glEnable(GL_DEPTH_TEST);
    
    // Flush
    GL_CHECK(glFlush());
    
    // Reenable the shader
    GL_CHECK(glUseProgram(renderContext->GetShaderID()));
}