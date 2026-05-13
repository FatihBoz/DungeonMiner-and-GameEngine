#include "Player.h"
#include "ProceduralMapGeneration.h"

#define TILE_SIZE 64

Player::Player(Bitmap* pBitmaps[4], RECT& rcBounds, BOUNDSACTION baBoundsAction)
  : Sprite(pBitmaps[0], rcBounds, baBoundsAction), m_iSpeed(8), m_iCurrentDir(PDIR_DOWN), m_bIsMoving(FALSE), m_bIsAttacking(FALSE), m_dScale(2.0)
{
  // Store all 4 directional bitmaps and initialize attack storage
  for (int i = 0; i < 4; i++)
  {
    m_pAnimationBitmaps[i] = pBitmaps[i];
    m_pAttackBitmaps[i] = NULL;
  }

  // Setup base sprite animation properties
  // The strips are 100x24, each frame is 20x24 (5 frames)
  SetNumFrames(5);
  SetFrameDelay(3); // Advances frame every 3 updates

  // Apply initial scaling factor and adjust logical collision boundaries!
  SetScale(2.0); 
}

Player::~Player()
{
  // The game engine's GameEnd method manages the lifetime/deletion of bitmaps,
  // so we do not need to delete them here.
}

void Player::HandleKeys()
{
  // Lock user inputs and stop speed during an active attack swing
  if (m_bIsAttacking)
  {
    SetVelocity(0, 0);
    return;
  }

  // Detect SPACE key to initiate directional mineral mining!
  if ((GetAsyncKeyState(VK_SPACE) < 0))
  {
    if (m_pAttackBitmaps[m_iCurrentDir] != NULL)
    {
      m_bIsAttacking = TRUE;
      m_bIsMoving = FALSE;
      SetVelocity(0, 0); // Zero out velocity instantly
      m_iCurFrame = 0;   // Jump to the start of the swing sequence
      SetNumFrames(5);   // The attack bitmap strips contain exactly 5 frames (120 / 24px)
      SetFrameDelay(3);  // Visual velocity of the mining strike
      m_pBitmap = m_pAttackBitmaps[m_iCurrentDir];
      
      // Recalculate visual bounding box to block legacy engine /6 division shrink!
      SetScale(m_dScale); 
      return;
    }
  }

  POINT ptVelocity = { 0, 0 };
  m_bIsMoving = FALSE;

  // Check keyboard state and switch direction/active bitmap accordingly
  if (GetAsyncKeyState(VK_LEFT) < 0)
  {
    ptVelocity.x -= m_iSpeed;
    m_iCurrentDir = PDIR_LEFT;
    m_bIsMoving = TRUE;
  }
  else if (GetAsyncKeyState(VK_RIGHT) < 0)
  {
    ptVelocity.x += m_iSpeed;
    m_iCurrentDir = PDIR_RIGHT;
    m_bIsMoving = TRUE;
  }

  if (GetAsyncKeyState(VK_UP) < 0)
  {
    ptVelocity.y -= m_iSpeed;
    m_iCurrentDir = PDIR_UP;
    m_bIsMoving = TRUE;
  }
  else if (GetAsyncKeyState(VK_DOWN) < 0)
  {
    ptVelocity.y += m_iSpeed;
    m_iCurrentDir = PDIR_DOWN;
    m_bIsMoving = TRUE;
  }

  // Dynamically point to the active directional bitmap strip!
  m_pBitmap = m_pAnimationBitmaps[m_iCurrentDir];

  SetVelocity(ptVelocity);
}

SPRITEACTION Player::Update()
{
  if (m_bIsAttacking)
  {
    // Wait for the 5th attack frame (index 4) to finish playing
    if (m_iCurFrame == 4 && m_iFrameTrigger == 1)
    {
      // Trigger damage calculations against nearby minerals
      ExecuteAttackDamage();

      // Re-enable movement and return back to standard 5-frame Run layouts!
      m_bIsAttacking = FALSE;
      SetNumFrames(5); 
      m_pBitmap = m_pAnimationBitmaps[m_iCurrentDir];
      m_iCurFrame = 0; // Snap back to idle state

      // Reset dimensional boundaries back to standard run pacing dimensions!
      SetScale(m_dScale);
    }
  }
  else
  {
    // Manage regular visual movement animation pacing
    if (!m_bIsMoving)
    {
      m_iCurFrame = 0;    // Set to first frame (standing idle)
      m_iFrameDelay = -1; // Turn off automatic frame timer advancement
    }
    else
    {
      m_iFrameDelay = 3;  // Turn back on the frame ticker
    }
  }

  // Call base class update to execute motion & bounds enforcement logic
  return Sprite::Update();
}

void Player::SetScale(double dScale)
{
  m_dScale = dScale;

  // Instantly recalculate and enforce physical world collision bounding rect size
  RECT rect = GetPosition();
  rect.right = rect.left + (int)(GetWidth() * m_dScale);
  rect.bottom = rect.top + (int)(GetHeight() * m_dScale);
  SetPosition(rect);
}

void Player::CalcCollisionRect()
{
  // Perform precise, narrowed collision calculation for smoother sliding against walls!
  // We shrink left/right by 25% and top/bottom by 15% to ignore wide transparent padding.
  int width = m_rcPosition.right - m_rcPosition.left;
  int height = m_rcPosition.bottom - m_rcPosition.top;

  int iXShrink = -(int)(width * 0.25);  // Shrink sides substantially
  int iYShrink = -(int)(height * 0.15); // Shrink vertical bounds to keep feet/head safe

  CopyRect(&m_rcCollision, &m_rcPosition);
  InflateRect(&m_rcCollision, iXShrink, iYShrink);
}

void Player::SetAttackBitmaps(Bitmap* pBitmaps[4])
{
  for (int i = 0; i < 4; i++)
  {
    m_pAttackBitmaps[i] = pBitmaps[i];
  }
}

void Player::ExecuteAttackDamage()
{
  extern ProceduralMapGeneration* _pMap;
  if (_pMap == NULL) return;

  // 1. Retrieve center coordinate of the actual physical player hitbox
  RECT rc = GetCollision();
  int cx = rc.left + (rc.right - rc.left) / 2;
  int cy = rc.top + (rc.bottom - rc.top) / 2;

  // 2. Calculate attack vector endpoint. 
  // TILE_SIZE * 3 / 4 (48px) offers clean coverage into the immediately adjacent tile!
  int reach = 48; 
  int targetX = cx;
  int targetY = cy;

  if (m_iCurrentDir == PDIR_DOWN)
    targetY += reach;
  else if (m_iCurrentDir == PDIR_LEFT)
    targetX -= reach;
  else if (m_iCurrentDir == PDIR_RIGHT)
    targetX += reach;
  else if (m_iCurrentDir == PDIR_UP)
    targetY -= reach;

  // 3. Project mathematical point back into map tile space
  int r = targetY / TILE_SIZE;
  int c = targetX / TILE_SIZE;

  // 4. Verify that an active mineral exists at the targeted coordinate
  int tileVal = _pMap->GetTile(r, c);
  if (tileVal >= 1 && tileVal <= 5)
  {
    // Center the text over the specific Ore tile being struck
    int textX = c * TILE_SIZE + (TILE_SIZE / 4);
    int textY = r * TILE_SIZE + (TILE_SIZE / 4);

    // Extern declaration for the safe global text factory
    extern void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color);

    // Map the tile value to its corresponding display name
    const TCHAR* szOreName = TEXT("Ore");
    switch (tileVal)
    {
      case 1: szOreName = TEXT("Iron"); break;
      case 2: szOreName = TEXT("Amethyst"); break;
      case 3: szOreName = TEXT("Gold"); break;
      case 4: szOreName = TEXT("Ruby"); break;
      case 5: szOreName = TEXT("Obsidian"); break;
    }

    // 5. Inflict persistent damage and assess mineral status!
    _pMap->DamageOre(r, c, 1);
    TCHAR szBuffer[32];
    wsprintf(szBuffer, TEXT("+1 %s"), szOreName);
    AddFloatingText(textX, textY, szBuffer, RGB(0, 255, 0)); // Green for collection success
  }
}

