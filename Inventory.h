#pragma once
#include <windows.h>

class Inventory
{
private:
  int m_iCounts[8]; // Stores counts for 8 distinct slots

public:
  Inventory()
  {
    for (int i = 0; i < 8; i++)
    {
      m_iCounts[i] = 0;
    }
  }

  void AddItem(int itemType, int count = 1)
  {
    if (itemType >= 1 && itemType <= 8)
    {
      m_iCounts[itemType - 1] += count;
    }
  }

  void RemoveItem(int itemType, int count = 1)
  {
    if (itemType >= 1 && itemType <= 8)
    {
      m_iCounts[itemType - 1] -= count;
      if (m_iCounts[itemType - 1] < 0) m_iCounts[itemType - 1] = 0;
    }
  }

  int GetItemCount(int itemType) const
  {
    if (itemType >= 1 && itemType <= 8)
    {
      return m_iCounts[itemType - 1];
    }
    return 0;
  }
};
