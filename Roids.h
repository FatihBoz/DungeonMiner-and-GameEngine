//-----------------------------------------------------------------
// Roids 2 Application
// C++ Header - Roids.h
//-----------------------------------------------------------------

#pragma once

#define TILE_SIZE 64

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include <windows.h>
#include <vector>

struct FloatingText {
  int x, y;
  TCHAR text[32];
  int lifetime;
  COLORREF color;
};
#include "Resource.h"
#include "GameEngine.h"
#include "Bitmap.h"
#include "Sprite.h"
#include "Background.h"
#include "ProceduralMapGeneration.h"
#include "LightMask.h"
#include "Player.h"
#include "Enemy.h"
#include "BatEnemy.h"
#include "GhostEnemy.h"
#include "SkeletonEnemy.h"
#include "SkeletonProjectile.h"
#include "DebugSystem.h"
#include "TankGolemEnemy.h"

//-----------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------
HINSTANCE         _hInstance;
GameEngine*       _pGame;
HDC               _hOffscreenDC;
HBITMAP           _hOffscreenBitmap;
Bitmap*           _pAsteroidBitmap;
Bitmap*           _pSaucerBitmap;
// enemies bitmap
Bitmap*           _pGhostBitmap;
Bitmap*				_pBatBitmap;
Bitmap*           _pGolemBitmap;
Bitmap*           _pSkeletonBitmap;
Bitmap*           _pBoneBitmap;
StarryBackground* _pBackground;
Sprite*           _pAsteroids[3];
Player*           _pPlayer;
Bitmap*           _pTilesetBitmap;
Bitmap*           _pOreBitmaps[5];
Bitmap*           _pStairsBitmap;
Bitmap*           _pToolIconsBitmap;
Bitmap*           _pPlayerBitmaps[4];       // 0: Down, 1: Left, 2: Right, 3: Up
Bitmap*           _pPlayerAttackBitmaps[4]; // Attack: Down, Left, Right, Up
ProceduralMapGeneration* _pMap;
LightMask*        _pLightMask;
std::vector<Enemy*> _vEnemies;
std::vector<Sprite*> _vMapCollisionIgnoredSprites;
int               _iCameraX;
int               _iCameraY;

// External tracking systems
extern std::vector<FloatingText> _vFloatingTexts;

//-----------------------------------------------------------------
// Function Declarations
//-----------------------------------------------------------------
void UpdateSaucer();
void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color);
void DescendLevel();
void RestartGame();
void ClearEnemies();
void SpawnBatEnemies();
void SpawnGhostEnemies();
void SpawnSkeletonEnemies();
void SpawnTankGolemEnemies();
void SpawnEnemiesForCurrentLevel();
void SpawnSkeletonProjectile(POINT ptStart, POINT ptTarget);
void UntrackEnemy(Sprite* pSprite);
Enemy* GetTrackedEnemy(Sprite* pSprite);
void IgnoreMapCollision(Sprite* pSprite);
BOOL IsMapCollisionIgnored(Sprite* pSprite);
BOOL IsTankGolemPathTileBlocked(int row, int col, void* pContext);
