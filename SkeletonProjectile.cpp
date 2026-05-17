#include "SkeletonProjectile.h"
#include "Player.h"
#include "SkeletonProjectileMath.h"

extern Player* _pPlayer;
extern BOOL MapCollision(Sprite* pSprite);

SkeletonProjectile::SkeletonProjectile(Bitmap* pBitmap, POINT ptPosition,
  POINT ptTarget, RECT& rcBounds)
  : Sprite(pBitmap, rcBounds, BA_DIE),
    m_ptTarget(ptTarget), m_iSpeed(10)
{
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  SetPosition(ptPosition.x - width / 2, ptPosition.y - height / 2);
  SetNumFrames(1);
  SetFrameDelay(-1);
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
  POINT ptFrom = { m_rcPosition.left + (m_rcPosition.right - m_rcPosition.left) / 2,
    m_rcPosition.top + (m_rcPosition.bottom - m_rcPosition.top) / 2 };
  POINT ptStep = ComputeStraightLineStep(ptFrom, m_ptTarget, m_iSpeed);
  SetVelocity(ptStep);

  SPRITEACTION sa = Sprite::Update();
  if (sa & SA_KILL)
    return sa;

  if (MapCollision(this) || HitsPlayer())
    return SA_KILL;

  return SA_NONE;
}
