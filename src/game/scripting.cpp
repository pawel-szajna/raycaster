#include "scripting.hpp"

#include "gameplay.hpp"
#include "player.hpp"
#include "ui.hpp"

#include <spdlog/spdlog.h>

Scripting::Scripting(GameplayMode& gameplay, Player& player, UI& ui) :
    gameplay(gameplay),
    player(player),
    ui(ui)
{
    lua.open_libraries(sol::lib::base);
    lua.set_function("messageBox", &Scripting::messageBox, this);
    lua.set_function("addItem", &Scripting::addItem, this);
}

void Scripting::addItem(int x, int y, int texture, const std::string& callback)
{
    player.currentLevel().addItem(x, y, texture, callback);
}

void Scripting::messageBox(const std::string& message)
{
    ui.addMessageWindow("Message from LUA", message);
    gameplay.pause();
}

void Scripting::loadScript(const std::string& filename)
{
    spdlog::info("Executing LUA script: {}", filename);
    try
    {
        lua.script_file(filename);
    }
    catch (std::runtime_error& error)
    {
        spdlog::error("LUA error: {}", error.what());
    }
}

void Scripting::runCallback(const std::string& callback)
{
    try
    {
        lua[callback]();
    }
    catch (std::runtime_error& e)
    {
        spdlog::error("LUA failure: {}", e.what());
    }
}
