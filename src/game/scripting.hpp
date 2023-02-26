#pragma once

#include <map>
#include <string>
#include <sol/sol.hpp>

class GameplayMode;
class Player;
class UI;

class Scripting
{
public:
    Scripting(GameplayMode& gameplay, Player& player, UI& ui);

    void loadScript(const std::string& filename);
    void runCallback(const std::string& callback);

private:

    void messageBox(const std::string& message);
    void addItem(int x, int y, int texture, const std::string& callback);

    GameplayMode& gameplay;
    Player& player;
    UI& ui;

    sol::state lua;
};
