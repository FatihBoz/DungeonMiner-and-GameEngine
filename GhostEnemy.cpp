#include "GhostEnemy.h"
#include "Player.h"
#include "ProceduralMapGeneration.h"
#include <cstdlib>

static const int GHOST_TILE_SIZE = 64;

GhostEnemy::GhostEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds)
  : Enemy(pBitmap, ptPosition, rcBounds), m_ptHome(ptPosition)
{
  m_iHealth = 1;
  m_iMoveSpeed = 4;
  m_iDetectRange = -1;
  m_iForgetRange = -1;
  m_iAttackRange = -1;
  SetScale(2.0);
  RefreshRandomPatrolRoute();
}

GhostEnemy::~GhostEnemy()
{
}

void GhostEnemy::UpdatePatrol(Player* pPlayer)
{
  if (m_vPatrolRoute.empty())
    RefreshRandomPatrolRoute();

  POINT ptTarget = m_vPatrolRoute[m_iPatrolIndex];
  POINT ptCenter = GetCenter();
  int dx = ptTarget.x - ptCenter.x;
  int dy = ptTarget.y - ptCenter.y;

  if ((dx * dx + dy * dy) < 1024)
  {
    RefreshRandomPatrolRoute();
    StopMoving();
    return;
  }

  MoveToward(m_vPatrolRoute[m_iPatrolIndex], m_iMoveSpeed);
  POINT ptVelocity = GetVelocity();
  if (ptVelocity.x < 0)
    FlipHorizontally(TRUE);
  else if (ptVelocity.x > 0)
    FlipHorizontally(FALSE);
}

void GhostEnemy::UpdateForget(Player* pPlayer)
{
  RefreshRandomPatrolRoute();
  SetState(ENEMY_PATROL);
}

void GhostEnemy::OnAttack(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  extern void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color);

  RECT rcPlayer = pPlayer->GetPosition();
  int x = rcPlayer.left + (rcPlayer.right - rcPlayer.left) / 2 - 28;
  int y = rcPlayer.top - 18;
  AddFloatingText(x, y, TEXT("GHOST!"), RGB(170, 220, 255));
}

void GhostEnemy::OnTouchPlayer(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  extern void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color);

  RECT rcPlayer = pPlayer->GetPosition();
  int x = rcPlayer.left + (rcPlayer.right - rcPlayer.left) / 2 - 36;
  int y = rcPlayer.top - 32;
  AddFloatingText(x, y, TEXT("-1 GHOST"), RGB(170, 220, 255));
}

POINT GhostEnemy::CreateRandomPointNearHome() const
{
  extern ProceduralMapGeneration* _pMap;

  POINT pt;
  int range = 6 * GHOST_TILE_SIZE;
  int maxX = (_pMap != NULL) ? (_pMap->GetCols() * GHOST_TILE_SIZE - GHOST_TILE_SIZE) : m_rcBounds.right;
  int maxY = (_pMap != NULL) ? (_pMap->GetRows() * GHOST_TILE_SIZE - GHOST_TILE_SIZE) : m_rcBounds.bottom;

  pt.x = m_ptHome.x + (rand() % (range * 2 + 1)) - range;
  pt.y = m_ptHome.y + (rand() % (range * 2 + 1)) - range;

  if (pt.x < 0)
    pt.x = 0;
  if (pt.y < 0)
    pt.y = 0;
  if (pt.x > maxX)
    pt.x = maxX;
  if (pt.y > maxY)
    pt.y = maxY;

  return pt;
}

void GhostEnemy::RefreshRandomPatrolRoute()
{
  std::vector<POINT> vRoute;
  vRoute.push_back(CreateRandomPointNearHome());
  SetPatrolRoute(vRoute);
}
