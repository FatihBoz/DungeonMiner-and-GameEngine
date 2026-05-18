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
  static const int MAX_TOOL_DURABILITY = 20;

  int              m_iSpeed;
  int              m_iHealth;
  Bitmap*          m_pAnimationBitmaps[4]; // Down, Left, Right, Up
  Bitmap*          m_pAttackBitmaps[4];    // Attack Animations (Down, Left, Right, Up)
  PlayerDirection  m_iCurrentDir;
  BOOL             m_bIsMoving;
  BOOL             m_bIsAttacking;
  int              m_iLastFootstepFrame;
  double           m_dScale; // Player scaling factor
  int              m_iActiveTool; // 0 = pickaxe, 1 = sword
  int              m_iToolDurability[2]; // Durability for 0:pickaxe, 1:sword
  int              m_iMaxToolDurability[2]; // Max durability for 0:pickaxe, 1:sword
  int              m_iToolLevel[2]; // Level for 0:pickaxe, 1:sword
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

  // Active Tool Accessor
  int GetActiveTool() const { return m_iActiveTool; }
  
  // Tool Durability & Level Accessors
  int GetToolDurability(int toolIndex) const { return m_iToolDurability[toolIndex]; }
  int GetMaxToolDurability(int toolIndex) const { return m_iMaxToolDurability[toolIndex]; }
  void DecreaseToolDurability(int toolIndex) {
    if (m_iToolDurability[toolIndex] > 0) m_iToolDurability[toolIndex]--;
  }
  
  int GetToolLevel(int toolIndex) const { return m_iToolLevel[toolIndex]; }
  void UpgradeTool(int toolIndex) {
    if (m_iToolLevel[toolIndex] < 5) {
      m_iToolLevel[toolIndex]++;
      m_iMaxToolDurability[toolIndex] += 5; // Increase max durability by 5
      m_iToolDurability[toolIndex] = m_iMaxToolDurability[toolIndex]; // Full restore
    }
  }
  
  void RepairActiveTool(); // Repairs based on level and inventory

  // Inventory Accessor
  const Inventory& GetInventory() const { return m_inventory; }
  Inventory& GetInventory() { return m_inventory; }
};
