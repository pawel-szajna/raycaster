#pragma once

#include "data.hpp"
#include "level.hpp"

#include <format>
#include <unordered_map>
#include <cstdint>

class GameConfig;

struct Position
{
    double x{}, y{};
    double dirX{}, dirY{};
    double planeX{}, planeY{};
};

class Player
{
public:
    using PlayerLevels = std::unordered_map<int, Level>;

    bool revolver{false};
    bool flashlight{false};
    int bullets{0};
    int battery{0};

    int levelId;
    bool reloadLevel{true};

    Level& currentLevel() { return current->second; }

    explicit Player(const GameConfig& config);

    void switchLevel()
    {
        if (not levels.contains(levelId))
        {
            levels.emplace(levelId, Level(std::format("map/level{}.dat", levelId)));
        }

        current = levels.find(levelId);
    }

    const Position& getPosition() const { return position; }

    void handleMovement(uint8_t* keys, int* level, char* visited, double frameTime);
    void shoot(int* level);
    int blink();

private:
    Position position;
    PlayerLevels levels{};
    PlayerLevels::iterator current;
};

void LoadText(char* texts);

