#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds)
  : Sprite(pBitmap, rcBounds, BA_STOP), m_eState(ENEMY_IDLE), m_iHealth(3),
    m_iMoveSpeed(3), m_iDetectRange(280), m_iForgetRange(420),
    m_iAttackRange(48), m_iStateTicks(0), m_iForgetTicks(0),
    m_iAttackTicks(0), m_iAttackCooldown(0), m_iTouchCooldown(0),
    m_iPatrolIndex(0)
{
  SetPosition(ptPosition);
  SetNumFrames(1);
  SetFrameDelay(-1);
}

Enemy::~Enemy()
{
}

SPRITEACTION Enemy::Update()
{
  extern Player* _pPlayer;

  if (m_bDying)
    return SA_KILL;

  if (IsDead())
    return SA_KILL;

  m_iStateTicks++;
  if (m_iAttackCooldown > 0)
    m_iAttackCooldown--;
  if (m_iTouchCooldown > 0)
    m_iTouchCooldown--;

  switch (m_eState)
  {
    case ENEMY_IDLE:
      UpdateIdle(_pPlayer);
      break;
    case ENEMY_PATROL:
      UpdatePatrol(_pPlayer);
      break;
    case ENEMY_CHASE:
      UpdateChase(_pPlayer);
      break;
    case ENEMY_FORGET:
      UpdateForget(_pPlayer);
      break;
    case ENEMY_ATTACK:
      UpdateAttack(_pPlayer);
      break;
  }

  return Sprite::Update();
}

void Enemy::SetPatrolRoute(const std::vector<POINT>& vRoute)
{
  m_vPatrolRoute = vRoute;
  m_iPatrolIndex = 0;
}

void Enemy::Damage(int iDamage)
{
  m_iHealth -= iDamage;
  if (m_iHealth <= 0)
    Kill();
}

void Enemy::TouchPlayer(Player* pPlayer)
{
  if (pPlayer == NULL || m_iTouchCooldown > 0)
    return;

  pPlayer->Damage(1);
  OnTouchPlayer(pPlayer);
  m_iTouchCooldown = 30;
}

void Enemy::SetState(EnemyAIState eState)
{
  if (m_eState == eState)
    return;

  m_eState = eState;
  m_iStateTicks = 0;
  OnEnterState(eState);
}

int Enemy::DistanceToPlayer(Player* pPlayer) const
{
  if (pPlayer == NULL)
    return 999999;

  POINT ptEnemy = GetCenter();
  POINT ptPlayer = GetPlayerCenter(pPlayer);
  int dx = ptPlayer.x - ptEnemy.x;
  int dy = ptPlayer.y - ptEnemy.y;
  return (int)std::sqrt((double)(dx * dx + dy * dy));
}

POINT Enemy::GetCenter() const
{
  RECT rc = m_rcPosition;
  POINT ptCenter = { rc.left + (rc.right - rc.left) / 2,
    rc.top + (rc.bottom - rc.top) / 2 };
  return ptCenter;
}

POINT Enemy::GetPlayerCenter(Player* pPlayer) const
{
  RECT rc = pPlayer->GetPosition();
  POINT ptCenter = { rc.left + (rc.right - rc.left) / 2,
    rc.top + (rc.bottom - rc.top) / 2 };
  return ptCenter;
}

void Enemy::MoveToward(POINT ptTarget, int iSpeed)
{
  POINT ptCenter = GetCenter();
  int dx = ptTarget.x - ptCenter.x;
  int dy = ptTarget.y - ptCenter.y;
  POINT ptVelocity = { 0, 0 };

  if (abs(dx) > iSpeed)
    ptVelocity.x = (dx > 0) ? iSpeed : -iSpeed;
  if (abs(dy) > iSpeed)
    ptVelocity.y = (dy > 0) ? iSpeed : -iSpeed;

  if (ptVelocity.x != 0 && ptVelocity.y != 0)
  {
    int diagSpeed = (int)(iSpeed * 0.707107 + 0.5);
    ptVelocity.x = (ptVelocity.x > 0) ? diagSpeed : -diagSpeed;
    ptVelocity.y = (ptVelocity.y > 0) ? diagSpeed : -diagSpeed;
  }

  SetVelocity(ptVelocity);
}

void Enemy::StopMoving()
{
  SetVelocity(0, 0);
}

BOOL Enemy::IsPlayerDetected(Player* pPlayer) const
{
  return DistanceToPlayer(pPlayer) <= m_iDetectRange;
}

BOOL Enemy::IsPlayerInAttackRange(Player* pPlayer) const
{
  return DistanceToPlayer(pPlayer) <= m_iAttackRange;
}

void Enemy::UpdateIdle(Player* pPlayer)
{
  StopMoving();

  if (IsPlayerDetected(pPlayer))
  {
    SetState(ENEMY_CHASE);
    return;
  }

  if (!m_vPatrolRoute.empty() && m_iStateTicks > 30)
    SetState(ENEMY_PATROL);
}

void Enemy::UpdatePatrol(Player* pPlayer)
{
  if (IsPlayerDetected(pPlayer))
  {
    SetState(ENEMY_CHASE);
    return;
  }

  if (m_vPatrolRoute.empty())
  {
    SetState(ENEMY_IDLE);
    return;
  }

  POINT ptTarget = m_vPatrolRoute[m_iPatrolIndex];
  POINT ptCenter = GetCenter();
  int dx = ptTarget.x - ptCenter.x;
  int dy = ptTarget.y - ptCenter.y;

  if ((dx * dx + dy * dy) < 256)
    m_iPatrolIndex = (m_iPatrolIndex + 1) % (int)m_vPatrolRoute.size();

  MoveToward(m_vPatrolRoute[m_iPatrolIndex], m_iMoveSpeed);
}

void Enemy::UpdateChase(Player* pPlayer)
{
  if (pPlayer == NULL)
  {
    SetState(ENEMY_FORGET);
    return;
  }

  if (IsPlayerInAttackRange(pPlayer) && m_iAttackCooldown <= 0)
  {
    SetState(ENEMY_ATTACK);
    return;
  }

  if (DistanceToPlayer(pPlayer) > m_iForgetRange)
  {
    SetState(ENEMY_FORGET);
    return;
  }

  MoveToward(GetPlayerCenter(pPlayer), m_iMoveSpeed);
}

void Enemy::UpdateForget(Player* pPlayer)
{
  StopMoving();

  if (IsPlayerDetected(pPlayer))
  {
    SetState(ENEMY_CHASE);
    return;
  }

  if (++m_iForgetTicks > 45)
  {
    m_iForgetTicks = 0;
    SetState(ENEMY_IDLE);
  }
}

void Enemy::UpdateAttack(Player* pPlayer)
{
  StopMoving();

  if (m_iAttackTicks == 0)
    OnAttack(pPlayer);

  m_iAttackTicks++;
  if (m_iAttackTicks > 12)
  {
    m_iAttackTicks = 0;
    m_iAttackCooldown = 24;
    SetState(IsPlayerDetected(pPlayer) ? ENEMY_CHASE : ENEMY_FORGET);
  }
}

void Enemy::OnEnterState(EnemyAIState eState)
{
  if (eState == ENEMY_ATTACK)
  {
    StopMoving();
    m_iAttackTicks = 0;
  }
  else if (eState == ENEMY_FORGET)
  {
    StopMoving();
    m_iForgetTicks = 0;
  }
}

void Enemy::OnAttack(Player* pPlayer)
{
}

void Enemy::OnTouchPlayer(Player* pPlayer)
{
}
