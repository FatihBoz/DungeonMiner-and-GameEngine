#pragma once
#include <windows.h>

class LightMask
{
private:
  HBITMAP m_hMask;
  int     m_iTextureSize;

public:
  LightMask(HDC hDC, int textureSize = 256);
  virtual ~LightMask();

  void Draw(HDC hDC, int centerX, int centerY, int viewportWidth = 1280, int viewportHeight = 720);
};
