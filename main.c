#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "sprig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "data.h"
#include "level.h"
#include "caster.h"
#include "player.h"
#include "ai.h"
#include "ui.h"
#include "generator.h"

#ifndef NDEBUG
#undef main
#endif

int main(int argc, char** argv)
{
	int level[LEVEL_SIZE][LEVEL_SIZE];
	char visited[LEVEL_SIZE][LEVEL_SIZE];

	int paused = 0;
	int flashlight = 1;
	int popup = 0;
	int popupmap = 0;
	int i, found;
	int danger_level;
	int mode;

	char texts[6][128];

	double nearest = 20;

	LevelInfo levelinfo;
	GameConfig config;

	Player player;
	NPCList* npcs = NULL;

	SDL_Surface* screen;
	SDL_Surface* worldview;
	SDL_Surface* map;
	SDL_Surface* shade;
	SDL_Surface* noise;
	SDL_Surface* noise_big;
	SDL_Surface* popupWindow;
	SDL_Surface* gun_hand;
	SDL_Surface* gun_shoot;

	SDL_Event event = { 0 };
	SDL_Rect r1 = { 0, 60, 0, 0 }, /* world view */
			 r2 = { 84, 210, 0, 0 }, /* world & noise target */
			 r4 = { 0, 0, 8 * LEVEL_SIZE, 8 * LEVEL_SIZE }, /* map target */
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

	LoadConfig(&config);
	LoadText((char*)texts);
	InitUI();
	
	screen = SDL_SetVideoMode(config.sWidth, config.sHeight, 32, SDL_HWSURFACE | (config.fullScreen ? SDL_FULLSCREEN : 0));
	worldview = SDL_CreateRGBSurface(SDL_HWSURFACE, wwWidth, wwHeight, 32, 0, 0, 0, 0);
	noise = SDL_CreateRGBSurface(SDL_HWSURFACE, 160, 90, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	shade = SDL_LoadBMP("gfx/shade.bmp");

	assert(screen);
	assert(worldview);
	assert(noise);
	assert(shade);

	SDL_ShowCursor(SDL_DISABLE);

	SDL_SetColorKey(shade, SDL_SRCCOLORKEY, 0x00ffffff);
	
	player.data.levels = NULL;
	mode = MODE_INIT;

	/* for map drawing */
	/* TODO: make this cleaner*/
	r4.x = config.sWidth / 2 - 256;
	r4.y = config.sHeight / 2 - 246;
	danger_level = 1;

	for(;;) switch(mode)
	{
		case MODE_INIT:
		SDL_WM_SetCaption(config.title, NULL);
		for(;;)
		{
			SDL_Surface* tmp;

			if(danger_level > 12)
			{
				GenerateNoiseLinear(noise, danger_level);
				danger_level -= 2;
			}
			else
			{
				GenerateNoise(noise, 12);
			}
			
			noise_big = SPG_Transform(noise, 0, 0, 4, 4, 0);
			assert(noise_big);

			SDL_FreeSurface(worldview);
			worldview = SDL_CreateRGBSurface(SDL_HWSURFACE, wwWidth, wwHeight, 32, 0, 0, 0, 0);
			
			tmp = TTF_RenderUTF8_Shaded(uiFontNotice, "Prece enter key", uiBG, uiFG);
			SDL_BlitSurface(tmp, 0, worldview, &r2);

			tmp = TTF_RenderUTF8_Shaded(uiFontTitle, "Techdemo", uiBG, uiFG);
			SDL_BlitSurface(tmp, 0, worldview, &r1);

			SDL_FreeSurface(tmp);
			SDL_BlitSurface(noise_big, 0, worldview, 0);
			SDL_SoftStretch(worldview, 0, screen, 0);
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			SDL_Delay(40);

			SDL_PollEvent(&event);

			if(OnKeyPress(&event, SDLK_ESCAPE) || OnKeyPress(&event, SDLK_q) || event.type == SDL_QUIT)
			{
				mode = MODE_QUIT;
				break;
			}

			if(OnKeyPress(&event, SDLK_RETURN)) break;
		}
		if(mode != MODE_QUIT) mode = MODE_MAIN_MENU;
		break;

		case MODE_MAIN_MENU:
		NewGame(&player, &config);
		mode = MODE_GAME;
		break;

		case MODE_GAME:
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
				npcs = NULL;
				sprintf(filename, "map/level%d.dat", player.level);
				LoadLevel((int*)level, &levelinfo, &npcs, filename);
				assert(npcs);
				generate_map((int*)level, (int)player.posX, (int)player.posY, 1);
				generate_npcs((int*)level, &npcs);
				InitAI((int*)level);
				InitUI();
				sprintf(filename, "%s: Level %d", config.title, player.level);
				SDL_WM_SetCaption(filename, NULL);
				SDL_FreeSurface(worldview);
				worldview = InitCaster((int*)level, &levelinfo);
				assert(worldview);
				player.data.current = player.data.levels;
				found = 0;

				while((player.data.current != NULL) && !found) /* check for this level state */
				{
					if(player.data.current->levelID == player.level) found = 1;
					else player.data.current = player.data.current->next;
				}

				if(!found) /* this level hasn't been visited yet, prepare its entry */
				{
					player.data.current = malloc(sizeof(PlayerLevel));
					assert(player.data.current);

					player.data.current->levelID = player.level;
					player.data.current->next = player.data.levels;
					player.data.current->npcs = malloc(sizeof(NPCList));
					assert(player.data.current->npcs);

					memcpy(player.data.current->npcs, npcs, sizeof(NPCList));
					assert(player.data.current->npcs);

					player.data.current->items = NULL;

					for(i = 0; i < LEVEL_SIZE * LEVEL_SIZE; ++i) player.data.current->visited[i] = 0;
					player.data.levels = player.data.current;
					if(npcs != NULL) free(npcs);
				}

				if(found) ResetAI(&npcs);

				player.data.current->items = AddItem(player.data.current->items, 27, 46, 0);
				player.data.current->items = AddItem(player.data.current->items, 27, 47, 1);
				player.data.current->items = AddItem(player.data.current->items, 27, 48, 3);

				player.reloadLevel = 0;
			}

			keys = SDL_GetKeyState(NULL);

			/* render frame */
			if(!paused)
			{
				CastFrame(worldview, (int*)level, &player, flashlight && player.flashlight && Blink(player.battery));
				if(player.revolver) SDL_BlitSurface(gun_hand, 0, worldview, &r9);
			}
			GenerateNoise(noise, danger_level);
			noise_big = SPG_Transform(noise, 0, 0, 4, 4, 0);
			assert(noise_big);
			SDL_BlitSurface(noise_big, 0, worldview, 0);
			SDL_SoftStretch(worldview, 0, screen, 0);
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
				HandleMovement(&player, keys, (int*)level, (char*)visited, frameTime);

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
				mode = MODE_GAMEOVER;
				break;
			}

			/* some functions */
			/* saving is disabled in techdemo */
			/*if(OnKeyPress(&event, SDLK_s)) SaveGame(&player, npcs, &levelinfo, (char*)visited);
			if(OnKeyPress(&event, SDLK_l)) LoadGame(&player, npcs, &levelinfo, (char*)visited);*/
			if(OnKeyPress(&event, SDLK_f)) flashlight = flashlight ? 0 : 1;
			if(OnKeyPress(&event, SDLK_b)) player.battery += 50;
			if(OnKeyPress(&event, SDLK_SPACE) && player.revolver) Shoot((int*)level, &player);

			if(OnKeyPress(&event, SDLK_ESCAPE) || OnKeyPress(&event, SDLK_q) || event.type == SDL_QUIT)
			{
				mode = MODE_INIT;
				break;
			}

			if(flashlight && !(player.battery)) flashlight = 0;
		}

		break;

		case MODE_GAMEOVER:
		for(int a = 0; a < 40; ++a)
		{
			GenerateNoise(noise, danger_level);
			noise_big = SPG_Transform(noise, 0, 0, 4, 4, 0);
			assert(noise_big);
			SDL_BlitSurface(noise_big, 0, worldview, 0);
			SDL_SoftStretch(worldview, 0, screen, 0);
			SPG_Free(noise_big);
			SDL_PollEvent(&event);
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			SDL_Delay(40);
		}
		danger_level = 128;
		mode = MODE_INIT;
		break;

		case MODE_QUIT:
		SDL_Quit();
		return 0;
		break;

		default:
		printf("Invalid game mode!\n");
	}
}
