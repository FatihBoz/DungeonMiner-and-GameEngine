//-----------------------------------------------------------------
// Roids 2 Application
// C++ Source - Roids.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Roids.h"
#include <cmath>

#pragma comment(lib, "Msimg32.lib")

#define TILE_SIZE 64



//-----------------------------------------------------------------
// Game Engine Functions
//-----------------------------------------------------------------
BOOL GameInitialize(HINSTANCE hInstance)
{
  // Create the game engine
  _pGame = new GameEngine(hInstance, TEXT("Roids 2"),
    TEXT("Roids 2"), IDI_ROIDS, IDI_ROIDS_SM, 1280, 720);
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
  FloatingText ft;
  ft.x = x;
  ft.y = y;
  ft.lifetime = 20; // Lives for 20 render cycles
  ft.color = color;
  lstrcpyn(ft.text, szText, 32);
  _vFloatingTexts.push_back(ft);
}

void DescendLevel()
{
  if (_pMap == NULL || _pPlayer == NULL) return;

  // 1. Determine next level Depth and instantiate a new procedural layout!
  int nextLevel = _pMap->GetLevel() + 1;
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

  // 5. Deliver highly polished HUD Level Descend notice!
  TCHAR szMsg[32];
  wsprintf(szMsg, TEXT("DEPTH %d"), nextLevel);
  AddFloatingText(tileCenterX - 32, tileCenterY - 48, szMsg, RGB(0, 255, 255)); // Vibrant Cyan
}

void GameStart(HWND hWindow)
{
  // Seed the random number generator
  srand(GetTickCount());

  // Create the offscreen device context and bitmap
  _hOffscreenDC = CreateCompatibleDC(GetDC(hWindow));
  _hOffscreenBitmap = CreateCompatibleBitmap(GetDC(hWindow),
    _pGame->GetWidth(), _pGame->GetHeight());
  SelectObject(_hOffscreenDC, _hOffscreenBitmap);

  // Create and load the asteroid and saucer bitmaps
  HDC hDC = GetDC(hWindow);
  _pAsteroidBitmap = new Bitmap(hDC, IDB_ASTEROID, _hInstance);
  _pSaucerBitmap = new Bitmap(hDC, IDB_SAUCER, _hInstance);
  _pTilesetBitmap = new Bitmap(hDC, TEXT("bitmaps/DungeonTileset.bmp"));
  _pStairsBitmap = new Bitmap(hDC, TEXT("bitmaps/stairs.bmp"));

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

  // Initialize the dynamic lighting system (Encapsulated!)
  _pLightMask = new LightMask(hDC);
  ReleaseDC(hWindow, hDC);

}

void GameEnd()
{
  // Cleanup the offscreen device context and bitmap
  DeleteObject(_hOffscreenBitmap);
  DeleteDC(_hOffscreenDC);  

  // Cleanup the asteroid, saucer, and tileset bitmaps
  delete _pAsteroidBitmap;
  delete _pSaucerBitmap;
  delete _pTilesetBitmap;
  delete _pStairsBitmap;
  
  // Cleanup Ore Bitmaps
  for (int i = 0; i < 5; i++) delete _pOreBitmaps[i];

  // Cleanup Player Animation Bitmaps
  for (int i = 0; i < 4; i++) delete _pPlayerBitmaps[i];
  for (int i = 0; i < 4; i++) delete _pPlayerAttackBitmaps[i];

  // Cleanup the light mask system
  delete _pLightMask;

  // Cleanup the map
  delete _pMap;

  // Cleanup the background
  delete _pBackground;

  // Cleanup the sprites
  _pGame->CleanupSprites();

  // Cleanup the game engine
  delete _pGame;
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

  // Draw the sprites (which live in world coords, so they are automatically offset too!)
  _pGame->DrawSprites(hDC);

  // --- AYDINLATMA SİSTEMİ ---
  if (_pPlayer != NULL && _pLightMask != NULL)
  {
    RECT rcSaucer = _pPlayer->GetPosition();
    int cx = rcSaucer.left + (rcSaucer.right - rcSaucer.left) / 2;
    int cy = rcSaucer.top + (rcSaucer.bottom - rcSaucer.top) / 2;
    
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
  // Update the background
  _pBackground->Update();

  // Update the sprites
  _pGame->UpdateSprites();

  // Update the saucer to help it dodge the asteroids
  UpdateSaucer();

  // Update active Floating Texts (float upward and decrement lifetimes)
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
  GamePaint(_hOffscreenDC);

  // Blit the offscreen bitmap to the game screen
  BitBlt(hDC, 0, 0, _pGame->GetWidth(), _pGame->GetHeight(),
    _hOffscreenDC, 0, 0, SRCCOPY);

  // Cleanup
  ReleaseDC(hWindow, hDC);
}

void HandleKeys()
{
  if (_pPlayer != NULL)
  {
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
        DescendLevel();
      }
    }
    s_bFKeyLatch = bFIsDown;
  }
}

void MouseButtonDown(int x, int y, BOOL bLeft)
{
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
  return FALSE;
}

BOOL MapCollision(Sprite* pSprite)
{
  if (_pMap == NULL || pSprite == NULL) return FALSE;

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
          return TRUE; // Actual physical collision with the ore core!
        }
      }
    }
  }

  return FALSE;
}

void SpriteDying(Sprite* pSprite)
{
}

//-----------------------------------------------------------------
// Functions
//-----------------------------------------------------------------

void UpdateSaucer()
{
  if (_pPlayer == NULL) return;


  // Camera Tracking
  RECT rcPos = _pPlayer->GetPosition();
  int saucerCenterX = rcPos.left + (rcPos.right - rcPos.left) / 2;
  int saucerCenterY = rcPos.top + (rcPos.bottom - rcPos.top) / 2;

  _iCameraX = saucerCenterX - (1280 / 2);
  _iCameraY = saucerCenterY - (720 / 2);

  int worldSize = 51 * TILE_SIZE; 
  if (_iCameraX < 0) _iCameraX = 0;
  if (_iCameraY < 0) _iCameraY = 0;
  if (_iCameraX > worldSize - 1280) _iCameraX = worldSize - 1280;
  if (_iCameraY > worldSize - 720) _iCameraY = worldSize - 720;
}
