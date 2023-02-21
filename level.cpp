#include "level.hpp"

#include <cstdio>
#include <cmath>

#include "ai.hpp"

int BlockType(int* level, int x, int y)
{
	return level[levelSize * x + y] % 16;
}

void LoadLevel(int* level, LevelInfo* li, NPCs& npcs, const char* filename)
{
	int x;
	int dx, dy, dz;
	FILE* data;

	printf("Loading map %s...\n", filename);
	data = fopen(filename, "rb");
	assert(data);
	fread((char*)(li), sizeof(char), sizeof(LevelInfo) / sizeof(char), data);
	fread((char*)(level), sizeof(char), (levelSize * levelSize * sizeof(int)) / sizeof(char), data);
    npcs.clear();
	for(x = 0; x < li->npcCount; ++x)
	{
		fread(&dx, sizeof(int), 1, data);
		fread(&dy, sizeof(int), 1, data);
		fread(&dz, sizeof(int), 1, data);
		AddNPC(npcs, dx, dy, dz);
	}
	fclose(data);
}

const char* PlayerArrow(double angle)
{
	if(angle < 0.39) return "gfx/map32.bmp";
	if(angle < 1.18) return "gfx/map31.bmp";
	if(angle < 1.96) return "gfx/map30.bmp";
	if(angle < 2.75) return "gfx/map29.bmp";
	if(angle < 3.53) return "gfx/map28.bmp";
	if(angle < 4.31) return "gfx/map27.bmp";
	if(angle < 5.11) return "gfx/map34.bmp";
	if(angle < 5.89) return "gfx/map33.bmp";	
	return "gfx/map32.bmp";
}

int isitwall(int* level, char* visited, int x, int y)
{
	if(x < 0 || y < 0 || x >= levelSize || y >= levelSize) return 0;
	if(!visited[levelSize * x + y]) return 0;
	if(level[levelSize * x + y] % 16 == 1) return 1;
	return 0;
}

int walltype(int* level, char* visited, int x, int y)
{
	int a = 0;

	if(isitwall(level, visited, x - 1, y)) a += 1;
	if(isitwall(level, visited, x, y - 1)) a += 2;
	if(isitwall(level, visited, x + 1, y)) a += 4;
	if(isitwall(level, visited, x, y + 1)) a += 8;
	
	return 1 + a;
}

SDL_Surface* DrawMap(int* level, Player* player)
{
	SDL_Surface* map;
	SDL_Surface* tmp;
	SDL_Rect b1 = { 0, 0, 8, 8 }, b2 = { 0, 0, 8, 8 };
	int x, y;
	char filename[64];
	char* visited = player->currentLevel().visited;
	int posX = (int)player->posX;
	int posY = (int)player->posY;
	double dirX = player->dirX;
	double dirY = player->dirY;

	map = SDL_CreateRGBSurface(SDL_HWSURFACE, levelSize * 8, levelSize * 8, 32, 0, 0, 0, 0);
	assert(map);

/*	for(int i = 0; i < 15; ++i)
	{
		sprintf(filename, "gfx/map%02d.bmp", i);
		b[i] = SDL_LoadBMP(filename);
	}
*/
	for(x = 0; x < levelSize; ++x) for(y = 0; y < levelSize; ++y)
	{
		if(posX == x && posY == y)
		{
			tmp = SDL_LoadBMP(PlayerArrow(atan2(dirY, dirX) + 3.1415));
			assert(tmp);
		}
		else if(visited[levelSize * x + y])
		{
			switch((level[levelSize * x + y]) % 16)
			{
				case 1:
					sprintf(filename, "gfx/map%02d.bmp", walltype(level, visited, x, y));
					tmp = SDL_LoadBMP(filename);
					break;
				case 2:
					tmp = SDL_LoadBMP("gfx/map19.bmp");
					break;
				default:
					tmp = SDL_LoadBMP("gfx/map20.bmp");
			}
		}
		else tmp = SDL_LoadBMP("gfx/map20.bmp");
		assert(tmp);

		b2.y = x * 8;
		b2.x = y * 8;

		SDL_BlitSurface(tmp, &b1, map, &b2);
		SDL_FreeSurface(tmp);
	}

	/*for(int i = 0; i < 15; ++i) SDL_FreeSurface(b[i]);*/

	return map;
}