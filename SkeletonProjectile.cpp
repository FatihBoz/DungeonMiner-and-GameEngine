#include "SkeletonProjectile.h"
#include "Player.h"
#include "SkeletonProjectileMath.h"

extern Player* _pPlayer;
extern BOOL MapCollision(Sprite* pSprite);

SkeletonProjectile::SkeletonProjectile(Bitmap* pBitmap, POINT ptPosition,
  POINT ptTarget, RECT& rcBounds)
  : Sprite(pBitmap, rcBounds, BA_DIE),
    m_ptTarget(ptTarget), m_iSpeed(10), m_iSpinAngle(0)
{
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  SetPosition(ptPosition.x - width / 2, ptPosition.y - height / 2);
  SetVelocity(ComputeStraightLineStep(ptPosition, ptTarget, m_iSpeed));
  SetNumFrames(1);
  SetFrameDelay(-1);
  SetScale(2.0);
  SetBoundsAction(BA_DIE);
}

SkeletonProjectile::~SkeletonProjectile()
{
}

BOOL SkeletonProjectile::HitsPlayer()
{
  if (_pPlayer == NULL)
    return FALSE;

  return TestCollision(_pPlayer);
}

SPRITEACTION SkeletonProjectile::Update()
{
  m_iSpinAngle = (m_iSpinAngle + 25) % 360;

  POINT ptVelocity = GetVelocity();
  if (ptVelocity.x == 0 && ptVelocity.y == 0)
    return SA_KILL;

  SPRITEACTION sa = Sprite::Update();
  if (sa & SA_KILL)
    return sa;

  if (MapCollision(this))
    return SA_KILL;

  if (HitsPlayer())
  {
    _pPlayer->Damage(1);
    return SA_KILL;
  }

  return SA_NONE;
}

void SkeletonProjectile::Draw(HDC hDC)
{
  if (m_pBitmap == NULL || m_bHidden)
    return;

  int fullWidth = m_rcPosition.right - m_rcPosition.left;
  int fullHeight = m_rcPosition.bottom - m_rcPosition.top;
  m_pBitmap->DrawScaledRotated(hDC, m_rcPosition.left, m_rcPosition.top,
    fullWidth, fullHeight, (double)m_iSpinAngle, TRUE);
}
