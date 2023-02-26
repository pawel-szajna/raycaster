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

    explicit Player(const GameConfig& config);

    const Position& getPosition() const { return position; }

    void switchLevel();
    Level& currentLevel();

    void handleMovement(uint8_t* keys, double frameTime);
    void shoot();
    int blink();

private:
    Position position;
    PlayerLevels levels{};
    PlayerLevels::iterator current;
};

void LoadText(char* texts);

