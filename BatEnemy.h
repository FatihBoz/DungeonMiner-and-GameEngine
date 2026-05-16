#pragma once

#include "Enemy.h"

class BatEnemy : public Enemy
{
private:
  POINT m_ptHome;
  POINT m_ptLastPosition;
  int   m_iStuckTicks;

  BOOL  IsOpenFloorTile(int row, int col) const;
  POINT CreateRandomPointNearHome() const;
  void  RefreshRandomPatrolRoute();

protected:
  virtual void UpdatePatrol(Player* pPlayer) override;
  virtual void UpdateForget(Player* pPlayer) override;
  virtual void OnAttack(Player* pPlayer) override;
  virtual void OnTouchPlayer(Player* pPlayer) override;

public:
  BatEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds);
  virtual ~BatEnemy();
};
