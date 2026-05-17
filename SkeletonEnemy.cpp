#include "SkeletonEnemy.h"
#include "DebugSystem.h"
#include "Player.h"
#include <cstdlib>

extern void SpawnSkeletonProjectile(POINT ptStart, POINT ptTarget);

SkeletonEnemy::SkeletonEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds)
  : Enemy(pBitmap, ptPosition, rcBounds), m_iFireCooldown(0)
{
  m_iHealth = 2;
  m_iMoveSpeed = 3;
  m_iDetectRange = 100;
  m_iForgetRange = 100;
  m_iAttackRange = 360;
  SetState(ENEMY_PATROL);
  DebugLogFormat(TEXT("Skeleton created this=0x%p pos=(%ld,%ld)"), this, ptPosition.x, ptPosition.y);
}

SkeletonEnemy::~SkeletonEnemy()
{
  DebugLogFormat(TEXT("Skeleton destroyed this=0x%p"), this);
}

void SkeletonEnemy::UpdateIdle(Player* pPlayer)
{
  SetState(ENEMY_PATROL);
}

void SkeletonEnemy::UpdatePatrol(Player* pPlayer)
{
  if (IsPlayerDetected(pPlayer))
  {
    StopMoving();
    SetState(ENEMY_ATTACK);
    return;
  }

  Enemy::UpdatePatrol(pPlayer);
}

void SkeletonEnemy::UpdateChase(Player* pPlayer)
{
  if (IsPlayerDetected(pPlayer))
    SetState(ENEMY_ATTACK);
  else
    SetState(ENEMY_PATROL);
}

void SkeletonEnemy::UpdateForget(Player* pPlayer)
{
  SetState(ENEMY_PATROL);
}

void SkeletonEnemy::UpdateAttack(Player* pPlayer)
{
  if (!IsPlayerDetected(pPlayer))
  {
    SetState(ENEMY_PATROL);
    return;
  }

  StopMoving();

  if (m_iFireCooldown > 0)
    m_iFireCooldown--;

  if (m_iFireCooldown <= 0)
  {
    OnAttack(pPlayer);
    m_iFireCooldown = 24;
  }
}

void SkeletonEnemy::OnAttack(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  POINT ptStart = GetCenter();
  POINT ptTarget = GetPlayerCenter(pPlayer);
  FireAtPlayer(pPlayer);

  if ((DebugGetFrameNumber() % 15) == 0)
    DebugLogFormat(TEXT("Skeleton fire this=0x%p start=(%ld,%ld) target=(%ld,%ld)"),
      this, ptStart.x, ptStart.y, ptTarget.x, ptTarget.y);
}

void SkeletonEnemy::FireAtPlayer(Player* pPlayer)
{
  if (pPlayer == NULL)
    return;

  POINT ptStart = GetCenter();
  POINT ptTarget = GetPlayerCenter(pPlayer);
  SpawnSkeletonProjectile(ptStart, ptTarget);
}
