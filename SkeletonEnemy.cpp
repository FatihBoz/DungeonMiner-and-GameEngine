#include "SkeletonEnemy.h"
#include "DebugSystem.h"
#include "Player.h"
#include <cstdlib>

extern void SpawnSkeletonProjectile(POINT ptStart, POINT ptTarget);

static const int SKELETON_ANIM_UP = 0;
static const int SKELETON_ANIM_RIGHT = 1;
static const int SKELETON_ANIM_DOWN = 2;
static const int SKELETON_ANIM_LEFT = 3;
static const int SKELETON_FIRE_COOLDOWN_TICKS = 60;

SkeletonEnemy::SkeletonEnemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds)
  : Enemy(pBitmap, ptPosition, rcBounds), m_iFireCooldown(0),
    m_iLastAnimationRow(SKELETON_ANIM_DOWN)
{
  m_iHealth = 1;
  m_iMoveSpeed = 3;
  m_iDetectRange = 300;
  m_iForgetRange = 300;
  m_iAttackRange = 360;
  SetScale(2.0);
  SetNumFrames(3);
  SetFrameRows(4);
  SetFrameRow(m_iLastAnimationRow);
  SetFrameDelay(6);
  SetState(ENEMY_PATROL);
  DebugLogFormat(TEXT("Skeleton created this=0x%p pos=(%ld,%ld)"), this, ptPosition.x, ptPosition.y);
}

SkeletonEnemy::~SkeletonEnemy()
{
  DebugLogFormat(TEXT("Skeleton destroyed this=0x%p"), this);
}

SPRITEACTION SkeletonEnemy::Update()
{
  SPRITEACTION saAction = Enemy::Update();
  HoldStoppedAnimationFrame();
  return saAction;
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
  UpdateAnimationDirection();
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
    m_iFireCooldown = SKELETON_FIRE_COOLDOWN_TICKS;
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

void SkeletonEnemy::UpdateAnimationDirection()
{
  POINT ptVelocity = GetVelocity();

  if (ptVelocity.y < 0)
    m_iLastAnimationRow = SKELETON_ANIM_UP;
  else if (ptVelocity.x > 0)
    m_iLastAnimationRow = SKELETON_ANIM_RIGHT;
  else if (ptVelocity.y > 0)
    m_iLastAnimationRow = SKELETON_ANIM_DOWN;
  else if (ptVelocity.x < 0)
    m_iLastAnimationRow = SKELETON_ANIM_LEFT;
  else
    return;

  SetFrameRow(m_iLastAnimationRow);
}

void SkeletonEnemy::HoldStoppedAnimationFrame()
{
  POINT ptVelocity = GetVelocity();

  if (ptVelocity.x != 0 || ptVelocity.y != 0)
    return;

  SetFrameRow(m_iLastAnimationRow);
  m_iCurFrame = 1;
}
