#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "sprig.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

#include "data.hpp"
#include "level.hpp"
#include "caster.hpp"
#include "player.hpp"
#include "ai.hpp"
#include "ui.hpp"
#include "generator.hpp"
#include "sdl.hpp"

#ifndef NDEBUG
#undef main
#endif

int OnKeyPress(SDL_Event* event, int key)
{
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == key)
    {
        event->type = 0;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
	int level[levelSize][levelSize];
	char visited[levelSize][levelSize];

	int paused = 0;
	int flashlight = 1;
	int popup = 0;
	int popupmap = 0;
	int danger_level;
	GameMode mode;

	char texts[6][128];

	double nearest = 20;

	LevelInfo levelinfo;
	NPCs npcs{};

	SDL_Surface* screen;
	SDL_Surface* map;
	SDL_Surface* shade;
	SDL_Surface* noise_big;
	SDL_Surface* popupWindow;
	SDL_Surface* gun_hand;
	SDL_Surface* gun_shoot;

	SDL_Event event = { 0 };
	SDL_Rect r1 = { 0, 60, 0, 0 }, /* world view */
			 r2 = { 84, 210, 0, 0 }, /* world & noise target */
			 r4 = {0, 0, 8 * levelSize, 8 * levelSize }, /* map target */
			 r8 = { 0, 0, 128, 128 },
			 r9 = { wwWidth / 2 - 29, wwHeight - 100, 59, 100 }; /* popup */

	Uint8* keys;
	
	double newTime = 0, oldTime = 0;
	double frameTime;

	char filename[128];

	srand(time(NULL));

	printf("RCFPGE Engine                                                        version 0.4"
		   "================================================================================\n");

	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

    GameConfig config{};
    Player player(config);
	LoadText((char*)texts);
	InitUI();
	
	screen = SDL_SetVideoMode(config.sWidth, config.sHeight, 32, SDL_HWSURFACE | (config.fullScreen ? SDL_FULLSCREEN : 0));
	auto worldview = sdl::make_surface(wwWidth, wwHeight);
    auto noise = sdl::make_alpha_surface(160, 90);

	shade = SDL_LoadBMP("gfx/shade.bmp");

	assert(screen);
	assert(shade);

	SDL_ShowCursor(SDL_DISABLE);

	SDL_SetColorKey(shade, SDL_SRCCOLORKEY, 0x00ffffff);

	mode = GameMode::Initial;

	/* for map drawing */
	/* TODO: make this cleaner*/
	r4.x = config.sWidth / 2 - 256;
	r4.y = config.sHeight / 2 - 246;
	danger_level = 1;

	for(;;) switch(mode)
	{
		case GameMode::Initial:
		SDL_WM_SetCaption(config.title, NULL);
		for(;;)
		{
			if(danger_level > 12)
			{
				generateNoiseLinear(noise, danger_level);
				danger_level -= 2;
			}
			else
			{
				generateNoise(noise, 12);
			}
			
			noise_big = SPG_Transform(*noise, 0, 0, 4, 4, 0);
			assert(noise_big);

            worldview = sdl::make_surface(wwWidth, wwHeight);

            uiFontNotice.render(worldview, r2, "Prece enter key");
            uiFontTitle.render(worldview, r1, "Techdemo");

			SDL_BlitSurface(noise_big, 0, *worldview, 0);
			SDL_SoftStretch(*worldview, 0, screen, 0);
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			SDL_Delay(40);

			SDL_PollEvent(&event);

			if(OnKeyPress(&event, SDLK_ESCAPE) || OnKeyPress(&event, SDLK_q) || event.type == SDL_QUIT)
			{
				mode = GameMode::Quit;
				break;
			}

			if(OnKeyPress(&event, SDLK_RETURN)) break;
		}
		if(mode != GameMode::Quit) mode = GameMode::MainMenu;
		break;

		case GameMode::MainMenu:
        player = Player(config);
        mode = GameMode::Game;
        break;

		case GameMode::Game:
		gun_hand = SDL_LoadBMP("gfx/gun/gun1.bmp");
		gun_shoot = SDL_LoadBMP("gfx/gun/gun2.bmp");
		assert(gun_hand);
		assert(gun_shoot);
		SDL_SetColorKey(gun_hand, SDL_SRCCOLORKEY, 0x00ffffff);
		SDL_SetColorKey(gun_shoot, SDL_SRCCOLORKEY, 0x00ffffff);

		for(;;)
		{
			if(player.reloadLevel) /* load new level */
			{
				sprintf(filename, "map/level%d.dat", player.levelId);
				LoadLevel((int*)level, &levelinfo, npcs, filename);
				generate_map((int*)level, (int)player.getPosition().x, (int)player.getPosition().y, 1);
				npcs = generate_npcs((int*)level);
				InitAI((int*)level);
				InitUI();
				sprintf(filename, "%s: Level %d", config.title, player.levelId);
				SDL_WM_SetCaption(filename, NULL);
				worldview = InitCaster((int*)level, &levelinfo);

                player.switchLevel();
				npcs = player.currentLevel().npcs;
                ResetAI(npcs);

				AddItem(player.currentLevel().items, 27, 46, 0);
				AddItem(player.currentLevel().items, 27, 47, 1);
				AddItem(player.currentLevel().items, 27, 48, 3);

				player.reloadLevel = false;
			}

			keys = SDL_GetKeyState(NULL);

			/* render frame */
			if(!paused)
			{
				CastFrame(*worldview, (int*)level, &player, flashlight && player.blink());
				if(player.revolver) SDL_BlitSurface(gun_hand, 0, *worldview, &r9);
			}
			generateNoise(noise, danger_level);
			noise_big = SPG_Transform(*noise, 0, 0, 4, 4, 0);
			assert(noise_big);
			SDL_BlitSurface(noise_big, 0, *worldview, 0);
			SDL_SoftStretch(*worldview, 0, screen, 0);
			SPG_Free(noise_big);

			/* draw UI */

			/* draw popups */
			if(popup) SDL_BlitSurface(popupWindow, NULL, screen, &r8);
			if(popupmap) SDL_BlitSurface(map, 0, screen, &r4);

			/* close popups */
			if(popup && OnKeyPress(&event, SDLK_RETURN))
			{
				if(popupmap) SDL_FreeSurface(map);
				SDL_FreeSurface(popupWindow);

				popup = 0;
				popupmap = 0;
				paused = 0;
			}

			/* map */
			if(OnKeyPress(&event, SDLK_m) && !popup && !popupmap)
			{
				paused = 1;
				popup = 1;
				popupmap = 1;

				popupWindow = MakeWindow(556, 556, "Mapa", &r8, &config);
				map = DrawMap((int*)level, &player);
				assert(popupWindow);
				assert(map);
			}

			/* draw everything */
			SDL_UpdateRect(screen, 0, 0, 0, 0);

			SDL_PollEvent(&event);
			oldTime = newTime;
			newTime = SDL_GetTicks();
			frameTime = (newTime - oldTime);
			// if(frameTime < 16.67) SDL_Delay(16.67 - frameTime);
			frameTime /= 1000.0;
			sprintf(filename, "%s: %s [%d fps]", config.title, levelinfo.name, (int)(1 / frameTime));
			SDL_WM_SetCaption(filename, NULL);

			/* simulate the game world */
			if(!paused)
			{
				nearest = AI_DistanceToNearestNPC(&player);
				popup = AI_Tick(&player, frameTime, flashlight);
				player.handleMovement(keys, (int*)level, (char*)visited, frameTime);

				if(popup)
				{
					popupWindow = MessageWindow("", texts[popup], &r8, &config);
					assert(popupWindow);
					paused = 1;
					popup = 1;
				}
			}

			/*printf("Nearest headless: %f\n", nearest);*/
			danger_level = (nearest < 4) ? ( (nearest < 0.5) ? 128 : ceil(nearest * (nearest * (5.9242 * nearest - 39.9883) + 35.5452) + 119.484) ) : (!(rand() % 5) ? 2 : 1);

			/* player dies */
			if(nearest < 0.5)
			{
				mode = GameMode::GameOver;
				break;
			}

			/* some functions */
			/* saving is disabled in techdemo */
			/*if(OnKeyPress(&event, SDLK_s)) SaveGame(&player, npcs, &levelinfo, (char*)visited);
			if(OnKeyPress(&event, SDLK_l)) LoadGame(&player, npcs, &levelinfo, (char*)visited);*/
			if(OnKeyPress(&event, SDLK_f)) flashlight = flashlight ? 0 : 1;
			if(OnKeyPress(&event, SDLK_b)) player.battery += 50;
			if(OnKeyPress(&event, SDLK_SPACE)) player.shoot((int*)level);

			if(OnKeyPress(&event, SDLK_ESCAPE) || OnKeyPress(&event, SDLK_q) || event.type == SDL_QUIT)
			{
				mode = GameMode::Initial;
				break;
			}

			if(flashlight && !(player.battery)) flashlight = 0;
		}

		break;

		case GameMode::GameOver:
		for(int a = 0; a < 40; ++a)
		{
			generateNoise(noise, danger_level);
			noise_big = SPG_Transform(*noise, 0, 0, 4, 4, 0);
			assert(noise_big);
			SDL_BlitSurface(noise_big, 0, *worldview, 0);
			SDL_SoftStretch(*worldview, 0, screen, 0);
			SPG_Free(noise_big);
			SDL_PollEvent(&event);
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			SDL_Delay(40);
		}
		danger_level = 128;
		mode = GameMode::Initial;
		break;

		case GameMode::Quit:
		SDL_Quit();
		return 0;
		break;

		default:
		printf("Invalid game mode!\n");
	}
}
