#include "ProceduralMapGeneration.h"

coord::coord(int x, int y, int dist, coord* prev) {
	this->x = x;
	this->y = y;
	this->dist = dist;
	this->prev = prev;
}

coord::coord(int x, int y) {
	this->x = x;
	this->y = y;
	dist = 0;
	prev = nullptr;
}

ProceduralMapGeneration::ProceduralMapGeneration(int maxRow, int maxCol, int maxWall, int mapLevel) {
	this->maxRow = maxRow;
	this->maxCol = maxCol;
	this->maxWall = maxWall;
	this->m_iMapLevel = mapLevel;
	this->wall = 0;

	path = new coord * [1000];
	map = new int* [maxRow];
	m_oreHealth = new int* [maxRow];
	for (int i = 0; i < maxRow; i++) {
		map[i] = new int[maxCol];
		m_oreHealth[i] = new int[maxCol](); // Value initialize to zeroes
	}

	for (int i = 0; i < maxRow; i++) {
		for (int j = 0; j < maxCol; j++) {
			if (i % 2 == 1 && j % 2 == 1)
				map[i][j] = 0;
			else
				map[i][j] = 100;
		}
	}


	int roomRows = (maxRow - 1) / 2; // 7
	int roomCols = (maxCol - 1) / 2; // 10
	bool** visited = new bool* [roomRows];
	for (int i = 0; i < roomRows; i++) {
		visited[i] = new bool[roomCols]();
	}

	const int DIR_COUNT = 4;
	int dRow[DIR_COUNT] = { 0,  0, -2,  2 };
	int dCol[DIR_COUNT] = { -2,  2,  0,  0 };

	std::stack<coord*> dfsStack;
	coord* startRoom = new coord(1, 1);
	dfsStack.push(startRoom);
	visited[0][0] = true;

	while (!dfsStack.empty()) {
		coord* cur = dfsStack.top();

		int dirs[DIR_COUNT] = { 0, 1, 2, 3 };
		for (int i = DIR_COUNT - 1; i > 0; i--) {
			int j = rand() % (i + 1);
			int tmp = dirs[i]; dirs[i] = dirs[j]; dirs[j] = tmp;
		}

		bool moved = false;
		for (int d = 0; d < DIR_COUNT; d++) {
			int ny = cur->y + dRow[dirs[d]];
			int nx = cur->x + dCol[dirs[d]];

			if (ny >= 1 && ny < maxRow - 1 && nx >= 1 && nx < maxCol - 1) {
				int ry = (ny - 1) / 2;
				int rx = (nx - 1) / 2;
				if (!visited[ry][rx]) {

					int wy = (cur->y + ny) / 2;
					int wx = (cur->x + nx) / 2;
					map[wy][wx] = 0;

					visited[ry][rx] = true;
					dfsStack.push(new coord(nx, ny));
					moved = true;
					break;
				}
			}
		}

		if (!moved)
			dfsStack.pop();
	}

	for (int i = 0; i < roomRows; i++) delete[] visited[i];
	delete[] visited;

	coord* entrance = new coord(1, 1);
	coord* exitRoom = new coord(maxCol - 2, maxRow - 2);
	int pathLen = findPathBFS(entrance, exitRoom, map, maxRow, maxCol, 0, path);

	for (int x = 0; x < 2; x++) {
		for (int i = 1; i < maxRow - 1; i++) {
			for (int j = 1; j < maxCol - 1; j++) {
				if (map[i][j] < 100) continue;

				int wallNeighbors = 0;
				if (map[i - 1][j] >= 100) wallNeighbors++;
				if (map[i + 1][j] >= 100) wallNeighbors++;
				if (map[i][j - 1] >= 100) wallNeighbors++;
				if (map[i][j + 1] >= 100) wallNeighbors++;

				if (wallNeighbors <= 1)
					map[i][j] = 0;
			}
		}
	}
	
	// Once the basic maze is finalized, seed the mineral nodes!
	GenerateMinerals();
}

ProceduralMapGeneration::~ProceduralMapGeneration()
{
	// Thoroughly delete dynamic 2D arrays to clean up memory
	for (int i = 0; i < maxRow; i++) {
		delete[] map[i];
		delete[] m_oreHealth[i];
	}
	delete[] map;
	delete[] m_oreHealth;
	delete[] path;
}

BOOL ProceduralMapGeneration::DamageOre(int r, int c, int iDamage)
{
	// Boundary safeguard
	if (r < 0 || r >= maxRow || c < 0 || c >= maxCol)
		return FALSE;

	// Identify active minerals (1 to 5)
	int tileVal = map[r][c];
	if (tileVal >= 1 && tileVal <= 5)
	{
		m_oreHealth[r][c] -= iDamage;
		if (m_oreHealth[r][c] <= 0)
		{
			m_oreHealth[r][c] = 0;
			map[r][c] = 0; // Successfully destroyed, returns to safe Floor space!
			return TRUE;
		}
	}
	return FALSE;
}


int ProceduralMapGeneration::distance(coord* c1, coord* c2) {
	int dx, dy;

	dx = abs(c1->x - c2->x);
	dy = abs(c1->y - c2->y);
	return dx + dy;
}

int ProceduralMapGeneration::findPathBFS(coord* start, coord* end, int** map, int maxRow, int maxCol, int wallCount, coord** path) {
	int** map2;
	std::queue<coord*> q;

	map2 = new int* [maxRow];

	for (int i = 0; i < maxRow; i++) {
		map2[i] = new int[maxCol];
	}

	for (int y = 0; y < maxRow; y++) {
		for (int x = 0; x < maxCol; x++) {
			map2[y][x] = map[y][x];
		}
	}

	q.push(start);

	while (!q.empty()) {
		coord* current = q.front();
		q.pop();
		map2[current->y][current->x] = 1; // Mark as visited

		if (distance(current, end) <= 1) {
			int i = 0;
			while (start->prev != NULL) {
				path[i] = current;
				current = current->prev;
				i++;
			}
			return i;
		}

		if (current->x + 1 < maxCol && map2[current->y][current->x + 1] == 0) { q.push(new coord(current->x + 1, current->y, current->dist + 1, current)); }
		if (current->x - 1 >= 0 && map2[current->y][current->x - 1] == 0) { q.push(new coord(current->x - 1, current->y, current->dist + 1, current)); }
		if (current->y + 1 < maxRow && map2[current->y + 1][current->x] == 0) { q.push(new coord(current->x, current->y + 1, current->dist + 1, current)); }
		if (current->y - 1 >= 0 && map2[current->y - 1][current->x] == 0) { q.push(new coord(current->x, current->y - 1, current->dist + 1, current)); }
	}

	return -1; // No path found
}

void ProceduralMapGeneration::displayMap() {
	for (int i = 0; i < maxRow; i++) {
		for (int j = 0; j < maxCol; j++) {
			if (map[i][j] >= 100);

		}
	}
}

// PERFORMANSLI VE GÜVENLİ MADEN YERLEŞTİRME ALGORİTMASI
void ProceduralMapGeneration::GenerateMinerals()
{
	// Maden seçkisini belirle (Level'e bağlı kaydırmalı pencere)
	// Maden Havuzu İndeksi: 0:Iron, 1:Amethyst, 2:Gold, 3:Ruby, 4:Obsidian
	// Harita Değerleri: 1'den 5'e kadar.
	int baseIdx = (m_iMapLevel - 1) % 5;
	int nextIdx = m_iMapLevel % 5;
	
	int oreA = baseIdx + 1; // Harita Değeri
	int oreB = nextIdx + 1;

	// Yerleştirilecek tahmini maden sayısı (Harita büyüklüğünün %3'ü civarı)
	int totalCells = maxRow * maxCol;
	int spawnTargets = totalCells / 30; 

	int placed = 0;
	int attempts = 0;
	int maxAttempts = spawnTargets * 10; // Sonsuz döngü önlemi

	while (placed < spawnTargets && attempts < maxAttempts)
	{
		attempts++;
		
		// Rastgele bir koordinat seç (Kenarlar hariç)
		int r = 1 + (rand() % (maxRow - 2));
		int c = 1 + (rand() % (maxCol - 2));

		// Sadece Boş Zemin (0) ise devam et
		if (map[r][c] != 0) continue;

		// --- DAR GEÇİT KONTROLÜ (KİLİTLEME ÖNLEME) ---
		// Üst ve Alt aynı anda kapalıysa, dikey geçişi kesmemek için koyma!
		bool wallUp = (map[r - 1][c] == 100 || map[r - 1][c] > 0 && map[r - 1][c] <= 5);
		bool wallDown = (map[r + 1][c] == 100 || map[r + 1][c] > 0 && map[r + 1][c] <= 5);
		if (wallUp && wallDown) continue;

		// Sol ve Sağ aynı anda kapalıysa, yatay geçişi kesmemek için koyma!
		bool wallLeft = (map[r][c - 1] == 100 || map[r][c - 1] > 0 && map[r][c - 1] <= 5);
		bool wallRight = (map[r][c + 1] == 100 || map[r][c + 1] > 0 && map[r][c + 1] <= 5);
		if (wallLeft && wallRight) continue;

		// Tüm testlerden geçti! Güvenli bir şekilde maden koyabiliriz.
		int selectedOre = (rand() % 2 == 0) ? oreA : oreB;
		map[r][c] = selectedOre;
		m_oreHealth[r][c] = selectedOre * 3; // HP maps to: 3, 6, 9, 12, 15
		placed++;
	}

	// --- BİR ADET MERDİVEN (EXIT) YERLEŞTİR ---
	bool stairsPlaced = false;
	int stairAttempts = 0;
	while (!stairsPlaced && stairAttempts < 1000)
	{
		stairAttempts++;
		int r = 1 + (rand() % (maxRow - 2));
		int c = 1 + (rand() % (maxCol - 2));

		// Sadece temiz zemin üzerine koyulabilir
		if (map[r][c] == 0)
		{
			map[r][c] = 6; // 6 = Stairs Tile
			stairsPlaced = true;
		}
	}
}

POINT ProceduralMapGeneration::FindStartingSpawnTile() const
{
	int centerR = maxRow / 2;
	int centerC = maxCol / 2;
	
	POINT ptSpawn = { centerC, centerR }; // Safe fallback coordinates
	bool spawnFound = false;

	for (int radius = 0; radius < centerR && !spawnFound; ++radius)
	{
		for (int r = centerR - radius; r <= centerR + radius && !spawnFound; ++r)
		{
			for (int c = centerC - radius; c <= centerC + radius && !spawnFound; ++c)
			{
				if (r >= 0 && r < maxRow && c >= 0 && c < maxCol)
				{
					if (map[r][c] == 0) // Safe, empty floor tile
					{
						ptSpawn.x = c;
						ptSpawn.y = r;
						spawnFound = true;
					}
				}
			}
		}
	}
	return ptSpawn;
}