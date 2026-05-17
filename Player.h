#pragma once
#include "Sprite.h"
#include "Inventory.h"

enum PlayerDirection {
  PDIR_DOWN = 0,
  PDIR_LEFT = 1,
  PDIR_RIGHT = 2,
  PDIR_UP = 3
};

class Player : public Sprite
{
private:
  static const int MAX_HEALTH = 5;

  int              m_iSpeed;
  int              m_iHealth;
  Bitmap*          m_pAnimationBitmaps[4]; // Down, Left, Right, Up
  Bitmap*          m_pAttackBitmaps[4];    // Attack Animations (Down, Left, Right, Up)
  PlayerDirection  m_iCurrentDir;
  BOOL             m_bIsMoving;
  BOOL             m_bIsAttacking;
  double           m_dScale; // Player scaling factor
  Inventory        m_inventory; // Player inventory tracking items

  void ExecuteAttackDamage(); // Triggers precise mineral damage based on facing direction

protected:
  virtual void CalcCollisionRect() override; // Tighten physical hitboxes

public:
  Player(Bitmap* pBitmaps[4], RECT& rcBounds, BOUNDSACTION baBoundsAction = BA_STOP);
  virtual ~Player();

  void HandleKeys();
  virtual SPRITEACTION Update() override; // Manage frame advances based on movement
  void Damage(int iDamage);
  void Heal(int iAmount);
  int GetHealth() const { return m_iHealth; }
  int GetMaxHealth() const { return MAX_HEALTH; }
  BOOL IsDead() const { return m_iHealth <= 0; }

  // Setters for Attack bitmaps
  void SetAttackBitmaps(Bitmap* pBitmaps[4]);

  // Scale Accessors
  double GetScale() const { return m_dScale; }
  void SetScale(double dScale);

  // Inventory Accessor
  const Inventory& GetInventory() const { return m_inventory; }
  Inventory& GetInventory() { return m_inventory; }
};
