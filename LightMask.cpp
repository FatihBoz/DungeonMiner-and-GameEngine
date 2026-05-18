#include "LightMask.h"
#include <cmath>

#pragma comment(lib, "Msimg32.lib")

LightMask::LightMask(HDC hDC, int textureSize)
  : m_iTextureSize(textureSize), m_hMask(NULL)
{
  BITMAPINFO bmi = { 0 };
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = textureSize;
  bmi.bmiHeader.biHeight = textureSize;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* pvBits;
  m_hMask = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
  if (!m_hMask) return;

  DWORD* pixels = (DWORD*)pvBits;
  float center = textureSize / 2.0f;
  float radius = textureSize / 2.0f; 

  for (int y = 0; y < textureSize; y++)
  {
    for (int x = 0; x < textureSize; x++)
    {
      float dx = x - center;
      float dy = y - center;
      float dist = sqrt(dx * dx + dy * dy);
      float norm = dist / radius;
      if (norm > 1.0f) norm = 1.0f;

      float alphaF = sqrtf(norm); 

      BYTE alpha = (BYTE)(alphaF * 255);
      pixels[y * textureSize + x] = (alpha << 24); // Black premultiplied
    }
  }
}

LightMask::~LightMask()
{
  if (m_hMask)
  {
    DeleteObject(m_hMask);
    m_hMask = NULL;
  }
}

void LightMask::Draw(HDC hDC, int centerX, int centerY, int viewW, int viewH)
{
  if (!m_hMask) return;

  int renderSize = 1600; 
  int destX = centerX - (renderSize / 2);
  int destY = centerY - (renderSize / 2);

  // Get current viewport origin to know which regions are visible
  POINT ptOrg;
  GetWindowOrgEx(hDC, &ptOrg);

  int camX = ptOrg.x;
  int camY = ptOrg.y;
  int camRight = camX + viewW;
  int camBottom = camY + viewH;

  HBRUSH hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

  // Top rect
  if (destY > camY) {
      RECT r = { camX, camY, camRight, destY };
      FillRect(hDC, &r, hBlackBrush);
  }
  // Bottom rect
  if (destY + renderSize < camBottom) {
      RECT r = { camX, destY + renderSize, camRight, camBottom };
      FillRect(hDC, &r, hBlackBrush);
  }
  // Left rect (between top and bottom bounds of destY)
  if (destX > camX) {
      RECT r = { camX, max(camY, destY), destX, min(camBottom, destY + renderSize) };
      FillRect(hDC, &r, hBlackBrush);
  }
  // Right rect
  if (destX + renderSize < camRight) {
      RECT r = { destX + renderSize, max(camY, destY), camRight, min(camBottom, destY + renderSize) };
      FillRect(hDC, &r, hBlackBrush);
  }

  BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

  HDC hMemDC = CreateCompatibleDC(hDC);
  HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, m_hMask);

  AlphaBlend(hDC, destX, destY, renderSize, renderSize, 
             hMemDC, 0, 0, m_iTextureSize, m_iTextureSize, blend);

  SelectObject(hMemDC, hOld);
  DeleteDC(hMemDC);
}
