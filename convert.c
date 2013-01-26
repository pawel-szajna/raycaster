#include <stdio.h>
#include "data.h"
#undef main

int main()
{
	FILE* map;
	FILE* info;
	LevelInfo li;
	int x;
	int dx, dy, dz;

	info = fopen("cinfo.txt", "r");
	fscanf(info, "%[^\n] %d %d %d %d %d %d %d %d", &li.name, &li.levelID, &li.textureSet, &li.textureSetSize, &li.npcTexCount, &li.npcCount, &li.itemCount, &li.pillarTexCount, &li.debrisTexCount);
	for(x = 0; x < li.pillarTexCount; ++x) fscanf(info, "%s", li.pillarTex[x]);
	for(x = 0; x < li.debrisTexCount; ++x) fscanf(info, "%s", li.debrisTex[x]);
	for(x = 0; x < li.npcTexCount; ++x) fscanf(info, "%s", li.npcTex[x]);

	map = fopen("mapa.dat", "wb");
	fwrite((char*)&li, sizeof(char), sizeof(LevelInfo)/sizeof(char), map);
	while(scanf("%d", &x) != EOF) fwrite((char*)&x, sizeof(char), sizeof(int) / sizeof(char), map);

	for(x = 0; x < li.npcCount; ++x)
	{
		fscanf(info, "%d %d %d", &dx, &dy, &dz);
		fwrite(&dx, sizeof(int), 1, map);
		fwrite(&dy, sizeof(int), 1, map);
		fwrite(&dz, sizeof(int), 1, map);
	}

	fclose(info);
	fclose(map);

	return 0;
}
