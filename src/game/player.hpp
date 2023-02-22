#pragma once

#include "SDL/SDL.h"
#include "data.hpp"
#include <unordered_map>

class GameConfig;

struct PlayerLevel
{
    int id;
    char visited[levelSize * levelSize]{};
    Items items{};
    NPCs npcs{};

    explicit PlayerLevel(int id) : id(id) {}
};

struct Position
{
    double x{}, y{};
    double dirX{}, dirY{};
    double planeX{}, planeY{};
};

class Player
{
public:
    using PlayerLevels = std::unordered_map<int, PlayerLevel>;

    bool revolver{false};
    bool flashlight{false};
    int bullets{0};
    int battery{0};

    int levelId;
    bool reloadLevel{true};

    PlayerLevel& currentLevel() { return current->second; }
    const PlayerLevel& currentLevel() const { return current->second; }

    explicit Player(const GameConfig& config);

    void switchLevel()
    {
        if (not levels.contains(levelId))
        {
            levels.emplace(levelId, PlayerLevel(levelId));
        }

        current = levels.find(levelId);
    }

    const Position& getPosition() const { return position; }

    void handleMovement(Uint8* keys, int* level, char* visited, double frameTime);
    void shoot(int* level);
    int blink();

private:
    Position position;
    PlayerLevels levels{};
    PlayerLevels::iterator current;
};

void LoadText(char* texts);

