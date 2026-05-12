//-----------------------------------------------------------------
// Roids 2 Application
// C++ Header - Roids.h
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include <windows.h>
#include "Resource.h"
#include "GameEngine.h"
#include "Bitmap.h"
#include "Sprite.h"
#include "Background.h"

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
Sprite*           _pSaucer;

//-----------------------------------------------------------------
// Function Declarations
//-----------------------------------------------------------------
void UpdateSaucer();
