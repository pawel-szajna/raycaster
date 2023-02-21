#include "caster.hpp"

#include "SDL/SDL.h"

#include "player.hpp"
#include "level.hpp"

#include <cmath>

#define SQR(x) ((x)*(x))

int mapX, mapY;
int stepX, stepY;
int hit, side;

int lineHeight;
int dStart, dEnd;

int numSprites;
int spriteScreenX;
int spriteHeight, spriteWidth;
int dStartX, dEndX;
int staticSprites;
int* spriteOrder;

int tex, texX, texY;
unsigned int r, g, b, d;

double camX, oldDir;
double rayPX, rayPY;
double rayDX, rayDY;
double sidedX, sidedY;
double deltadX, deltadY;
double wallDist, wallX;

double spriteX, spriteY;
double transX, transY;
double invDet;
double ZBuffer[wwWidth];
double* spriteDistance;

double fStart = 1.5, fEnd = 5.5;
double fLength = 4.0;

unsigned char datar, datag, datab;

Uint8* keys;
Uint32* pixels;
Uint32 buffer[wwWidth][wwHeight];
Uint32 textures[texCount][texWidth * texHeight];
Uint32 color;

Sprite* sprite;

void SpriteSort(int* order, double* distance, int count)
{
	int i, j;
	int gap = count;
	int swapped = 0;
	int x;
	double y;

	while(gap > 1 || swapped)
	{
		gap = (gap * 10) / 13;
		if (gap == 9 || gap == 10) gap = 11;
		if (gap < 1) gap = 1;
		swapped = 0;
		
		for(i = 0; i < count; ++i)
		{
			j = i + gap;
			if(j > count) continue;
			if(distance[i] < distance[j])
			{
				x = order[i];
				order[i] = order[j];
				order[j] = x;
				y = distance[i];
				distance[i] = distance[j];
				distance[j] = y;
				swapped = 1;
			}
		}
	}	
}

void LoadTexture(Uint32* memory, const char* filename)
{
	FILE* data;
	int y;

	assert(memory);
	data = fopen(filename, "rb");
	if(!data) printf("\n\n** CASTER ERROR **\nFile not found: %s!\n", filename);
	assert(data);
	for(y = 0; y < (texWidth * texHeight); ++y)
	{
		fread(&datar, sizeof(char), 1, data);
		fread(&datag, sizeof(char), 1, data);
		fread(&datab, sizeof(char), 1, data);
		memory[y] = 65536 * datar + 256 * datag + datab;
	}
	fclose(data);
}

SDL_Surface* InitCaster(int* level, LevelInfo* li)
{
	int i, x, y, u;
	char filename[128];
	SDL_Surface* wv;

	printf("Initializing caster... ");
	assert(level);
	assert(li);

	wv = SDL_CreateRGBSurface(SDL_HWSURFACE, wwWidth, wwHeight, 32, 0, 0, 0, 0);
	assert(wv);

	numSprites = 0;
	for(x = 0; x < levelSize; ++x) for(y = 0; y < levelSize; ++y) if(BlockType(level, x, y) == 2 || BlockType(level, x, y) == 5 || BlockType(level, x, y) == 6) ++numSprites;

	if(sprite != NULL) free(sprite);
	if(spriteOrder != NULL) free(spriteOrder);
	if(spriteDistance != NULL) free(spriteDistance);

	sprite = new Sprite[numSprites];
	spriteOrder = new int[numSprites];
	spriteDistance = new double[numSprites];

	staticSprites = numSprites;
	
	i = 0;

	for(x = 0; x < levelSize; ++x) for(y = 0; y < levelSize; ++y)
	{
		u = BlockType(level, x, y);
		if(u == 2 || u == 5 || u == 6)
		{
			sprite[i].x = 0.5 + x;
			sprite[i].y = 0.5 + y;
			sprite[i].texture = ((level[levelSize * x + y] / 16) % 16) + (u == 2 ? 16 : (u == 5 ? 32 : 0));
			++i;
		}
	}

	printf("OK\nLoading textures... ");

	for(x = 1; x <= (li->textureSetSize); ++x)
	{
		sprintf(filename, "gfx/set_%d/%d.raw", li->textureSet, x);
		LoadTexture(textures[x], filename);
	}

	for(x = 0; x < li->pillarTexCount; ++x)
	{
		sprintf(filename, "gfx/%s.raw", li->pillarTex[x]);
		LoadTexture(textures[16+x], filename);
	}

	for(x = 0; x < li->debrisTexCount; ++x)
	{
		sprintf(filename, "gfx/%s.raw", li->debrisTex[x]);
		LoadTexture(textures[32+x], filename);
	}

	for(x = 0; x < li->npcTexCount; ++x)
	{
		sprintf(filename, "gfx/%s_walk1.raw", li->npcTex[x]); LoadTexture(textures[48+x], filename);
		sprintf(filename, "gfx/%s_walk2.raw", li->npcTex[x]); LoadTexture(textures[56+x], filename);
		sprintf(filename, "gfx/%s_walk3.raw", li->npcTex[x]); LoadTexture(textures[64+x], filename);
		sprintf(filename, "gfx/%s_walk4.raw", li->npcTex[x]); LoadTexture(textures[72+x], filename);
		sprintf(filename, "gfx/%s_shoot.raw", li->npcTex[x]); LoadTexture(textures[80+x], filename);
		sprintf(filename, "gfx/%s_dead.raw", li->npcTex[x]); LoadTexture(textures[88+x], filename);	
	}

	for(x = 0; x < 4; ++x)
	{
		sprintf(filename, "gfx/items/%d.raw", x); LoadTexture(textures[96+x], filename);
	}

	LoadTexture(textures[0], "gfx/transparent.raw");
	LoadTexture(textures[10], "gfx/portal.raw");

	printf("OK\n");

	return wv;
}

void ResetDynamicSprites()
{
	numSprites = staticSprites;
	
	sprite = (Sprite*)realloc(sprite, numSprites * sizeof(Sprite)); assert(sprite);
	spriteOrder = (int*)realloc(spriteOrder, numSprites * sizeof(int)); assert(spriteOrder);
	spriteDistance = (double*)realloc(spriteDistance, numSprites * sizeof(double)); assert(spriteDistance);
}

void AddDynamicSprite(double x, double y, int texture)
{
	++numSprites;
	
	sprite = (Sprite*)realloc(sprite, numSprites * sizeof(Sprite)); assert(sprite);
	spriteOrder = (int*)realloc(spriteOrder, numSprites * sizeof(int)); assert(spriteOrder);
	spriteDistance = (double*)realloc(spriteDistance, numSprites * sizeof(double)); assert(spriteDistance);

	sprite[numSprites - 1].x = x;
	sprite[numSprites - 1].y = y;
	sprite[numSprites - 1].texture = texture;
}

void CastFrame(SDL_Surface* worldview, int* worldMap, Player* player, int flashlight)
{
	int i, x, y;

	fStart = flashlight ? 2.5 : 0.5;
	fEnd = flashlight ? 5.5 : 3.1;
	fLength = fEnd - fStart;
    
    const auto& position = player->getPosition() ;

	for(x = 0; x < wwWidth; ++x)
	{
		camX = 2 * x / (double)wwWidth - 1; /* wspolrzedna x na plaszczyznie na ktorej rzutowany jest obraz */
		rayPX = position.x;
		rayPY = position.y;
		rayDX = position.dirX + position.planeX * camX;
		rayDY = position.dirY + position.planeY * camX;
		mapX = (int)rayPX;
		mapY = (int)rayPY;
		deltadX = sqrt(1 + SQR(rayDY) / SQR(rayDX));
		deltadY = sqrt(1 + SQR(rayDX) / SQR(rayDY));
		hit = 0;
		stepX = (rayDX < 0) ? -1 : 1;
		stepY = (rayDY < 0) ? -1 : 1;
		sidedX = (stepX < 0) ? (rayPX - mapX) * deltadX : (mapX + 1.0 - rayPX) * deltadX;
		sidedY = (stepY < 0) ? (rayPY - mapY) * deltadY : (mapY + 1.0 - rayPY) * deltadY;

		/* lecimy promyczkiem swiatla dopoki nie trafimy na sciane albo nie wylecimy z planszy */
		while(!hit || mapX < 0 || mapY < 0 || mapX >= levelSize || mapY >= levelSize)
		{
			if(sidedX < sidedY)
			{
				sidedX += deltadX;
				mapX += stepX;
				side = 0;
			}
			else
			{
				sidedY += deltadY;
				mapY += stepY;
				side = 1;
			}

			if((worldMap[mapX * levelSize + mapY] % 16) == 1) hit = 1;
		}

		if(!hit) continue;

		wallDist = side ? fabs((mapY - rayPY + (1 - stepY) / 2) / rayDY) : fabs((mapX - rayPX + (1 - stepX) / 2) / rayDX);
		lineHeight = abs((int)(wwHeight / wallDist));
		dStart = -lineHeight / 2 + wwHeight / 2;
		dEnd = lineHeight / 2 + wwHeight / 2;
		if(dStart < 0) dStart = 0;
		if(dEnd >= wwHeight) dEnd = wwHeight - 1;

		tex = (worldMap[mapX * levelSize + mapY] >> 4);
		if(!side && position.x < mapX) tex >>= 8;
		if(side)
		{
			if(position.y < mapY) tex >>= 12;
			else tex >>= 4;
		}
		tex %= 16;

		wallX = side ? rayPX + ((mapY - rayPY + (1 - stepY) / 2) / rayDY) * rayDX : rayPY + ((mapX - rayPX + (1 - stepX) / 2) / rayDX) * rayDY;
		wallX -= floor(wallX);
		texX = (int)(wallX * (double)texWidth);
		if((!side && rayDX > 0) || (side && rayDY < 0)) texX = texWidth - texX - 1;

		for(y = dStart; y < dEnd; ++y)
		{
			d = y * 256 - wwHeight * 128 + lineHeight * 128;
			texY = ((d * texHeight) / lineHeight) / 256;
			color = textures[tex][texHeight * texY + texX];
			if(wallDist > fStart && wallDist < fEnd)
			{
				r = (color >> 16) % 256;
				g = (color >> 8) % 256;
				b = color % 256;
				r *= (1 - ((wallDist - fStart) / (fLength - 0.1)));
				g *= (1 - ((wallDist - fStart) / (fLength)));
				b *= (1 - ((wallDist - fStart) / (fLength - 0.05)));
				if(r > 255) r = 0;
				if(b > 255) b = 0;
				color = 65536 * r + 256 * g + b;
			}
			if(wallDist < fEnd) buffer[x][y] = color;
		}

		ZBuffer[x] = wallDist;
	}

	for(i = 0; i < numSprites; ++i)
	{
		spriteOrder[i] = i;
		spriteDistance[i] = (SQR(position.x - sprite[i].x) + SQR(position.y - sprite[i].y));
	}
	
	SpriteSort(spriteOrder, spriteDistance, numSprites);

	for(i = 0; i < numSprites; ++i)
	{
		if (spriteOrder[i] < 0 || spriteOrder[i] >= numSprites)
		{
			continue;
		}

		spriteX = sprite[spriteOrder[i]].x - position.x;
		spriteY = sprite[spriteOrder[i]].y - position.y;
		invDet = 1.0 / (position.planeX * position.dirY - position.dirX * position.planeY);

		transX = invDet * (position.dirY * spriteX - position.dirX * spriteY);
		transY = invDet * (-position.planeY * spriteX + position.planeX * spriteY);

		spriteScreenX = (int)((wwWidth / 2) * (1 + transX / transY));

		spriteHeight = abs((int)(wwHeight / transY));
		dStart = -spriteHeight / 2 + wwHeight / 2;
		dEnd = spriteHeight / 2 + wwHeight / 2;
		if(dStart < 0) dStart = 0;
		if(dEnd >= wwHeight) dEnd = wwHeight - 1;
		spriteWidth = abs((int)(wwHeight / transY));
		dStartX = -spriteWidth / 2 + spriteScreenX;
		dEndX = spriteWidth / 2 + spriteScreenX;
		if(dStartX < 0) dStartX = 0;
		if(dEndX >= wwWidth) dEndX = wwWidth - 1;

		for(x = dStartX; x < dEndX; ++x)
		{
			texX = (int)(256 * (x - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
			if(transY > 0 && x > 0 && x < wwWidth && transY < ZBuffer[x]) for(y = dStart; y < dEnd; ++y)
			{
				d = y * 256 - wwHeight * 128 + spriteHeight * 128;
				texY = ((d * texHeight) / spriteHeight) / 256;
				color = textures[sprite[spriteOrder[i]].texture][texWidth * texY + texX];
				if(color != 0x00FFFFFF)
				{
					if(transY > fStart && transY < fEnd)
					{
						r = (color >> 16);
						g = (color >> 8) % 256;
						b = color % 256;
						r *= (1 - ((transY - fStart) / (fLength - 0.1)));
						g *= (1 - ((transY - fStart) / (fLength)));
						b *= (1 - ((transY - fStart) / (fLength - 0.05)));
						if(r > 255) r = 0;
						if(b > 255) b = 0;
						color = 65536 * r + 256 * g + b;
					}
					if(transY < fEnd) buffer[x][y] = color;
				}
			}
		}
	}

	pixels = (Uint32*)(worldview->pixels);
	assert(pixels);

	for(y = 0; y < wwHeight; ++y)
	{
		for(x = 0; x < wwWidth; ++x)
		{
			*pixels = buffer[x][y];
			++pixels;
		}
		pixels += worldview->pitch / 4;
		pixels -= wwWidth;
	}

	for(x = 0; x < wwWidth; ++x) for(y = wwHeight/3; y < 2*wwHeight/3; ++y) buffer[x][y] = 0;
	for(y = 0; y <= wwHeight / (flashlight ? 2.45 : 3); ++y)
	{
		/*color = 40 * SQR((double)y / (wwHeight / 3)) - 80 * ((double)y / (wwHeight / 3)) + 40;*/
		color = (flashlight ? 40 : 30) - (flashlight ? 98 : 90) * y / wwHeight;
		color = 65536 * color + 256 * color + color;
		for(x = 0; x < wwWidth; ++x)
		{
			buffer[x][y] = color;
			buffer[x][wwHeight - y - 1] = color;
		}
	}
}

void GenerateNoise(SDL_Surface* noise, int amount)
{
	int x, y, buf;

	if(amount < 1) amount = 1;

	assert(noise);
	pixels = (Uint32*)(noise->pixels);
	for(y = 0; y < 90; ++y)
	{
		for(x = 0; x < 160; ++x)
		{
			buf = rand() % 256;
			buf = (buf << 16) + (buf << 8) + (buf);
			if(amount <= 112) buf+= (rand() % (256 * amount / 112)) << 24;
			else if(amount == 128) buf+= 0xff000000;
			else buf+= ((rand() % (2048 - 16 * amount)) + 16 * amount - 1792) << 24;
			*pixels = buf;
			++pixels;
		}
		pixels += noise->pitch / 4;
		pixels -= 160;
	}
}

void GenerateNoiseLinear(SDL_Surface* noise, int amount)
{
	int x, y, buf;

	if(amount < 1) amount = 1;

	assert(noise);
	pixels = (Uint32*)(noise->pixels);
	for(y = 0; y < 90; ++y)
	{
		for(x = 0; x < 160; ++x)
		{
			buf = rand() % 256;
			buf = (buf << 16) + (buf << 8) + (buf);
			if(amount <= 64) buf+= (rand() % (4 * amount)) << 24;
			else if(amount == 128) buf+= 0xff000000;
			else buf+= ((rand() % (512 - 4 * amount)) + 4 * amount - 256) << 24;
			*pixels = buf;
			++pixels;
		}
		pixels += noise->pitch / 4;
		pixels -= 160;
	}
}
