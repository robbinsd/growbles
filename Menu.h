#ifndef MENU_H
#define MENU_H

#include "Framework.h"
#include "RenderContext.h"
#include <string>

struct menuItem
{
    std::string text;
    float x, y;
    unsigned size;
    unsigned r, g, b;
};

struct menuInstance
{
    int numStrings;
    menuItem strings[50];
    int numItems;
    menuItem items[50];
};

class Menu
{
public:
    /*
     * Constructor
     */
    Menu(RenderContext* rc);
    int update();
    void render();
    
private:
    int currentMenu;
    int currentSelected;
    menuInstance menus[1]; // Just one menu for now
    RenderContext* renderContext;
};

#endif