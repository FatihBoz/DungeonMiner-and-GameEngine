#pragma once

#include "Enemy.h"

class SkeletonEnemy : public Enemy
{
private:
  int   m_iFireCooldown;
  int   m_iLastAnimationRow;

  void  FireAtPlayer(Player* pPlayer);
  void  UpdateAnimationDirection();
  void  HoldStoppedAnimationFrame();

protected:
  virtual void UpdateIdle(Player* pPlayer) override;
  virtual void UpdatePatrol(Player* pPlayer) override;
  virtual void UpdateChase(Player* pPlayer) override;
  virtual void UpdateForget(Player* pPlayer) override;
  virtual void UpdateAttack(Player* pPlayer) override;
  virtual void OnAttack(Player* pPlayer) override;

public:
  SkeletonEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds);
  virtual ~SkeletonEnemy();

  virtual SPRITEACTION Update() override;
};
