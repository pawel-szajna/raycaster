#include "generator.hpp"

void clear_level(int* level)
{
	for(int i = 0; i < levelSize; ++i)
		for(int j = 0; j < levelSize; ++j)
			level[levelSize * i + j] = 1 + (1 << 4) + (1 << 8) + (1 << 12) + (1 << 16) + (1 << 20) + (1 << 24);
}

void drawmap(int* level, int pX, int pY)
{
	for(int i = 0; i < levelSize; ++i)
	{
		for(int j = 0; j < levelSize; ++j)
		{
			if(pX == i && pY == j) printf("@");
			else printf("%c", level[levelSize * i + j] ? '#' : ' ');
		}
		printf("\n");
	}
}

void wall_fix(int* level)
{
	int tex1, tex2, tex3, tex4;

	for(int i = 0; i < 64; ++i) for(int j = 0; j < 64; ++j) if(LEV(i,j)%16 == 1)
	{
		if((i == 0 && j ==0) || (i == 0 && j == 63) || (i == 63 && j == 0) || (i == 63 && j == 63)) continue;

		tex1 = tex2 = tex3 = tex4 = 1;

		/* dolna sciana */
		if(i != 63 && j != 0 && j != 63)
		{
			if(LEV(i+1,j-1)%16 == 1 || LEV(i,j-1)%16 != 1)
			{
				if(LEV(i+1,j+1)%16 == 1 || LEV(i,j+1)%16 != 1) tex1 = 4;
				else tex1 = 5;
			}
			else if(LEV(i+1,j+1)%16 == 1 || LEV(i,j+1)%16 != 1) tex1 = 3;
		}

		/* gorna sciana */
		if(i != 0 && j != 0 && j != 63)
		{
			if(LEV(i-1,j-1)%16 == 1 || LEV(i,j-1)%16 != 1)
			{
				if(LEV(i-1,j+1)%16 == 1 || LEV(i,j+1)%16 != 1) tex3 = 4;
				else tex3 = 3;
			}
			else if(LEV(i-1,j+1)%16 == 1 || LEV(i,j+1)%16 != 1) tex3 = 5;
		}

		/* prawa sciana */
		if(j != 63 && i != 0 && i != 63)
		{
			if(LEV(i-1,j+1)%16 == 1 || LEV(i-1,j)%16 != 1)
			{
				if(LEV(i+1,j+1)%16 == 1 || LEV(i+1,j)%16 != 1) tex2 = 4;
				else tex2 = 3;
			}
			else if(LEV(i+1,j+1)%16 == 1 || LEV(i+1,j)%16 != 1) tex2 = 5;
		}

		/* lewa ściana */
		if(j != 0 && i != 0 && i != 63)
		{
			if(LEV(i-1,j-1)%16 == 1 || LEV(i-1,j)%16 != 1)
			{
				if(LEV(i+1,j-1)%16 == 1 || LEV(i+1,j)%16 != 1) tex4 = 4;
				else tex4 = 5;
			}
			else if(LEV(i+1,j-1)%16 == 1 || LEV(i+1,j)%16 != 1) tex4 = 3;
		}
		
		tex1 <<= 4;
		tex2 <<= 8;
		tex3 <<= 12;
		tex4 <<= 16;

		LEV(i,j) = 1 + tex1 + tex2 + tex3 + tex4;
	}
}

void generate_npcs(int* level, NPCList** npcs)
{
	int dx, dy, i = levelSize / 4;

	while(i)
	{
		dx = rand() % levelSize;
		dy = rand() % levelSize;
		if(!LEV(dx,dy))
		{
			printf("Spawning headless at %d:%d (block = %d)\n", dx, dy, LEV(dx, dy));
			--i;
			*npcs = AddNPC(*npcs, dx, dy, 0);
		}
	}
}

void bonus_room(int* level)
{
	int x;

	for(x = 43; x <= 50; ++x) LEV(27, x) = 0;
	for(x = 46; x <= 48; ++x) LEV(26, x) = LEV(28, x) = 2;
	for(x = 45; x <= 49; ++x) LEV(25, x) = LEV(29, x) = 1;
	LEV(26, 45) = LEV(28, 45) = LEV(26, 49) = LEV(28, 49) = LEV(26, 42) = LEV(26, 43) = LEV(26, 44) = LEV(28, 42) = LEV(28, 43) = LEV(28, 44) = LEV(27, 42) = 1;
}

void generate_map(int* level, int pX, int pY, int bonusroom)
{
	printf("Generating level structure... ");
	clear_level(level);
	depth_first(level, pX, pY, -1, -1, -1, -1, -1);
	for(int i = 0; i < 12; ++i) random_room(level);
	for(int i = 0; i < 4; ++i) corridor(level);
	LEV(pX, pY) = 0;
	if(bonusroom) bonus_room(level);
	wall_fix(level);
	level[levelSize * levelSize - 1] = 2;
	/*drawmap(level, pX, pY);*/
	printf("OK\n");
}

void depth_first(int* level, int a, int b, int c, int d, int e, int f, int g)
{
	int sasiedzi[4][4], ilosc_sasiadow, kolejnosc[4];
	if(!level[levelSize * a + b]) return;
	
	if(g > 0)
	{
		level[levelSize * a + b] = 1;
		return;
	}

	if(c>-1) level[levelSize * c + d] = 0;
	
	level[levelSize * a + b] = 0;

	ilosc_sasiadow = 0;
	
	if(a > 1)
	{
		sasiedzi[ilosc_sasiadow][0] = a - 2;
		sasiedzi[ilosc_sasiadow][1] = b;
		sasiedzi[ilosc_sasiadow][2] = a - 1;
		sasiedzi[ilosc_sasiadow][3] = b;
		++ilosc_sasiadow;
	}

	if(b > 1)
	{
		sasiedzi[ilosc_sasiadow][0] = a;
		sasiedzi[ilosc_sasiadow][1] = b - 2;
		sasiedzi[ilosc_sasiadow][2] = a;
		sasiedzi[ilosc_sasiadow][3] = b - 1;
		++ilosc_sasiadow;
	} 

	if(a < levelSize - 3)
	{
		sasiedzi[ilosc_sasiadow][0] = a + 2;
		sasiedzi[ilosc_sasiadow][1] = b;
		sasiedzi[ilosc_sasiadow][2] = a + 1;
		sasiedzi[ilosc_sasiadow][3] = b;
		++ilosc_sasiadow;
	} 

	if(b < levelSize - 3)
	{
		sasiedzi[ilosc_sasiadow][0] = a;
		sasiedzi[ilosc_sasiadow][1] = b + 2;
		sasiedzi[ilosc_sasiadow][2] = a;
		sasiedzi[ilosc_sasiadow][3] = b + 1;
		++ilosc_sasiadow;
	}

	kolejnosc[0] = 0;
	kolejnosc[1] = 1;
	kolejnosc[2] = 2;
	kolejnosc[3] = 3;

	for(int i = ilosc_sasiadow - 1; i >= 1; --i)
	{
		if(rand()%2)
		{
			int tmp = kolejnosc[i];
			kolejnosc[i] = kolejnosc[0];
			kolejnosc[0] = tmp;
		}
	}

	for(int k = 0; k < ilosc_sasiadow; ++k)
	{
		depth_first(level, sasiedzi[kolejnosc[k]][0], sasiedzi[kolejnosc[k]][1], sasiedzi[kolejnosc[k]][2], sasiedzi[kolejnosc[k]][3], a, b, (ilosc_sasiadow >= 3) ? (rand() % 6) - 4 : -1);
	}
}

void random_room(int* level)
{
	int x, y;
	int width = (rand() % 2) + 4;
	int height = (rand() % 2) + 4;
	int rx = (rand() % (levelSize - width - 3)) + 1;
	int ry = (rand() % (levelSize - height - 3)) + 1;

	width += rx;
	height += ry;

	for(x = rx; x < width; ++x) for(y = ry; y < height; ++y) LEV(x,y) = 0;
}

void corridor(int* level)
{
	int orientacja = rand() % 2;
	if(orientacja)
	{
		int dlugosc = RND(4, levelSize / 5);
		int x1 = RND(3, levelSize - dlugosc - 3);
		int y1 = RND(2, levelSize - 6);
		int x2 = x1 + dlugosc;
		int y2 = y1 + 6;
		for(int x = x1; x < x2; ++x) for(int y = y1; y < y2; ++y) LEV(x, y) = 0;
		for(int x = x1 + 1; x < x2; x += 2) { LEV(x, y1 + 1) = 1; LEV(x, y2 - 2) = 1; }
	}
	else
	{
		int dlugosc = RND(4, levelSize / 5);
		int x1 = RND(3, levelSize - dlugosc - 3);
		int y1 = RND(2, levelSize - 6);
		int x2 = x1 + dlugosc;
		int y2 = y1 + 6;
		for(int x = x1; x < x2; ++x) for(int y = y1; y < y2; ++y) LEV(y, x) = 0;
		for(int x = x1 + 1; x < x2; x += 2) { LEV(y1 + 1, x) = 1; LEV(y2 - 2, x) = 1; }	
	}
}
