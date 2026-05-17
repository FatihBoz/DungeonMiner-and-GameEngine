#pragma once

#include "Enemy.h"
#include "AStarPathfinder.h"

class TankGolemEnemy : public Enemy
{
private:
  POINT m_ptDirection;
  POINT m_ptLastPosition;
  int   m_iStuckTicks;
  BOOL  m_bHasPatrolSample;
  AStarTileBlockedFn m_pTileBlockedFn;
  void* m_pPathContext;
  int   m_iPathRows;
  int   m_iPathCols;
  int   m_iTileSize;
  int   m_iPathIndex;
  int   m_iPathRefreshTicks;
  BOOL  m_bPathAttempted;
  std::vector<POINT> m_vPath;

  void  PickRandomDirection();
  void  PickDifferentDirection();
  void  MoveInCurrentDirection();
  BOOL  UpdatePathToPlayer(Player* pPlayer);
  BOOL  FindNearestOpenTile(int row, int col, int radius, POINT& ptTile);
  void  MoveAlongPath();
  void  SetCenterPosition(int x, int y);
  void  AlignToCurrentTile();
  void  ClearPath();
  void  LogState(const TCHAR* szMessage);
  void  UpdateAnimationDirection();

protected:
  virtual void UpdateIdle(Player* pPlayer) override;
  virtual void UpdatePatrol(Player* pPlayer) override;
  virtual void UpdateChase(Player* pPlayer) override;
  virtual void UpdateForget(Player* pPlayer) override;
  virtual void OnAttack(Player* pPlayer) override;
  virtual void OnTouchPlayer(Player* pPlayer) override;

public:
  TankGolemEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds);
  virtual ~TankGolemEnemy();

  void ConfigurePathfinding(int iRows, int iCols, int iTileSize,
    AStarTileBlockedFn pTileBlockedFn, void* pContext);
};
