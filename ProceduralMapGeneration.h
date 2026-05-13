#pragma once

#include <windows.h>
#include <stack>
#include <queue>

class coord {
public:
	int x;
	int y;
	int dist;
	coord* prev;

public:
	coord(int x, int y, int dist, coord* prev);
	coord(int x, int y);
};

class ProceduralMapGeneration
{
private:
	int maxRow, maxCol;
	int maxWall, wall;
	int** map;
	int** m_oreHealth; // Health points for each mineral cell
	coord** path;
	int m_iMapLevel; // Added level tracker

	void GenerateMinerals(); // Internal procedure

public:
	BOOL DamageOre(int r, int c, int iDamage); // Deduct health and destroy ore if HP <= 0

public:
	ProceduralMapGeneration(int maxRow, int maxCol, int maxWall, int mapLevel = 1);
    ~ProceduralMapGeneration();
	int distance(coord* c1, coord* c2);
	int findPathBFS(coord* start, coord* end, int** map, int maxRow, int maxCol, int wallCount, coord** path);
	void displayMap();
	int GetLevel() const { return m_iMapLevel; }
	POINT FindStartingSpawnTile() const;

	// Getters for rendering
	int GetRows() const { return maxRow; }
	int GetCols() const { return maxCol; }
	int GetTile(int r, int c) const {
		if (r >= 0 && r < maxRow && c >= 0 && c < maxCol) return map[r][c];
		return 100; // Out of bounds as wall
	}
};

