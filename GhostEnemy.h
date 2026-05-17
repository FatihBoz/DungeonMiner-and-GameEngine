#pragma once

#include "Enemy.h"

class GhostEnemy : public Enemy
{
private:
  POINT m_ptHome;
  POINT CreateRandomPointNearHome() const;
  void  RefreshRandomPatrolRoute();

protected:
  virtual void UpdatePatrol(Player* pPlayer) override;
  virtual void UpdateForget(Player* pPlayer) override;
  virtual void OnAttack(Player* pPlayer) override;
  virtual void OnTouchPlayer(Player* pPlayer) override;

public:
  GhostEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds);
  virtual ~GhostEnemy();
};
