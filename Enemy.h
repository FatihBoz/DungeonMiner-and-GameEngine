#pragma once

#include <windows.h>
#include <vector>
#include "Sprite.h"

enum EnemyAIState
{
  ENEMY_IDLE = 0,
  ENEMY_PATROL,
  ENEMY_CHASE,
  ENEMY_FORGET,
  ENEMY_ATTACK
};

class Player;

class Enemy : public Sprite
{
protected:
  EnemyAIState    m_eState;
  int             m_iHealth;
  int             m_iMoveSpeed;
  int             m_iDetectRange;
  int             m_iForgetRange;
  int             m_iAttackRange;
  int             m_iStateTicks;
  int             m_iForgetTicks;
  int             m_iAttackTicks;
  int             m_iAttackCooldown;
  int             m_iTouchCooldown;
  std::vector<POINT> m_vPatrolRoute;
  int             m_iPatrolIndex;

  void            SetState(EnemyAIState eState);
  int             DistanceToPlayer(Player* pPlayer) const;
  POINT           GetCenter() const;
  POINT           GetPlayerCenter(Player* pPlayer) const;
  void            MoveToward(POINT ptTarget, int iSpeed);
  void            StopMoving();
  BOOL            IsPlayerDetected(Player* pPlayer) const;
  BOOL            IsPlayerInAttackRange(Player* pPlayer) const;

  virtual void    UpdateIdle(Player* pPlayer);
  virtual void    UpdatePatrol(Player* pPlayer);
  virtual void    UpdateChase(Player* pPlayer);
  virtual void    UpdateForget(Player* pPlayer);
  virtual void    UpdateAttack(Player* pPlayer);
  virtual void    OnEnterState(EnemyAIState eState);
  virtual void    OnAttack(Player* pPlayer);
  virtual void    OnTouchPlayer(Player* pPlayer);

public:
  Enemy(Bitmap* pBitmap, POINT ptPosition, RECT& rcBounds);
  virtual ~Enemy();

  virtual SPRITEACTION Update() override;
  void SetPatrolRoute(const std::vector<POINT>& vRoute);
  EnemyAIState GetState() const { return m_eState; }
  int GetHealth() const { return m_iHealth; }
  BOOL IsDead() const { return m_iHealth <= 0; }
  void Damage(int iDamage);
  void TouchPlayer(Player* pPlayer);
};
