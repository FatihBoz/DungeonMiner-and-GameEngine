//-----------------------------------------------------------------
// Dungeon Miner Application
// C++ Source - DungeonMiner.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "DungeonMiner.h"
#include <cmath>
#include <cstdlib>
//#include <mmsystem.h>

#pragma comment(lib, "Msimg32.lib")

#define TILE_SIZE 64



//-----------------------------------------------------------------
// Game Engine Functions
//-----------------------------------------------------------------
BOOL GameInitialize(HINSTANCE hInstance)
{
  // Create the game engine
  _pGame = new GameEngine(hInstance, TEXT("Dungeon Miner"),
    TEXT("Dungeon Miner"), IDI_ROIDS, IDI_ROIDS_SM, 1280, 720);
  if (_pGame == NULL)
    return FALSE;

  // Set the frame rate
  _pGame->SetFrameRate(30);

  // Store the instance handle
  _hInstance = hInstance;

  return TRUE;
}

std::vector<FloatingText> _vFloatingTexts;

void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color)
{
  DebugSetPhase(TEXT("AddFloatingText"));

  FloatingText ft;
  ft.x = x;
  ft.y = y;
  ft.lifetime = 20; // Lives for 20 render cycles
  ft.color = color;
  lstrcpyn(ft.text, szText, 32);
  _vFloatingTexts.push_back(ft);
  DebugLogFormat(TEXT("floatingText count=%u text=%s pos=(%d,%d)"),
    (unsigned int)_vFloatingTexts.size(), ft.text, x, y);
}

void ClearEnemies()
{
  DebugLogFormat(TEXT("ClearEnemies count=%u"), (unsigned int)_vEnemies.size());

  for (size_t i = 0; i < _vEnemies.size(); i++)
  {
    if (_vEnemies[i] != NULL)
    {
      if (_pGame == NULL || !_pGame->RemoveSprite(_vEnemies[i]))
        _vEnemies[i]->Kill();
    }
  }

  _vEnemies.clear();
  _vMapCollisionIgnoredSprites.clear();
}

void UntrackEnemy(Sprite* pSprite)
{
  DebugLogFormat(TEXT("UntrackEnemy sprite=0x%p"), pSprite);

  for (std::vector<Enemy*>::iterator it = _vEnemies.begin(); it != _vEnemies.end(); )
  {
    if (*it == pSprite)
      it = _vEnemies.erase(it);
    else
      ++it;
  }

  for (std::vector<Sprite*>::iterator it = _vMapCollisionIgnoredSprites.begin(); it != _vMapCollisionIgnoredSprites.end(); )
  {
    if (*it == pSprite)
      it = _vMapCollisionIgnoredSprites.erase(it);
    else
      ++it;
  }
}

Enemy* GetTrackedEnemy(Sprite* pSprite)
{
  for (size_t i = 0; i < _vEnemies.size(); i++)
  {
    if (_vEnemies[i] == pSprite)
      return _vEnemies[i];
  }

  return NULL;
}

void IgnoreMapCollision(Sprite* pSprite)
{
  if (pSprite != NULL)
  {
    _vMapCollisionIgnoredSprites.push_back(pSprite);
    DebugLogFormat(TEXT("IgnoreMapCollision sprite=0x%p ignoredCount=%u"),
      pSprite, (unsigned int)_vMapCollisionIgnoredSprites.size());
  }
}

BOOL IsMapCollisionIgnored(Sprite* pSprite)
{
  for (size_t i = 0; i < _vMapCollisionIgnoredSprites.size(); i++)
  {
    if (_vMapCollisionIgnoredSprites[i] == pSprite)
      return TRUE;
  }

  return FALSE;
}

BOOL IsTankGolemPathTileBlocked(int row, int col, void* pContext)
{
  ProceduralMapGeneration* pMap = (ProceduralMapGeneration*)pContext;
  if (pMap == NULL)
    return TRUE;

  return pMap->GetTile(row, col) != 0;
}

static BOOL IsOpenFloorTile(int row, int col)
{
  if (_pMap == NULL)
    return FALSE;

  return _pMap->GetTile(row, col) == 0;
}

static BOOL FindRandomOpenTile(POINT& ptTile, int maxAttempts)
{
  if (_pMap == NULL || _pMap->GetRows() <= 2 || _pMap->GetCols() <= 2)
    return FALSE;

  for (int attempt = 0; attempt < maxAttempts; attempt++)
  {
    int row = 1 + (rand() % (_pMap->GetRows() - 2));
    int col = 1 + (rand() % (_pMap->GetCols() - 2));
    if (!IsOpenFloorTile(row, col))
      continue;

    ptTile.x = col;
    ptTile.y = row;
    return TRUE;
  }

  return FALSE;
}

static BOOL IsClearHorizontalPath(int row, int colA, int colB)
{
  int left = min(colA, colB);
  int right = max(colA, colB);

  for (int col = left + 1; col < right; col++)
  {
    if (!IsOpenFloorTile(row, col))
      return FALSE;
  }

  return TRUE;
}

static BOOL IsClearVerticalPath(int col, int rowA, int rowB)
{
  int top = min(rowA, rowB);
  int bottom = max(rowA, rowB);

  for (int row = top + 1; row < bottom; row++)
  {
    if (!IsOpenFloorTile(row, col))
      return FALSE;
  }

  return TRUE;
}

static BOOL TryCreateSkeletonRouteFromTile(int row, int col, std::vector<POINT>& vRoute)
{
  if (_pMap == NULL)
    return FALSE;

  if (!IsOpenFloorTile(row, col))
    return FALSE;

  const int minDistance = 4;
  const int maxDistance = 10;

  for (int distance = minDistance; distance <= maxDistance; distance++)
  {
    if (col + distance < _pMap->GetCols() - 1 &&
      IsOpenFloorTile(row, col + distance) &&
      IsClearHorizontalPath(row, col, col + distance))
    {
      POINT ptStart = { col * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2 };
      POINT ptEnd = { (col + distance) * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2 };
      vRoute.clear();
      vRoute.push_back(ptStart);
      vRoute.push_back(ptEnd);
      return TRUE;
    }

    if (col - distance > 0 &&
      IsOpenFloorTile(row, col - distance) &&
      IsClearHorizontalPath(row, col, col - distance))
    {
      POINT ptStart = { col * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2 };
      POINT ptEnd = { (col - distance) * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2 };
      vRoute.clear();
      vRoute.push_back(ptStart);
      vRoute.push_back(ptEnd);
      return TRUE;
    }

    if (row + distance < _pMap->GetRows() - 1 &&
      IsOpenFloorTile(row + distance, col) &&
      IsClearVerticalPath(col, row, row + distance))
    {
      POINT ptStart = { col * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2 };
      POINT ptEnd = { col * TILE_SIZE + TILE_SIZE / 2, (row + distance) * TILE_SIZE + TILE_SIZE / 2 };
      vRoute.clear();
      vRoute.push_back(ptStart);
      vRoute.push_back(ptEnd);
      return TRUE;
    }

    if (row - distance > 0 &&
      IsOpenFloorTile(row - distance, col) &&
      IsClearVerticalPath(col, row, row - distance))
    {
      POINT ptStart = { col * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2 };
      POINT ptEnd = { col * TILE_SIZE + TILE_SIZE / 2, (row - distance) * TILE_SIZE + TILE_SIZE / 2 };
      vRoute.clear();
      vRoute.push_back(ptStart);
      vRoute.push_back(ptEnd);
      return TRUE;
    }
  }

  return FALSE;
}

static BOOL CreateSkeletonPatrolRoute(std::vector<POINT>& vRoute)
{
  vRoute.clear();

  if (_pMap == NULL)
    return FALSE;

  for (int attempt = 0; attempt < 250; attempt++)
  {
    int row = 1 + (rand() % (_pMap->GetRows() - 2));
    int col = 1 + (rand() % (_pMap->GetCols() - 2));
    if (TryCreateSkeletonRouteFromTile(row, col, vRoute))
      return TRUE;
  }

  return FALSE;
}

void SpawnBatEnemies()
{
  DebugSetPhase(TEXT("SpawnBatEnemies"));

  if (_pGame == NULL || _pMap == NULL || _pBatBitmap == NULL)
    return;

  RECT rcWorldBounds = { 0, 0, _pMap->GetCols() * TILE_SIZE, _pMap->GetRows() * TILE_SIZE };
  POINT ptTile;
  if (!FindRandomOpenTile(ptTile, 300))
  {
    DebugLogEvent(TEXT("spawn Bat failed: no open spawn tile"));
    return;
  }

  POINT ptSpawn = { ptTile.x * TILE_SIZE, ptTile.y * TILE_SIZE };
  BatEnemy* pBat = new BatEnemy(_pBatBitmap, ptSpawn, rcWorldBounds);
  pBat->SetZOrder(1);
  _vEnemies.push_back(pBat);
  _pGame->AddSprite(pBat);
  DebugLogFormat(TEXT("spawn Bat sprite=0x%p pos=(%ld,%ld) enemies=%u"),
    pBat, ptSpawn.x, ptSpawn.y, (unsigned int)_vEnemies.size());
}

void SpawnGhostEnemies()
{
  DebugSetPhase(TEXT("SpawnGhostEnemies"));

  if (_pGame == NULL || _pMap == NULL || _pGhostBitmap == NULL)
    return;

  RECT rcWorldBounds = { 0, 0, _pMap->GetCols() * TILE_SIZE, _pMap->GetRows() * TILE_SIZE };
  POINT ptTile;
  if (!FindRandomOpenTile(ptTile, 300))
  {
    DebugLogEvent(TEXT("spawn Ghost failed: no open spawn tile"));
    return;
  }

  POINT ptSpawn = { ptTile.x * TILE_SIZE, ptTile.y * TILE_SIZE };
  GhostEnemy* pGhost = new GhostEnemy(_pGhostBitmap, ptSpawn, rcWorldBounds);
  pGhost->SetZOrder(1);
  _vEnemies.push_back(pGhost);
  IgnoreMapCollision(pGhost);
  _pGame->AddSprite(pGhost);
  DebugLogFormat(TEXT("spawn Ghost sprite=0x%p pos=(%ld,%ld) enemies=%u"),
    pGhost, ptSpawn.x, ptSpawn.y, (unsigned int)_vEnemies.size());
}

void SpawnSkeletonProjectile(POINT ptStart, POINT ptTarget)
{
  DebugSetPhase(TEXT("SpawnSkeletonProjectile"));

  Bitmap* pProjectileBitmap = _pBoneBitmap;
  if (_pGame == NULL || pProjectileBitmap == NULL || _pMap == NULL)
    return;

  RECT rcWorldBounds = { 0, 0, _pMap->GetCols() * TILE_SIZE, _pMap->GetRows() * TILE_SIZE };
  SkeletonProjectile* pShot = new SkeletonProjectile(pProjectileBitmap, ptStart, ptTarget, rcWorldBounds);
  pShot->SetZOrder(2);
  _pGame->AddSprite(pShot);
  //PlaySound(TEXT("Sounds\\swing.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
  DebugLogFormat(TEXT("spawn SkeletonProjectile sprite=0x%p start=(%ld,%ld) target=(%ld,%ld)"),
    pShot, ptStart.x, ptStart.y, ptTarget.x, ptTarget.y);
}

void SpawnSkeletonEnemies()
{
  DebugSetPhase(TEXT("SpawnSkeletonEnemies"));

  Bitmap* pSkeletonBitmap = _pSkeletonBitmap;
  if (_pGame == NULL || _pMap == NULL || pSkeletonBitmap == NULL)
    return;

  std::vector<POINT> vRoute;
  if (!CreateSkeletonPatrolRoute(vRoute) || vRoute.size() < 2)
  {
    DebugLogEvent(TEXT("spawn Skeleton failed: no patrol route"));
    return;
  }

  RECT rcWorldBounds = { 0, 0, _pMap->GetCols() * TILE_SIZE, _pMap->GetRows() * TILE_SIZE };
  POINT ptSpawnCenter = vRoute[0];

  SkeletonEnemy* pSkeleton = new SkeletonEnemy(pSkeletonBitmap, ptSpawnCenter, rcWorldBounds);
  RECT rcSkeleton = pSkeleton->GetPosition();
  int skeletonWidth = rcSkeleton.right - rcSkeleton.left;
  int skeletonHeight = rcSkeleton.bottom - rcSkeleton.top;
  POINT ptSpawn = { ptSpawnCenter.x - skeletonWidth / 2, ptSpawnCenter.y - skeletonHeight / 2 };
  pSkeleton->SetPosition(ptSpawn);
  pSkeleton->SetPatrolRoute(vRoute);
  pSkeleton->SetZOrder(1);
  _vEnemies.push_back(pSkeleton);
  _pGame->AddSprite(pSkeleton);
  DebugLogFormat(TEXT("spawn Skeleton sprite=0x%p pos=(%ld,%ld) routeStart=(%ld,%ld) routeEnd=(%ld,%ld) enemies=%u"),
    pSkeleton, ptSpawn.x, ptSpawn.y,
    vRoute[0].x, vRoute[0].y, vRoute[1].x, vRoute[1].y,
    (unsigned int)_vEnemies.size());
}

void SpawnTankGolemEnemies()
{
  DebugSetPhase(TEXT("SpawnTankGolemEnemies"));

  if (_pGame == NULL || _pMap == NULL || _pGolemBitmap == NULL)
    return;

  RECT rcWorldBounds = { 0, 0, _pMap->GetCols() * TILE_SIZE, _pMap->GetRows() * TILE_SIZE };
  POINT ptTile;
  if (!FindRandomOpenTile(ptTile, 400))
  {
    DebugLogEvent(TEXT("spawn TankGolem failed: no open spawn tile"));
    return;
  }

  POINT ptSpawn = { ptTile.x * TILE_SIZE, ptTile.y * TILE_SIZE };
  TankGolemEnemy* pGolem = new TankGolemEnemy(_pGolemBitmap, ptSpawn, rcWorldBounds);
  pGolem->ConfigurePathfinding(_pMap->GetRows(), _pMap->GetCols(), TILE_SIZE,
    IsTankGolemPathTileBlocked, _pMap);
  pGolem->SetZOrder(1);
  _vEnemies.push_back(pGolem);
  _pGame->AddSprite(pGolem);
  DebugLogFormat(TEXT("spawn TankGolem sprite=0x%p pos=(%ld,%ld) mode=%s enemies=%u"),
    pGolem, ptSpawn.x, ptSpawn.y, TEXT("random"),
    (unsigned int)_vEnemies.size());
}

void SpawnEnemiesForCurrentLevel()
{
  if (_pMap == NULL)
    return;

  int level = _pMap->GetLevel();
  int count = level + 2;

  int batCount = count;
  int ghostCount = count;
  int skeletonCount = (level >= 2) ? count : 0;
  int golemCount = (level >= 3) ? count : 0;

  DebugLogFormat(TEXT("SpawnEnemiesForCurrentLevel level=%d bat=%d ghost=%d skeleton=%d golem=%d"),
    level, batCount, ghostCount, skeletonCount, golemCount);

  for (int i = 0; i < batCount; i++)
    SpawnBatEnemies();
  for (int i = 0; i < ghostCount; i++)
    SpawnGhostEnemies();
  for (int i = 0; i < skeletonCount; i++)
    SpawnSkeletonEnemies();
  for (int i = 0; i < golemCount; i++)
    SpawnTankGolemEnemies();
}

void DescendLevel()
{
  DebugSetPhase(TEXT("DescendLevel"));

  if (_pMap == NULL || _pPlayer == NULL) return;

  ClearEnemies();

  // 1. Determine next level Depth and instantiate a new procedural layout!
  int nextLevel = _pMap->GetLevel() + 1;
  DebugLogFormat(TEXT("DescendLevel begin nextLevel=%d"), nextLevel);
  delete _pMap;
  _pMap = new ProceduralMapGeneration(51, 51, 0, nextLevel);

  // 2. Zero out current floating texts to prevent layout artifacts
  _vFloatingTexts.clear();

  // 3. Delegate safe starting point spatial searches directly to the Map Object!
  POINT ptSpawn = _pMap->FindStartingSpawnTile();
  int spawnRow = ptSpawn.y;
  int spawnCol = ptSpawn.x;

  // 4. Physically relocate the player to center center of that safe coordinate!

  // 4. Physically relocate the player to center center of that safe coordinate!
  int tileCenterX = spawnCol * TILE_SIZE + (TILE_SIZE / 2);
  int tileCenterY = spawnRow * TILE_SIZE + (TILE_SIZE / 2);

  int playerWidth = _pPlayer->GetPosition().right - _pPlayer->GetPosition().left;
  int playerHeight = _pPlayer->GetPosition().bottom - _pPlayer->GetPosition().top;

  int spawnX = tileCenterX - (playerWidth / 2);
  int spawnY = tileCenterY - (playerHeight / 2);

  _pPlayer->SetPosition(spawnX, spawnY);
  _pPlayer->SetVelocity(0, 0);
  _pPlayer->Heal(1);

  // 5. Deliver highly polished HUD Level Descend notice!
  TCHAR szMsg[32];
  wsprintf(szMsg, TEXT("DEPTH %d"), nextLevel);
  AddFloatingText(tileCenterX - 32, tileCenterY - 48, szMsg, RGB(0, 255, 255)); // Vibrant Cyan
  AddFloatingText(tileCenterX - 32, tileCenterY - 76, TEXT("HP +1"), RGB(80, 255, 120));

  SpawnEnemiesForCurrentLevel();
  DebugLogFormat(TEXT("DescendLevel complete level=%d player=(%d,%d)"),
    nextLevel, spawnX, spawnY);
}

void GameStart(HWND hWindow)
{
  DebugSetPhase(TEXT("GameStart"));
  DebugLogEvent(TEXT("GameStart begin"));

  // Seed the random number generator
  srand(GetTickCount());

  // Create the offscreen device context and bitmap
  _hOffscreenDC = CreateCompatibleDC(GetDC(hWindow));
  _hOffscreenBitmap = CreateCompatibleBitmap(GetDC(hWindow),
    _pGame->GetWidth(), _pGame->GetHeight());
  SelectObject(_hOffscreenDC, _hOffscreenBitmap);

  // Create and load the asteroid bitmap
  HDC hDC = GetDC(hWindow);
  _pAsteroidBitmap = new Bitmap(hDC, IDB_ASTEROID, _hInstance);
  _pTilesetBitmap = new Bitmap(hDC, TEXT("bitmaps/DungeonTileset.bmp"));
  _pStairsBitmap = new Bitmap(hDC, TEXT("bitmaps/stairs.bmp"));
  _pToolIconsBitmap = new Bitmap(hDC, TEXT("bitmaps/sword_pickaxe_icons.bmp"));

  // enemies
  _pGhostBitmap = new Bitmap(hDC, TEXT("bitmaps/ghost.bmp"));
  _pBatBitmap = new Bitmap(hDC, TEXT("bitmaps/newbat.bmp"));
  _pGolemBitmap = new Bitmap(hDC, TEXT("bitmaps/golem.bmp"));
  _pSkeletonBitmap = new Bitmap(hDC, TEXT("bitmaps/skeleton.bmp"));
  _pBoneBitmap = new Bitmap(hDC, TEXT("bitmaps/bone.bmp"));

  // Load Ore Bitmaps in defined logical order (1 to 5)
  _pOreBitmaps[0] = new Bitmap(hDC, TEXT("bitmaps/IronOre.bmp"));
  _pOreBitmaps[1] = new Bitmap(hDC, TEXT("bitmaps/AmethystOre.bmp"));
  _pOreBitmaps[2] = new Bitmap(hDC, TEXT("bitmaps/GoldOre.bmp"));
  _pOreBitmaps[3] = new Bitmap(hDC, TEXT("bitmaps/RubyOre.bmp"));
  _pOreBitmaps[4] = new Bitmap(hDC, TEXT("bitmaps/ObsidianOre.bmp"));

  // Load player animations (0: Down, 1: Left, 2: Right, 3: Up)
  _pPlayerBitmaps[0] = new Bitmap(hDC, TEXT("bitmaps/player/player_run_down.bmp"));
  _pPlayerBitmaps[1] = new Bitmap(hDC, TEXT("bitmaps/player/player_run_left.bmp"));
  _pPlayerBitmaps[2] = new Bitmap(hDC, TEXT("bitmaps/player/player_run_right.bmp"));
  _pPlayerBitmaps[3] = new Bitmap(hDC, TEXT("bitmaps/player/player_run_up.bmp"));

  // Load new player attack animations (Same order: Down, Left, Right, Up)
  _pPlayerAttackBitmaps[0] = new Bitmap(hDC, TEXT("bitmaps/player/player_attack_down.bmp"));
  _pPlayerAttackBitmaps[1] = new Bitmap(hDC, TEXT("bitmaps/player/player_attack_left.bmp"));
  _pPlayerAttackBitmaps[2] = new Bitmap(hDC, TEXT("bitmaps/player/player_attack_right.bmp"));
  _pPlayerAttackBitmaps[3] = new Bitmap(hDC, TEXT("bitmaps/player/player_attack_up.bmp"));

  // Initialize the larger procedural map - LEVEL 1 selected
  _pMap = new ProceduralMapGeneration(51, 51, 0, 1); 

  // Create the starry background
  _pBackground = new StarryBackground(1280, 720);

  // Define world bounds
  RECT rcWorldBounds = { 0, 0, 51 * TILE_SIZE, 51 * TILE_SIZE };

  _pPlayer = new Player(_pPlayerBitmaps, rcWorldBounds, BA_STOP);
  _pPlayer->SetAttackBitmaps(_pPlayerAttackBitmaps); // Bind attack bitmaps!

  // Delegate spatial starting point searches cleanly to the instantiated map object!
  POINT ptSpawn = _pMap->FindStartingSpawnTile();
  int spawnRow = ptSpawn.y;
  int spawnCol = ptSpawn.x;
  int tileCenterX = spawnCol * TILE_SIZE + (TILE_SIZE / 2);
  int tileCenterY = spawnRow * TILE_SIZE + (TILE_SIZE / 2);

  int playerWidth = _pPlayer->GetPosition().right - _pPlayer->GetPosition().left;
  int playerHeight = _pPlayer->GetPosition().bottom - _pPlayer->GetPosition().top;

  int spawnX = tileCenterX - (playerWidth / 2);
  int spawnY = tileCenterY - (playerHeight / 2);

  _pPlayer->SetPosition(spawnX, spawnY);
  _pPlayer->SetVelocity(0, 0);
  _pGame->AddSprite(_pPlayer);

  SpawnEnemiesForCurrentLevel();
  DebugLogFormat(TEXT("GameStart spawned enemies=%u"), (unsigned int)_vEnemies.size());

  // Initialize the dynamic lighting system (Encapsulated!)
  _pLightMask = new LightMask(hDC);
  ReleaseDC(hWindow, hDC);

  DebugLogEvent(TEXT("GameStart complete"));
}

static void CleanupGameResources(BOOL bDeleteGameEngine)
{
  // Cleanup the offscreen device context and bitmap
  if (_hOffscreenBitmap != NULL)
  {
    DeleteObject(_hOffscreenBitmap);
    _hOffscreenBitmap = NULL;
  }
  if (_hOffscreenDC != NULL)
  {
    DeleteDC(_hOffscreenDC);
    _hOffscreenDC = NULL;
  }

  // Cleanup sprites before deleting shared bitmap resources they reference
  if (_pGame != NULL)
    _pGame->CleanupSprites();
  _pPlayer = NULL;
  _vEnemies.clear();
  _vMapCollisionIgnoredSprites.clear();
  _vFloatingTexts.clear();

  // Cleanup the asteroid and tileset bitmaps
  delete _pAsteroidBitmap;
  _pAsteroidBitmap = NULL;
  delete _pTilesetBitmap;
  _pTilesetBitmap = NULL;
  delete _pStairsBitmap;
  _pStairsBitmap = NULL;
  delete _pToolIconsBitmap;
  _pToolIconsBitmap = NULL;

  // enemies
  delete _pGhostBitmap;
  _pGhostBitmap = NULL;
  delete _pBatBitmap;
  _pBatBitmap = NULL;
  delete _pGolemBitmap;
  _pGolemBitmap = NULL;
  delete _pSkeletonBitmap;
  _pSkeletonBitmap = NULL;
  delete _pBoneBitmap;
  _pBoneBitmap = NULL;

  // Cleanup Ore Bitmaps
  for (int i = 0; i < 5; i++)
  {
    delete _pOreBitmaps[i];
    _pOreBitmaps[i] = NULL;
  }

  // Cleanup Player Animation Bitmaps
  for (int i = 0; i < 4; i++)
  {
    delete _pPlayerBitmaps[i];
    _pPlayerBitmaps[i] = NULL;
  }
  for (int i = 0; i < 4; i++)
  {
    delete _pPlayerAttackBitmaps[i];
    _pPlayerAttackBitmaps[i] = NULL;
  }

  // Cleanup the light mask system
  delete _pLightMask;
  _pLightMask = NULL;

  // Cleanup the map
  delete _pMap;
  _pMap = NULL;

  // Cleanup the background
  delete _pBackground;
  _pBackground = NULL;

  if (bDeleteGameEngine)
  {
    delete _pGame;
    _pGame = NULL;
  }
}

void GameEnd()
{
  static BOOL s_bGameEnded = FALSE;
  if (s_bGameEnded)
    return;
  s_bGameEnded = TRUE;

  DebugSetPhase(TEXT("GameEnd"));
  DebugLogEvent(TEXT("GameEnd begin"));
  CleanupGameResources(TRUE);
  DebugLogEvent(TEXT("GameEnd complete"));
}

void RestartGame()
{
  if (_pGame == NULL)
    return;

  HWND hWindow = _pGame->GetWindow();
  DebugLogEvent(TEXT("RestartGame"));
  CleanupGameResources(FALSE);
  GameStart(hWindow);
}

void GameActivate(HWND hWindow)
{
}

void GameDeactivate(HWND hWindow)
{
}

void GamePaint(HDC hDC)
{
  // Draw the fixed starry background first (0,0 screen space)
  _pBackground->Draw(hDC);

  POINT ptOldOrg;
  SetWindowOrgEx(hDC, _iCameraX, _iCameraY, &ptOldOrg);

  // Draw the procedural map tiles ONLY within the view frustum (Culling Optimization)
  if (_pMap != NULL && _pTilesetBitmap != NULL)
  {
    // Calculate tile boundaries based on camera position
    int startCol = max(0, _iCameraX / TILE_SIZE);
    int endCol   = min(_pMap->GetCols(), (_iCameraX + 1280) / TILE_SIZE + 1);
    int startRow = max(0, _iCameraY / TILE_SIZE);
    int endRow   = min(_pMap->GetRows(), (_iCameraY + 720) / TILE_SIZE + 1);

    for (int r = startRow; r < endRow; ++r)
    {
      for (int c = startCol; c < endCol; ++c)
      {
        int tileValue = _pMap->GetTile(r, c);
        
        // First, render Floor underlying EVERYTHING that isn't a wall
        // (Assuming Ores may have transparent or overlay needs)
        int xSource = (tileValue == 100) ? 0 : 16; 
        _pTilesetBitmap->DrawPartScaled(hDC, c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, xSource, 0, 16, 16, TRUE);

        // Draw Ore if existing (Map values 1-5 correspond to indices 0-4)
        if (tileValue >= 1 && tileValue <= 5)
        {
           Bitmap* pOre = _pOreBitmaps[tileValue - 1];
           if (pOre != NULL) {
               // Ores are native 64x64, so direct Draw aligns perfectly!
               pOre->Draw(hDC, c * TILE_SIZE, r * TILE_SIZE, TRUE);
           }
        }
        // Draw Exit Stairs (Map value 6)
        else if (tileValue == 6)
        {
           if (_pStairsBitmap != NULL) {
               // Scale 16x16 native asset to TILE_SIZE
               _pStairsBitmap->DrawPartScaled(hDC, c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, 0, 0, 16, 16, TRUE);
           }
        }
      }
    }
  }

  // Draw the sprites (which live in world coords, so they are automatically offset too)
  _pGame->DrawSprites(hDC);

  if (_pPlayer != NULL && _pLightMask != NULL)
  {
    RECT rcPlayer = _pPlayer->GetPosition();
    int cx = rcPlayer.left + (rcPlayer.right - rcPlayer.left) / 2;
    int cy = rcPlayer.top + (rcPlayer.bottom - rcPlayer.top) / 2;
    
    _pLightMask->Draw(hDC, cx, cy);
  }

  // --- DRAW FLOATING TEXTS ---
  if (!_vFloatingTexts.empty())
  {
    SetBkMode(hDC, TRANSPARENT);
    HFONT hFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

    for (const auto& ft : _vFloatingTexts)
    {
      SetTextColor(hDC, ft.color);
      TextOut(hDC, ft.x, ft.y, ft.text, lstrlen(ft.text));
    }

    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);
  }

  // Restore logical origin for integrity of next cycle
  SetWindowOrgEx(hDC, ptOldOrg.x, ptOldOrg.y, NULL);

  if (_pPlayer != NULL)
  {
    TCHAR szHealth[32];
    wsprintf(szHealth, TEXT("HP %d/%d"), _pPlayer->GetHealth(), _pPlayer->GetMaxHealth());
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 80, 80));
    TextOut(hDC, 24, 24, szHealth, lstrlen(szHealth));

    if (_pPlayer->IsDead() && _pGame != NULL)
    {
      const TCHAR* szRestart = TEXT("Press R to restart");
      HFONT hFont = CreateFont(42, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
      HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
      UINT oldAlign = SetTextAlign(hDC, TA_CENTER | TA_BASELINE);
      SetTextColor(hDC, RGB(255, 255, 255));
      TextOut(hDC, _pGame->GetWidth() / 2, _pGame->GetHeight() / 2,
        szRestart, lstrlen(szRestart));
      SetTextAlign(hDC, oldAlign);
      SelectObject(hDC, hOldFont);
      DeleteObject(hFont);
    }
  }

  // --- RENDER INVENTORY HUD (8 SQUARE BARS AT THE BOTTOM) ---
  if (_pPlayer != NULL && _pGame != NULL)
  {
    const int numSlots = 8;
    const int barWidth = 56;
    const int barHeight = 56;
    const int spacing = 8;
    const int totalWidth = numSlots * barWidth + (numSlots - 1) * spacing;
    
    int screenW = _pGame->GetWidth();
    int screenH = _pGame->GetHeight();
    
    int hudX = (screenW - totalWidth) / 2;
    int hudY = screenH - barHeight - 15; // 15px from bottom edge

    // High-quality slate/metallic style brushes & pens
    HBRUSH hBgBrush = CreateSolidBrush(RGB(20, 22, 30));
    HPEN hBorderPen = CreatePen(PS_SOLID, 2, RGB(65, 70, 90));
    HPEN hOldPen = (HPEN)SelectObject(hDC, hBorderPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBgBrush);

    // Crisp, small numeric font for slot corners
    HFONT hTextFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
    HFONT hPrevFont = (HFONT)SelectObject(hDC, hTextFont);
    
    SetBkMode(hDC, TRANSPARENT);

    const Inventory& inv = _pPlayer->GetInventory();

    for (int i = 0; i < numSlots; i++)
    {
      int itemID = i + 1;
      int count = inv.GetItemCount(itemID);

      int xLeft = hudX + i * (barWidth + spacing);
      int xRight = xLeft + barWidth;
      int yTop = hudY;
      int yBottom = hudY + barHeight;

      // Always draw the empty inventory grid cell background
      RoundRect(hDC, xLeft, yTop, xRight, yBottom, 8, 8);

      // Render content ONLY if collected items exist in that slot!
      if (count > 0)
      {
        int iconSize = 38;
        int iconX = xLeft + (barWidth - iconSize) / 2;
        int iconY = yTop + (barHeight - iconSize) / 2;

        // 1. Render scaled Ore Icon
        if (itemID >= 1 && itemID <= 5 && _pOreBitmaps[itemID - 1] != NULL)
        {
           _pOreBitmaps[itemID - 1]->DrawPartScaled(hDC, iconX, iconY, iconSize, iconSize, 0, 0, 64, 64, TRUE);
        }

        // 2. Render count text with high-visibility drop shadow pushed precisely to the bottom-right corner
        TCHAR szCount[16];
        wsprintf(szCount, TEXT("%d"), count);
        
        // Target coordinates for the bottom-right corner (with a tight 5px margin from edge)
        int textX = xRight - 5;
        int textY = yBottom - 2;

        // Temporarily switch to RIGHT/BOTTOM alignment for pixel-perfect slot text behavior!
        UINT oldAlign = SetTextAlign(hDC, TA_RIGHT | TA_BOTTOM);

        // Render drop shadow (black offset)
        SetTextColor(hDC, RGB(0, 0, 0));
        TextOut(hDC, textX + 1, textY + 1, szCount, lstrlen(szCount));

        // Render foreground text (golden color)
        SetTextColor(hDC, RGB(255, 225, 120));
        TextOut(hDC, textX, textY, szCount, lstrlen(szCount));

        // Restore previous alignment to avoid side-effects on other HUD texts
        SetTextAlign(hDC, oldAlign);
      }
    }

    // --- RENDER TOOL BARS (LEFT ALIGNED) ---
    int toolHudX = 15;
    int toolHudY = screenH - barHeight - 15;

    for (int i = 0; i < 2; i++)
    {
      int xLeft = toolHudX + i * (barWidth + spacing);
      int xRight = xLeft + barWidth;
      int yTop = toolHudY;
      int yBottom = toolHudY + barHeight;

      // Draw the tool background, highlighted if active
      if (_pPlayer->GetActiveTool() == i)
      {
        HPEN hHighlightPen = CreatePen(PS_SOLID, 3, RGB(255, 215, 0)); // Golden/Yellow border
        HPEN hOldPen2 = (HPEN)SelectObject(hDC, hHighlightPen);
        RoundRect(hDC, xLeft, yTop, xRight, yBottom, 8, 8);
        SelectObject(hDC, hOldPen2);
        DeleteObject(hHighlightPen);
      }
      else
      {
        RoundRect(hDC, xLeft, yTop, xRight, yBottom, 8, 8);
      }

      if (_pToolIconsBitmap != NULL)
      {
        int iconSize = 30;
        int iconX = xLeft + (barWidth - iconSize) / 2;
        int iconY = yTop + (barHeight - iconSize) / 2;
        
        // Image contains 2 icons (60x30), take 30x30 chunks
        int srcX = i * 30;
        _pToolIconsBitmap->DrawPartScaled(hDC, iconX, iconY, iconSize, iconSize, srcX, 0, 30, 30, TRUE);
      }

      // Draw durability text above the box
      TCHAR szDurability[32];
      wsprintf(szDurability, TEXT("%d/%d"), _pPlayer->GetToolDurability(i), _pPlayer->GetMaxToolDurability(i));
      
      int durX = xLeft + (barWidth / 2);
      int durY = yTop - 18;

      UINT oldAlign2 = SetTextAlign(hDC, TA_CENTER | TA_TOP);

      SetTextColor(hDC, RGB(0, 0, 0));
      TextOut(hDC, durX + 1, durY + 1, szDurability, lstrlen(szDurability));

      if (_pPlayer->GetToolDurability(i) == 0)
         SetTextColor(hDC, RGB(255, 60, 60)); // red if broken
      else
         SetTextColor(hDC, RGB(200, 200, 200)); // light gray

      TextOut(hDC, durX, durY, szDurability, lstrlen(szDurability));

      SetTextAlign(hDC, oldAlign2);

      // --- UPGRADE BUTTON ---
      int btnTop = yTop - 65;
      int btnBottom = yTop - 45;
      
      int level = _pPlayer->GetToolLevel(i);
      if (level < 5)
      {
        int cost = 30;
        for (int c = 1; c < level; c++) cost = (int)(cost * 2.5);

        HBRUSH hBtnBrush = CreateSolidBrush(RGB(50, 150, 50));
        HBRUSH hOldBtnBrush = (HBRUSH)SelectObject(hDC, hBtnBrush);
        RoundRect(hDC, xLeft, btnTop, xRight, btnBottom, 4, 4);
        SelectObject(hDC, hOldBtnBrush);
        DeleteObject(hBtnBrush);

        TCHAR szUpg[32];
        wsprintf(szUpg, TEXT("UPG (%d)"), cost);
        
        UINT oldAlign3 = SetTextAlign(hDC, TA_CENTER | TA_TOP);
        SetTextColor(hDC, RGB(0, 0, 0));
        TextOut(hDC, durX + 1, btnTop + 3, szUpg, lstrlen(szUpg));
        SetTextColor(hDC, RGB(255, 255, 255));
        TextOut(hDC, durX, btnTop + 2, szUpg, lstrlen(szUpg));
        SetTextAlign(hDC, oldAlign3);
      }
      // ----------------------

      // --- MATERIAL TEXT ---
      const TCHAR* szMaterial = TEXT("Unknown");
      switch (level)
      {
         case 1: szMaterial = TEXT("Iron"); break;
         case 2: szMaterial = TEXT("Amethyst"); break;
         case 3: szMaterial = TEXT("Gold"); break;
         case 4: szMaterial = TEXT("Ruby"); break;
         case 5: szMaterial = TEXT("Obsidian"); break;
      }
      
      int matY = yTop - 36;
      UINT oldAlignMat = SetTextAlign(hDC, TA_CENTER | TA_TOP);
      SetTextColor(hDC, RGB(0, 0, 0));
      TextOut(hDC, durX + 1, matY + 1, szMaterial, lstrlen(szMaterial));
      SetTextColor(hDC, RGB(200, 200, 255));
      TextOut(hDC, durX, matY, szMaterial, lstrlen(szMaterial));
      SetTextAlign(hDC, oldAlignMat);
      // ----------------------

      // Render count text with high-visibility drop shadow pushed precisely to the bottom-right corner
      TCHAR szNum[16];
      wsprintf(szNum, TEXT("%d"), i + 1);
      
      int textX = xRight - 5;
      int textY = yBottom - 2;

      UINT oldAlign = SetTextAlign(hDC, TA_RIGHT | TA_BOTTOM);

      // Render drop shadow (black offset)
      SetTextColor(hDC, RGB(0, 0, 0));
      TextOut(hDC, textX + 1, textY + 1, szNum, lstrlen(szNum));

      // Render foreground text (white)
      SetTextColor(hDC, RGB(255, 255, 255));
      TextOut(hDC, textX, textY, szNum, lstrlen(szNum));

      SetTextAlign(hDC, oldAlign);
    }

    // Proper GDI cleanup
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, hPrevFont);
    DeleteObject(hBgBrush);
    DeleteObject(hBorderPen);
    DeleteObject(hTextFont);
  }
}

void GameCycle()
{
  DebugSetPhase(TEXT("GameCycle.Begin"));

  POINT ptPlayer = { 0, 0 };
  if (_pPlayer != NULL)
  {
    RECT rcPlayer = _pPlayer->GetPosition();
    ptPlayer.x = rcPlayer.left;
    ptPlayer.y = rcPlayer.top;
  }
  POINT ptCamera = { _iCameraX, _iCameraY };
  int iLevel = (_pMap != NULL) ? _pMap->GetLevel() : -1;
  size_t nSprites = (_pGame != NULL) ? _pGame->GetSpriteCount() : 0;
  DebugFrameHeartbeat(iLevel, ptPlayer, ptCamera, nSprites,
    _vEnemies.size(), _vFloatingTexts.size());

  // Update the background
  DebugSetPhase(TEXT("GameCycle.BackgroundUpdate"));
  _pBackground->Update();

  if (_pPlayer == NULL || !_pPlayer->IsDead())
  {
    // Update the sprites
    DebugSetPhase(TEXT("GameCycle.UpdateSprites"));
    _pGame->UpdateSprites();

    // Update the camera to track the player
    DebugSetPhase(TEXT("GameCycle.UpdateCamera"));
    UpdateCamera();
  }

  // Update active Floating Texts (float upward and decrement lifetimes)
  DebugSetPhase(TEXT("GameCycle.FloatingTexts"));
  for (auto it = _vFloatingTexts.begin(); it != _vFloatingTexts.end(); )
  {
    it->y -= 2; // Move upwards 2 pixels per frame
    it->lifetime--;
    if (it->lifetime <= 0)
      it = _vFloatingTexts.erase(it);
    else
      ++it;
  }

  // Obtain a device context for repainting the game
  HWND  hWindow = _pGame->GetWindow();
  HDC   hDC = GetDC(hWindow);

  // Paint the game to the offscreen device context
  DebugSetPhase(TEXT("GameCycle.Paint"));
  GamePaint(_hOffscreenDC);

  // Blit the offscreen bitmap to the game screen
  BitBlt(hDC, 0, 0, _pGame->GetWidth(), _pGame->GetHeight(),
    _hOffscreenDC, 0, 0, SRCCOPY);

  // Cleanup
  ReleaseDC(hWindow, hDC);
  DebugSetPhase(TEXT("GameCycle.End"));
}

void HandleKeys()
{
  DebugSetPhase(TEXT("HandleKeys"));

  if (_pPlayer != NULL)
  {
    if (_pPlayer->IsDead())
    {
      if (GetAsyncKeyState('R') & 0x8000)
        RestartGame();
      return;
    }

    // Execute character locomotion and mining checks!
    _pPlayer->HandleKeys();

    // --- EXIT STAIRS TRIGGER (F KEY) ---
    // Use an atomic latch to prevent multiple-frame level skipping!
    static bool s_bFKeyLatch = false;
    bool bFIsDown = (GetAsyncKeyState('F') & 0x8000) != 0;

    if (bFIsDown && !s_bFKeyLatch)
    {
      // Pinpoint player physical core coordinate
      RECT rcCol = _pPlayer->GetCollision();
      int cx = rcCol.left + (rcCol.right - rcCol.left) / 2;
      int cy = rcCol.top + (rcCol.bottom - rcCol.top) / 2;

      // Cross-reference projected coordinates back into the tile matrix
      int r = cy / TILE_SIZE;
      int c = cx / TILE_SIZE;

      // Identify if character is physically superimposed on stairs (index 6)
      if (_pMap != NULL && _pMap->GetTile(r, c) == 6)
      {
        DebugLogFormat(TEXT("stairs trigger tile=(%d,%d)"), r, c);
        //PlaySound(TEXT("Sounds\\ladder.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
        DescendLevel();
      }
    }
    s_bFKeyLatch = bFIsDown;
  }
}

void MouseButtonDown(int x, int y, BOOL bLeft)
{
  if (_pPlayer != NULL && _pGame != NULL && bLeft)
  {
    int screenW = _pGame->GetWidth();
    int screenH = _pGame->GetHeight();
    const int barWidth = 56;
    const int barHeight = 56;
    const int spacing = 8;
    int toolHudX = 15;
    int toolHudY = screenH - barHeight - 15;

    for (int i = 0; i < 2; i++)
    {
      int xLeft = toolHudX + i * (barWidth + spacing);
      int xRight = xLeft + barWidth;
      int btnTop = toolHudY - 65;
      int btnBottom = toolHudY - 45;

      if (x >= xLeft && x <= xRight && y >= btnTop && y <= btnBottom)
      {
        int level = _pPlayer->GetToolLevel(i);
        if (level < 5)
        {
          int nextLevel = level + 1;
          int cost = 30;
          for (int c = 1; c < level; c++) cost = (int)(cost * 2.5);

          if (_pPlayer->GetInventory().GetItemCount(nextLevel) >= cost)
          {
            _pPlayer->GetInventory().RemoveItem(nextLevel, cost);
            _pPlayer->UpgradeTool(i);
            //PlaySound(TEXT("Sounds\\pickup.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
          }
        }
      }
    }
  }
}

void MouseButtonUp(int x, int y, BOOL bLeft)
{
}

void MouseMove(int x, int y)
{
}

void HandleJoystick(JOYSTATE jsJoystickState)
{
}

BOOL SpriteCollision(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
  DebugSetPhase(TEXT("SpriteCollision"));

  Enemy* pEnemy = NULL;

  if (pSpriteHitter == _pPlayer)
    pEnemy = GetTrackedEnemy(pSpriteHittee);
  else if (pSpriteHittee == _pPlayer)
    pEnemy = GetTrackedEnemy(pSpriteHitter);

  if (pEnemy != NULL && _pPlayer != NULL)
  {
    if ((DebugGetFrameNumber() % 15) == 0)
    {
      DebugLogFormat(TEXT("player enemy collision enemy=0x%p hitter=0x%p hittee=0x%p"),
        pEnemy, pSpriteHitter, pSpriteHittee);
    }
    pEnemy->TouchPlayer(_pPlayer);
  }

  return FALSE;
}

BOOL MapCollision(Sprite* pSprite)
{
  DebugSetPhase(TEXT("MapCollision"));

  if (_pMap == NULL || pSprite == NULL) return FALSE;

  if (IsMapCollisionIgnored(pSprite))
    return FALSE;

  // IMPORTANT: Use the tightened physical collision rect instead of visual position!
  RECT rcSprite = pSprite->GetCollision();

  int leftTile   = rcSprite.left / TILE_SIZE;
  int rightTile  = (rcSprite.right - 1) / TILE_SIZE;
  int topTile    = rcSprite.top / TILE_SIZE;
  int bottomTile = (rcSprite.bottom - 1) / TILE_SIZE;

  for (int r = topTile; r <= bottomTile; ++r)
  {
    for (int c = leftTile; c <= rightTile; ++c)
    {
      int val = _pMap->GetTile(r, c);
      
      if (val == 100)
      {
        if ((DebugGetFrameNumber() % 30) == 0)
        {
          DebugLogFormat(TEXT("map collision wall sprite=0x%p tile=(%d,%d)"),
            pSprite, r, c);
        }

        // Walls (100) block the sprite completely if the collision rect overlaps
        return TRUE;
      }
      else if (val >= 1 && val <= 5)
      {
        // Shrink the 64x64 ore tile rect by 12 pixels on each edge (results in a 40x40 core)
        RECT rcOre = { c * TILE_SIZE, r * TILE_SIZE, (c + 1) * TILE_SIZE, (r + 1) * TILE_SIZE };
        InflateRect(&rcOre, -12, -12); 

        RECT rcDummy;
        if (IntersectRect(&rcDummy, &rcSprite, &rcOre))
        {
          if ((DebugGetFrameNumber() % 30) == 0)
          {
            DebugLogFormat(TEXT("map collision ore sprite=0x%p tile=(%d,%d) val=%d"),
              pSprite, r, c, val);
          }
          return TRUE; // Actual physical collision with the ore core!
        }
      }
    }
  }

  return FALSE;
}

void SpriteDying(Sprite* pSprite)
{
  DebugSetPhase(TEXT("SpriteDying"));
  DebugLogFormat(TEXT("SpriteDying sprite=0x%p"), pSprite);
  UntrackEnemy(pSprite);
}

//-----------------------------------------------------------------
// Functions
//-----------------------------------------------------------------

void UpdateCamera()
{
  DebugSetPhase(TEXT("UpdateCamera"));

  if (_pPlayer == NULL) return;


  // Camera Tracking
  RECT rcPos = _pPlayer->GetPosition();
  int playerCenterX = rcPos.left + (rcPos.right - rcPos.left) / 2;
  int playerCenterY = rcPos.top + (rcPos.bottom - rcPos.top) / 2;

  _iCameraX = playerCenterX - (1280 / 2);
  _iCameraY = playerCenterY - (720 / 2);

  int worldSize = 51 * TILE_SIZE; 
  if (_iCameraX < 0) _iCameraX = 0;
  if (_iCameraY < 0) _iCameraY = 0;
  if (_iCameraX > worldSize - 1280) _iCameraX = worldSize - 1280;
  if (_iCameraY > worldSize - 720) _iCameraY = worldSize - 720;
}
