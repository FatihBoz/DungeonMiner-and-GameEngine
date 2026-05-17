#pragma once

#include "Sprite.h"

class SkeletonProjectile : public Sprite
{
private:
  POINT m_ptTarget;
  int   m_iSpeed;

  BOOL  HitsPlayer();

public:
  SkeletonProjectile(Bitmap* pBitmap, POINT ptPosition, POINT ptTarget,
    RECT& rcBounds);
  virtual ~SkeletonProjectile();

  virtual SPRITEACTION Update() override;
};
