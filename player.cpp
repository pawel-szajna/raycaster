#include "player.hpp"
#include "level.hpp"
#include "ai.hpp"

#include <cstdio>
#include <cstring>
#include <cmath>

#define CANENTER(x) (!((x)==1 || (x)==2))

void Shoot(int* level, Player* player)
{
	double bulletX = player->posX;
	double bulletY = player->posY;

	for(int i = 0; i < 100; ++i)
	{

		if((!CANENTER(level[(int)bulletX * levelSize + (int)bulletY] % 16)) || bulletX < 0 || bulletY < 0 || bulletX > levelSize || bulletY > levelSize) break;
		if(KillNPC(bulletX, bulletY, player->currentLevel().npcs)) break;

		bulletX += player->dirX * 2;
		bulletY += player->dirY * 2;
	}
}

void LoadConfig(GameConfig* config)
{
	FILE* cfile;

	assert(config);
	printf("Loading configuration file... ");
	cfile = fopen("ray.cfg", "r");
	assert(cfile);
	fscanf(cfile, "%d %d %d %d %d %d %d %[^\n]", &config->sWidth, &config->sHeight, &config->fullScreen, &config->level, &config->pX, &config->pY, &config->dir, config->title);
	fclose(cfile);
	printf("OK\n");
}


void LoadText(char* texts)
{
	FILE* txt;
	char text[128];

	printf("Loading texts... ");
	txt = fopen("txt/eng.txt", "r");
	assert(txt);
	for(int i = 0; i < 6; ++i)
	{
		fgets(text, sizeof(text), txt); /* fscanf nie przechodzil do nowej linii, trzeba tak ;_; */
		strcpy(&texts[128 * i], text); /* bier go */
		texts[128 * i + strlen(text) - 1] = '\0'; /* debil zostawia znak nowej linii w stringu */
	}
	fclose(txt);
	printf("OK\n");
}

void NewGame(Player* player, GameConfig* config)
{
	player->posX = config->pX + 0.5;
	player->posY = config->pY + 0.5;
	player->dirX = - cos(config->dir * 0.7854);
	player->dirY = - sin(config->dir * 0.7854);
	player->planeX = - 0.77 * sin(config->dir * 0.7854);
	player->planeY = 0.77 * cos(config->dir * 0.7854);
	player->levelId = config->level;
	player->reloadLevel = 1;

	player->revolver = 0;
	player->bullets = 0;
	player->flashlight = 0;
	player->battery = 0;
}

void MarkVisitedSub(char* visited, int x, int y)
{
	if(x>=0 && y>=0 && x < levelSize && y < levelSize) visited[levelSize * x + y] = 1;
}

void MarkVisited(Player* player, int x, int y)
{
	char* visited = player->currentLevel().visited;

	if(visited[levelSize * x + y] == 2) return;
	visited[levelSize * x + y] = 2;
	MarkVisitedSub(visited, x - 1, y - 1);
	MarkVisitedSub(visited, x - 1, y);
	MarkVisitedSub(visited, x - 1, y + 1);
	MarkVisitedSub(visited, x, y + 1);
	MarkVisitedSub(visited, x, y - 1);
	MarkVisitedSub(visited, x + 1, y + 1);
	MarkVisitedSub(visited, x + 1, y);
	MarkVisitedSub(visited, x + 1, y - 1);
	MarkVisitedSub(visited, x, y - 2);
	MarkVisitedSub(visited, x, y + 2);
	MarkVisitedSub(visited, x - 2, y);
	MarkVisitedSub(visited, x + 2, y);
}

void HandleMovement(Player* player, Uint8* keys, int* level, char* visited, double frameTime)
{
	double mSpeed = frameTime * 1.6 * speedFactor; /* pola/sekunde */
	double rSpeed = frameTime * 1.2 * speedFactor; /* radiany/sekunde */
	double oldDir;
	int collision, a;

	assert(player);

	if(keys[SDLK_UP])
	{
		collision = level[(int)(player->posX + player->dirX * mSpeed * 3) * levelSize + (int)player->posY] % 16;
		if(CANENTER(collision)) player->posX += player->dirX * mSpeed;
		collision = level[(int)player->posX * levelSize + (int)(player->posY + player->dirY * mSpeed * 3)] % 16;
		if(CANENTER(collision)) player->posY += player->dirY * mSpeed;	
		MarkVisited(player, (int)(player->posX), (int)(player->posY));
	}

	if(keys[SDLK_DOWN])
	{
		collision = level[(int)(player->posX - player->dirX * mSpeed * 3) * levelSize + (int)player->posY] % 16;
		if(CANENTER(collision)) player->posX -= player->dirX * mSpeed;
		collision = level[(int)player->posX * levelSize + (int)(player->posY - player->dirY * mSpeed * 3)] % 16;
		if(CANENTER(collision)) player->posY -= player->dirY * mSpeed;		
		MarkVisited(player, (int)(player->posX), (int)(player->posY));
	}

	if(keys[SDLK_z])
	{
		collision = level[(int)(player->posX - 3 * mSpeed * sin(atan2(player->dirY, player->dirX))) * levelSize + (int)player->posY] % 16;
		if(CANENTER(collision)) player->posX -= mSpeed * sin(atan2(player->dirY, player->dirX));
		collision = level[(int)player->posX * levelSize + (int)(player->posY + 3 * mSpeed * cos(atan2(player->dirY, player->dirX)))] % 16;
		if(CANENTER(collision)) player->posY += mSpeed * cos(atan2(player->dirY, player->dirX));
		MarkVisited(player, (int)(player->posX), (int)(player->posY));
	}

	if(keys[SDLK_x])
	{
		collision = level[(int)(player->posX + 3 * mSpeed * sin(atan2(player->dirY, player->dirX))) * levelSize + (int)player->posY] % 16;
		if(CANENTER(collision)) player->posX += mSpeed * sin(atan2(player->dirY, player->dirX));
		collision = level[(int)player->posX * levelSize + (int)(player->posY - 3 * mSpeed * cos(atan2(player->dirY, player->dirX)))] % 16;
		if(CANENTER(collision)) player->posY -= mSpeed * cos(atan2(player->dirY, player->dirX));
		MarkVisited(player, (int)(player->posX), (int)(player->posY));
	}

	if(keys[SDLK_LEFT])
	{
		oldDir = player->dirX;
		player->dirX = player->dirX * cos(rSpeed) - player->dirY * sin(rSpeed);
		player->dirY = oldDir * sin(rSpeed) + player->dirY * cos(rSpeed);

		oldDir = player->planeX;
		player->planeX = player->planeX * cos(rSpeed) - player->planeY * sin(rSpeed);
		player->planeY = oldDir * sin(rSpeed) + player->planeY * cos(rSpeed);
	}

	if(keys[SDLK_RIGHT])
	{
		oldDir = player->dirX;
		player->dirX = player->dirX * cos(-rSpeed) - player->dirY * sin(-rSpeed);
		player->dirY = oldDir * sin(-rSpeed) + player->dirY * cos(-rSpeed);

		oldDir = player->planeX;
		player->planeX = player->planeX * cos(-rSpeed) - player->planeY * sin(-rSpeed);
		player->planeY = oldDir * sin(-rSpeed) + player->planeY * cos(-rSpeed);
	}

	if(level[(int)player->posX * levelSize + (int)player->posY] % 16 == 6)
	{
		a = level[(int)player->posX * levelSize + (int)player->posY];

		player->levelId = (a >> 8) % 16;
		player->posX = ((a >> 12) % 256) + 0.5;
		player->posY = ((a >> 20) % 256) + 0.5;
		player->reloadLevel = 1;
	}
}

int OnKeyPress(SDL_Event* event, int key)
{
	if(event->type == SDL_KEYDOWN && event->key.keysym.sym == key)
	{
		event->type = 0;
		return 1;
	}
	return 0;
}

int Blink(int battery)
{
	if(!battery) return 0;
	if(battery > 45) return 1;

	if(battery < 10) return !(rand() % (10 - battery));
	return (rand() % battery);
}