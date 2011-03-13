#include "Text.h"

void
Text::Text(sf::RenderWindow win) : window(win)
{
    /*
     sf::Font MyFont;
     
     // Load the font from a file
     sf::Font MyFont;
     if (!MyFont.LoadFromFile("arial.ttf", 50))
     {
     // Error...
     }
     */
    myText.SetText("Hello");
    myText.SetFont(sf::Font::GetDefaultFont());
    myText.SetSize(50);
    myText.SetColor(sf::Color(128, 128, 0));
    //myText.SetRotation(90.f);
    //myText.SetScale(2.f, 2.f);
    myText.Move(100.f, 200.f);
} 

void
Text::displayAt()
{
    sf::RenderWindow Window;
    Window.Draw(myText);
}