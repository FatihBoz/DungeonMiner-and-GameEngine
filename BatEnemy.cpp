#include "BatEnemy.h"
#include "Player.h"
#include "ProceduralMapGeneration.h"
#include <cstdlib>

static const int BAT_TILE_SIZE = 64;
static const int BAT_ANIM_DOWN = 0;
static const int BAT_ANIM_RIGHT = 1;
static const int BAT_ANIM_UP = 2;
static const int BAT_ANIM_LEFT = 3;

BatEnemy::BatEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds)
  : Enemy(pBitmap, ptPosition, rcBounds), m_ptHome(ptPosition),
    m_ptLastPosition(ptPosition), m_iStuckTicks(0)
{
  m_iHealth = 2;
  m_iMoveSpeed = 6;
  m_iDetectRange = -1;
  m_iForgetRange = -1;
  m_iAttackRange = -1;
  SetNumFrames(4);
  SetFrameRows(4);
  SetFrameRow(BAT_ANIM_DOWN);
  SetFrameDelay(4);
  SetScale(2.0);
  RefreshRandomPatrolRoute();
}

BatEnemy::~BatEnemy()
{
}

void BatEnemy::UpdatePatrol(Player* pPlayer)
{
  RECT rcPosition = GetPosition();
  POINT ptCurrent = { rcPosition.left, rcPosition.top };
  POINT ptVelocity = GetVelocity();
  if ((ptVelocity.x != 0 || ptVelocity.y != 0) &&
    abs(ptCurrent.x - m_ptLastPosition.x) < 2 &&
    abs(ptCurrent.y - m_ptLastPosition.y) < 2)
  {
    m_iStuckTicks++;
  }
  else
  {
    m_iStuckTicks = 0;
  }
  m_ptLastPosition = ptCurrent;

  if (m_vPatrolRoute.empty() || m_iStuckTicks > 8)
  {
    m_iStuckTicks = 0;
    RefreshRandomPatrolRoute();
  }

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
  UpdateAnimationDirection();
}

void BatEnemy::UpdateForget(Player* pPlayer)
{
  RefreshRandomPatrolRoute();
  SetState(ENEMY_PATROL);
}

void BatEnemy::OnAttack(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  extern void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color);

  RECT rcPlayer = pPlayer->GetPosition();
  int x = rcPlayer.left + (rcPlayer.right - rcPlayer.left) / 2 - 20;
  int y = rcPlayer.top - 18;
  AddFloatingText(x, y, TEXT("BAT!"), RGB(255, 80, 80));
}

void BatEnemy::OnTouchPlayer(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  extern void AddFloatingText(int x, int y, const TCHAR* szText, COLORREF color);

  RECT rcPlayer = pPlayer->GetPosition();
  int x = rcPlayer.left + (rcPlayer.right - rcPlayer.left) / 2 - 28;
  int y = rcPlayer.top - 24;
  AddFloatingText(x, y, TEXT("-1 BAT"), RGB(255, 70, 70));
}

POINT BatEnemy::CreateRandomPointNearHome() const
{
  extern ProceduralMapGeneration* _pMap;

  POINT pt;
  int range = 5 * BAT_TILE_SIZE;
  int maxX = (_pMap != NULL) ? (_pMap->GetCols() * BAT_TILE_SIZE - BAT_TILE_SIZE) : m_rcBounds.right;
  int maxY = (_pMap != NULL) ? (_pMap->GetRows() * BAT_TILE_SIZE - BAT_TILE_SIZE) : m_rcBounds.bottom;

  for (int attempt = 0; attempt < 20; attempt++)
  {
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

    if (_pMap == NULL)
      return pt;

    int row = pt.y / BAT_TILE_SIZE;
    int col = pt.x / BAT_TILE_SIZE;
    if (IsOpenFloorTile(row, col))
      return pt;
  }

  pt = m_ptHome;

  return pt;
}

BOOL BatEnemy::IsOpenFloorTile(int row, int col) const
{
  extern ProceduralMapGeneration* _pMap;

  if (_pMap == NULL)
    return TRUE;

  if (_pMap->GetTile(row, col) != 0)
    return FALSE;

  if (_pMap->GetTile(row - 1, col) == 100 ||
    _pMap->GetTile(row + 1, col) == 100 ||
    _pMap->GetTile(row, col - 1) == 100 ||
    _pMap->GetTile(row, col + 1) == 100)
  {
    return FALSE;
  }

  return TRUE;
}

void BatEnemy::RefreshRandomPatrolRoute()
{
  std::vector<POINT> vRoute;
  vRoute.push_back(CreateRandomPointNearHome());
  SetPatrolRoute(vRoute);
}

void BatEnemy::UpdateAnimationDirection()
{
  POINT ptVelocity = GetVelocity();

  if (ptVelocity.x == 0 && ptVelocity.y == 0)
    return;

  if (abs(ptVelocity.x) > abs(ptVelocity.y))
  {
    SetFrameRow((ptVelocity.x > 0) ? BAT_ANIM_RIGHT : BAT_ANIM_LEFT);
  }
  else
  {
    SetFrameRow((ptVelocity.y > 0) ? BAT_ANIM_DOWN : BAT_ANIM_UP);
  }
}
