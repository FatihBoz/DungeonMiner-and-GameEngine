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
  int              m_iSpeed;
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

  // Setters for Attack bitmaps
  void SetAttackBitmaps(Bitmap* pBitmaps[4]);

  // Scale Accessors
  double GetScale() const { return m_dScale; }
  void SetScale(double dScale);

  // Inventory Accessor
  const Inventory& GetInventory() const { return m_inventory; }
  Inventory& GetInventory() { return m_inventory; }
};
