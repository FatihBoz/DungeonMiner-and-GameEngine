#pragma once

#include "Sprite.h"

class SkeletonProjectile : public Sprite
{
private:
  POINT m_ptTarget;
  int   m_iSpeed;
  int   m_iSpinAngle;

  BOOL  HitsPlayer();

public:
  SkeletonProjectile(Bitmap* pBitmap, POINT ptPosition, POINT ptTarget,
    RECT& rcBounds);
  virtual ~SkeletonProjectile();

  virtual SPRITEACTION Update() override;
  virtual void Draw(HDC hDC) override;
};
