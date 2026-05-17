#include "TankGolemEnemy.h"
#include "DebugSystem.h"
#include "Player.h"
#include <cstdlib>

TankGolemEnemy::TankGolemEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds)
  : Enemy(pBitmap, ptPosition, rcBounds), m_ptLastPosition(ptPosition),
    m_iStuckTicks(0), m_iLastAnimationRow(2), m_bHasPatrolSample(FALSE),
    m_pTileBlockedFn(NULL), m_pPathContext(NULL), m_iPathRows(0),
    m_iPathCols(0), m_iTileSize(64), m_iPathIndex(0),
    m_iPathRefreshTicks(0), m_bPathAttempted(FALSE)
{
  m_iHealth = 5;
  m_iMoveSpeed = 2;
  m_iDetectRange = 800;
  m_iForgetRange = 800;
  m_iAttackRange = 56;
  SetNumFrames(3);
  SetFrameRows(4);
  SetFrameDelay(6);
  SetScale(2.0);
  AlignToCurrentTile();
  PickRandomDirection();
  SetState(ENEMY_PATROL);
  LogState(TEXT("created"));
}

TankGolemEnemy::~TankGolemEnemy()
{
  LogState(TEXT("destroyed"));
}

SPRITEACTION TankGolemEnemy::Update()
{
  SPRITEACTION saAction = Enemy::Update();
  HoldStoppedAnimationFrame();
  return saAction;
}

void TankGolemEnemy::UpdateIdle(Player* pPlayer)
{
  SetState(ENEMY_PATROL);
}

void TankGolemEnemy::UpdatePatrol(Player* pPlayer)
{
  if (IsPlayerDetected(pPlayer))
  {
    LogState(TEXT("player detected, chase"));
    ClearPath();
    SetState(ENEMY_CHASE);
    return;
  }

  RECT rcPosition = GetPosition();
  POINT ptCurrent = { rcPosition.left, rcPosition.top };
  POINT ptVelocity = GetVelocity();
  if (!m_bHasPatrolSample)
  {
    m_bHasPatrolSample = TRUE;
    m_ptLastPosition = ptCurrent;
    MoveInCurrentDirection();
    return;
  }

  BOOL bWasMoving = (ptVelocity.x != 0 || ptVelocity.y != 0);
  BOOL bDidNotMove = (abs(ptCurrent.x - m_ptLastPosition.x) < 1 &&
    abs(ptCurrent.y - m_ptLastPosition.y) < 1);
  BOOL bBlocked = ((ptVelocity.x == 0 && ptVelocity.y == 0) ||
    (bWasMoving && bDidNotMove));

  if (bBlocked)
  {
    m_iStuckTicks++;
  }
  else
  {
    m_iStuckTicks = 0;
  }
  m_ptLastPosition = ptCurrent;

  if (m_iStuckTicks > 0)
  {
    m_iStuckTicks = 0;
    PickDifferentDirection();
    LogState(TEXT("blocked, changed direction"));
  }

  MoveInCurrentDirection();
}

void TankGolemEnemy::UpdateChase(Player* pPlayer)
{
  if (pPlayer == NULL || DistanceToPlayer(pPlayer) > m_iForgetRange)
  {
    LogState(TEXT("player out of range, patrol"));
    m_bHasPatrolSample = FALSE;
    ClearPath();
    SetState(ENEMY_PATROL);
    return;
  }

  if (IsPlayerInAttackRange(pPlayer) && m_iAttackCooldown <= 0)
  {
    ClearPath();
    SetState(ENEMY_ATTACK);
    return;
  }

  if (m_pTileBlockedFn == NULL)
  {
    Enemy::UpdateChase(pPlayer);
    return;
  }

  if (m_iPathRefreshTicks > 0)
    m_iPathRefreshTicks--;

  if (m_iPathRefreshTicks <= 0 || !m_bPathAttempted ||
    (!m_vPath.empty() && m_iPathIndex >= (int)m_vPath.size()))
    UpdatePathToPlayer(pPlayer);

  if (!m_vPath.empty() && m_iPathIndex < (int)m_vPath.size())
    MoveAlongPath();
  else
  {
    if ((DebugGetFrameNumber() % 30) == 0)
      LogState(TEXT("path unavailable, continuing patrol movement"));
    MoveInCurrentDirection();
  }
}

void TankGolemEnemy::UpdateForget(Player* pPlayer)
{
  PickRandomDirection();
  m_bHasPatrolSample = FALSE;
  ClearPath();
  SetState(ENEMY_PATROL);
}

void TankGolemEnemy::OnAttack(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  if ((DebugGetFrameNumber() % 30) == 0)
    LogState(TEXT("attack"));
}

void TankGolemEnemy::OnTouchPlayer(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  if ((DebugGetFrameNumber() % 30) == 0)
    LogState(TEXT("touch player"));
}

void TankGolemEnemy::PickRandomDirection()
{
  int iDirection = rand() % 4;
  m_ptDirection.x = 0;
  m_ptDirection.y = 0;

  switch (iDirection)
  {
    case 0:
      m_ptDirection.x = 1;
      break;
    case 1:
      m_ptDirection.x = -1;
      break;
    case 2:
      m_ptDirection.y = 1;
      break;
    default:
      m_ptDirection.y = -1;
      break;
  }
}

void TankGolemEnemy::PickDifferentDirection()
{
  POINT ptOldDirection = m_ptDirection;

  for (int i = 0; i < 8; i++)
  {
    PickRandomDirection();
    if (m_ptDirection.x != ptOldDirection.x ||
      m_ptDirection.y != ptOldDirection.y)
      return;
  }

  if (ptOldDirection.x != 0)
  {
    m_ptDirection.x = 0;
    m_ptDirection.y = (rand() % 2 == 0) ? 1 : -1;
  }
  else
  {
    m_ptDirection.x = (rand() % 2 == 0) ? 1 : -1;
    m_ptDirection.y = 0;
  }
}

void TankGolemEnemy::MoveInCurrentDirection()
{
  SetVelocity(m_ptDirection.x * m_iMoveSpeed,
    m_ptDirection.y * m_iMoveSpeed);
  UpdateAnimationDirection();
}

BOOL TankGolemEnemy::UpdatePathToPlayer(Player* pPlayer)
{
  if (pPlayer == NULL || m_pTileBlockedFn == NULL)
    return FALSE;

  RECT rcEnemy = GetPosition();
  RECT rcPlayer = pPlayer->GetPosition();
  int enemyCenterX = rcEnemy.left + (rcEnemy.right - rcEnemy.left) / 2;
  int enemyCenterY = rcEnemy.top + (rcEnemy.bottom - rcEnemy.top) / 2;
  int playerCenterX = rcPlayer.left + (rcPlayer.right - rcPlayer.left) / 2;
  int playerCenterY = rcPlayer.top + (rcPlayer.bottom - rcPlayer.top) / 2;

  int startCol = enemyCenterX / m_iTileSize;
  int startRow = enemyCenterY / m_iTileSize;
  int goalCol = playerCenterX / m_iTileSize;
  int goalRow = playerCenterY / m_iTileSize;
  POINT ptStart = { startCol, startRow };
  POINT ptGoal = { goalCol, goalRow };

  m_iPathRefreshTicks = 12;
  m_iPathIndex = 0;
  m_bPathAttempted = TRUE;

  if (!FindNearestOpenTile(startRow, startCol, 1, ptStart) ||
    !FindNearestOpenTile(goalRow, goalCol, 3, ptGoal))
  {
    m_vPath.clear();
    DebugLogFormat(TEXT("TankGolem path failed this=0x%p start=(%d,%d) goal=(%d,%d) reason=no-open-endpoint"),
      this, startRow, startCol, goalRow, goalCol);
    return FALSE;
  }

  BOOL bFound = AStarPathfinder::FindPath(ptStart.y, ptStart.x, ptGoal.y, ptGoal.x,
    m_iPathRows, m_iPathCols, m_pTileBlockedFn, m_pPathContext, m_vPath);

  if (bFound)
  {
    DebugLogFormat(TEXT("TankGolem path found this=0x%p start=(%d,%d) goal=(%d,%d) nodes=%u"),
      this, ptStart.y, ptStart.x, ptGoal.y, ptGoal.x,
      (unsigned int)m_vPath.size());
  }
  else
  {
    m_vPath.clear();
    DebugLogFormat(TEXT("TankGolem path failed this=0x%p start=(%d,%d) goal=(%d,%d)"),
      this, ptStart.y, ptStart.x, ptGoal.y, ptGoal.x);
  }

  return bFound;
}

BOOL TankGolemEnemy::FindNearestOpenTile(int row, int col, int radius,
  POINT& ptTile)
{
  if (m_pTileBlockedFn == NULL)
    return FALSE;

  for (int r = 0; r <= radius; r++)
  {
    for (int dy = -r; dy <= r; dy++)
    {
      for (int dx = -r; dx <= r; dx++)
      {
        if (abs(dx) != r && abs(dy) != r)
          continue;

        int testRow = row + dy;
        int testCol = col + dx;
        if (testRow < 0 || testRow >= m_iPathRows ||
          testCol < 0 || testCol >= m_iPathCols)
          continue;

        if (!m_pTileBlockedFn(testRow, testCol, m_pPathContext))
        {
          ptTile.x = testCol;
          ptTile.y = testRow;
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

void TankGolemEnemy::MoveAlongPath()
{
  if (m_iPathIndex >= (int)m_vPath.size())
  {
    StopMoving();
    return;
  }

  POINT ptTile = m_vPath[m_iPathIndex];
  POINT ptCenter = GetCenter();
  int targetX = ptTile.x * m_iTileSize + m_iTileSize / 2;
  int targetY = ptTile.y * m_iTileSize + m_iTileSize / 2;
  int dx = targetX - ptCenter.x;
  int dy = targetY - ptCenter.y;

  if (abs(dx) <= m_iMoveSpeed && abs(dy) <= m_iMoveSpeed)
  {
    SetCenterPosition(targetX, targetY);
    ptCenter.x = targetX;
    ptCenter.y = targetY;
    m_iPathIndex++;
    if (m_iPathIndex >= (int)m_vPath.size())
    {
      StopMoving();
      return;
    }

    ptTile = m_vPath[m_iPathIndex];
    targetX = ptTile.x * m_iTileSize + m_iTileSize / 2;
    targetY = ptTile.y * m_iTileSize + m_iTileSize / 2;
    dx = targetX - ptCenter.x;
    dy = targetY - ptCenter.y;
  }

  if (abs(dx) > m_iMoveSpeed && abs(dy) > m_iMoveSpeed)
  {
    if (abs(dx) < abs(dy))
      SetVelocity((dx > 0) ? m_iMoveSpeed : -m_iMoveSpeed, 0);
    else
      SetVelocity(0, (dy > 0) ? m_iMoveSpeed : -m_iMoveSpeed);
  }
  else if (abs(dx) > m_iMoveSpeed)
    SetVelocity((dx > 0) ? m_iMoveSpeed : -m_iMoveSpeed, 0);
  else if (abs(dy) > m_iMoveSpeed)
    SetVelocity(0, (dy > 0) ? m_iMoveSpeed : -m_iMoveSpeed);
  else
    SetVelocity(0, 0);

  UpdateAnimationDirection();
}

void TankGolemEnemy::SetCenterPosition(int x, int y)
{
  RECT rc = GetPosition();
  int width = rc.right - rc.left;
  int height = rc.bottom - rc.top;
  POINT ptPosition = { x - width / 2, y - height / 2 };
  SetPosition(ptPosition);
}

void TankGolemEnemy::AlignToCurrentTile()
{
  RECT rc = GetPosition();
  int centerX = rc.left + (rc.right - rc.left) / 2;
  int centerY = rc.top + (rc.bottom - rc.top) / 2;
  int col = centerX / m_iTileSize;
  int row = centerY / m_iTileSize;

  SetCenterPosition(col * m_iTileSize + m_iTileSize / 2,
    row * m_iTileSize + m_iTileSize / 2);
}

void TankGolemEnemy::ClearPath()
{
  m_vPath.clear();
  m_iPathIndex = 0;
  m_iPathRefreshTicks = 0;
  m_bPathAttempted = FALSE;
}

void TankGolemEnemy::LogState(const TCHAR* szMessage)
{
  RECT rc = GetPosition();
  DebugLogFormat(TEXT("TankGolem %s this=0x%p state=%d health=%d pos=(%ld,%ld) dir=(%ld,%ld)"),
    szMessage, this, m_eState, m_iHealth, rc.left, rc.top,
    m_ptDirection.x, m_ptDirection.y);
}

void TankGolemEnemy::UpdateAnimationDirection()
{
  POINT ptVelocity = GetVelocity();

  if (ptVelocity.y < 0)
    m_iLastAnimationRow = 0;
  else if (ptVelocity.x > 0)
    m_iLastAnimationRow = 1;
  else if (ptVelocity.y > 0)
    m_iLastAnimationRow = 2;
  else if (ptVelocity.x < 0)
    m_iLastAnimationRow = 3;
  else
    return;

  SetFrameRow(m_iLastAnimationRow);
}

void TankGolemEnemy::HoldStoppedAnimationFrame()
{
  POINT ptVelocity = GetVelocity();

  if (ptVelocity.x != 0 || ptVelocity.y != 0)
    return;

  SetFrameRow(m_iLastAnimationRow);
  m_iCurFrame = 1;
}

void TankGolemEnemy::ConfigurePathfinding(int iRows, int iCols, int iTileSize,
  AStarTileBlockedFn pTileBlockedFn, void* pContext)
{
  m_iPathRows = iRows;
  m_iPathCols = iCols;
  m_iTileSize = iTileSize;
  m_pTileBlockedFn = pTileBlockedFn;
  m_pPathContext = pContext;
  ClearPath();
}
