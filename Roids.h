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

//-----------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------
HINSTANCE         _hInstance;
GameEngine*       _pGame;
HDC               _hOffscreenDC;
HBITMAP           _hOffscreenBitmap;
Bitmap*           _pAsteroidBitmap;
Bitmap*           _pSaucerBitmap;
StarryBackground* _pBackground;
Sprite*           _pAsteroids[3];
Player*           _pPlayer;
Bitmap*           _pTilesetBitmap;
Bitmap*           _pOreBitmaps[5];
Bitmap*           _pStairsBitmap;
Bitmap*           _pPlayerBitmaps[4];       // 0: Down, 1: Left, 2: Right, 3: Up
Bitmap*           _pPlayerAttackBitmaps[4]; // Attack: Down, Left, Right, Up
ProceduralMapGeneration* _pMap;
LightMask*        _pLightMask;
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
