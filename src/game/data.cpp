#include "data.hpp"

#include <spdlog/spdlog.h>

GameConfig::GameConfig()
{
    FILE* cfile;

    spdlog::info("Loading configuration file...");
    cfile = fopen("ray.cfg", "r");
    assert(cfile);
    fscanf(cfile, "%d %d %d %d %d %d %d %[^\n]", &sWidth, &sHeight, &fullScreen, &level, &pX, &pY, &dir, title);
    fclose(cfile);
}
