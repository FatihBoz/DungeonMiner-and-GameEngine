//-----------------------------------------------------------------
// Bitmap Object
// C++ Source - Bitmap.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Bitmap.h"

//-----------------------------------------------------------------
// Bitmap Constructor(s)/Destructor
//-----------------------------------------------------------------
Bitmap::Bitmap()
  : m_hBitmap(NULL), m_iWidth(0), m_iHeight(0)
{
}

// Create a bitmap from a file
Bitmap::Bitmap(HDC hDC, LPTSTR szFileName)
  : m_hBitmap(NULL), m_iWidth(0), m_iHeight(0)
{
  Create(hDC, szFileName);
}

// Create a bitmap from a resource
Bitmap::Bitmap(HDC hDC, UINT uiResID, HINSTANCE hInstance)
  : m_hBitmap(NULL), m_iWidth(0), m_iHeight(0)
{
  Create(hDC, uiResID, hInstance);
}

// Create a blank bitmap from scratch
Bitmap::Bitmap(HDC hDC, int iWidth, int iHeight, COLORREF crColor)
  : m_hBitmap(NULL), m_iWidth(0), m_iHeight(0)
{
  Create(hDC, iWidth, iHeight, crColor);
}

Bitmap::~Bitmap()
{
  Free();
}

//-----------------------------------------------------------------
// Bitmap Helper Methods
//-----------------------------------------------------------------
void Bitmap::Free()
{
  // Delete the bitmap graphics object
  if (m_hBitmap != NULL)
  {
    DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
  }
}

//-----------------------------------------------------------------
// Bitmap General Methods
//-----------------------------------------------------------------
BOOL Bitmap::Create(HDC hDC, LPTSTR szFileName)
{
  // Free any previous bitmap info
  Free();

  // Use standard, robust Windows API to load the bitmap file.
  // This handles all BMP formats, bit depths, and compression methods correctly.
  m_hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0,
    LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  
  if (m_hBitmap == NULL)
    return FALSE;

  // Retrieve bitmap dimensions
  BITMAP bm;
  GetObject(m_hBitmap, sizeof(BITMAP), &bm);
  m_iWidth = bm.bmWidth;
  m_iHeight = bm.bmHeight;

  return TRUE;
}

BOOL Bitmap::Create(HDC hDC, UINT uiResID, HINSTANCE hInstance)
{
  // Free any previous bitmap info
  Free();

  // Use standard, robust Windows API to load the bitmap from resources.
  m_hBitmap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(uiResID),
    IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

  if (m_hBitmap == NULL)
    return FALSE;

  // Retrieve bitmap dimensions
  BITMAP bm;
  GetObject(m_hBitmap, sizeof(BITMAP), &bm);
  m_iWidth = bm.bmWidth;
  m_iHeight = bm.bmHeight;

  return TRUE;
}

BOOL Bitmap::Create(HDC hDC, int iWidth, int iHeight, COLORREF crColor)
{
  // Create a blank bitmap
  m_hBitmap = CreateCompatibleBitmap(hDC, iWidth, iHeight);
  if (m_hBitmap == NULL)
    return FALSE;

  // Set the width and height
  m_iWidth = iWidth;
  m_iHeight = iHeight;

  // Create a memory device context to draw on the bitmap
  HDC hMemDC = CreateCompatibleDC(hDC);

  // Create a solid brush to fill the bitmap
  HBRUSH hBrush = CreateSolidBrush(crColor);

  // Select the bitmap into the device context
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, m_hBitmap);

  // Fill the bitmap with a solid color
  RECT rcBitmap = { 0, 0, m_iWidth, m_iHeight };
  FillRect(hMemDC, &rcBitmap, hBrush);

  // Cleanup
  SelectObject(hMemDC, hOldBitmap);
  DeleteDC(hMemDC);
  DeleteObject(hBrush);

  return TRUE;
}

void Bitmap::Draw(HDC hDC, int x, int y, BOOL bTrans, COLORREF crTransColor)
{
  DrawPart(hDC, x, y, 0, 0, GetWidth(), GetHeight(), bTrans, crTransColor);
}

void Bitmap::DrawPart(HDC hDC, int x, int y, int xPart, int yPart,
  int wPart, int hPart, BOOL bTrans, COLORREF crTransColor)
{
  if (m_hBitmap != NULL)
  {
    // Create a memory device context for the bitmap
    HDC hMemDC = CreateCompatibleDC(hDC);

    // Select the bitmap into the device context
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, m_hBitmap);

    // Draw the bitmap to the destination device context
    if (bTrans)
      TransparentBlt(hDC, x, y, wPart, hPart, hMemDC, xPart, yPart,
        wPart, hPart, crTransColor);
    else
      BitBlt(hDC, x, y, wPart, hPart, hMemDC, xPart, yPart, SRCCOPY);

    // Restore and delete the memory device context
    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);
  }
}

void Bitmap::DrawPartScaled(HDC hDC, int x, int y, int wDest, int hDest,
  int xPart, int yPart, int wPart, int hPart, BOOL bTrans, COLORREF crTransColor)
{
  if (m_hBitmap != NULL)
  {
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, m_hBitmap);

    // Enforce pixel-perfect stretching mode for crisp resizing
    int oldStretchMode = SetStretchBltMode(hDC, COLORONCOLOR);

    if (bTrans)
      TransparentBlt(hDC, x, y, wDest, hDest, hMemDC, xPart, yPart,
        wPart, hPart, crTransColor);
    else
      StretchBlt(hDC, x, y, wDest, hDest, hMemDC, xPart, yPart, 
        wPart, hPart, SRCCOPY);

    // Restore stretch mode
    SetStretchBltMode(hDC, oldStretchMode);

    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);
  }
}

void Bitmap::DrawPartScaledFlipped(HDC hDC, int x, int y, int wDest, int hDest,
  int xPart, int yPart, int wPart, int hPart, BOOL bTrans, COLORREF crTransColor)
{
  if (m_hBitmap != NULL)
  {
    HDC hSrcDC = CreateCompatibleDC(hDC);
    HBITMAP hOldSrcBitmap = (HBITMAP)SelectObject(hSrcDC, m_hBitmap);

    HDC hFlipDC = CreateCompatibleDC(hDC);
    HBITMAP hFlipBitmap = CreateCompatibleBitmap(hDC, wPart, hPart);
    HBITMAP hOldFlipBitmap = (HBITMAP)SelectObject(hFlipDC, hFlipBitmap);

    HBRUSH hTransparentBrush = CreateSolidBrush(crTransColor);
    RECT rcFlip = { 0, 0, wPart, hPart };
    FillRect(hFlipDC, &rcFlip, hTransparentBrush);
    DeleteObject(hTransparentBrush);

    int oldFlipStretchMode = SetStretchBltMode(hFlipDC, COLORONCOLOR);
    StretchBlt(hFlipDC, 0, 0, wPart, hPart, hSrcDC, xPart + wPart, yPart,
      -wPart, hPart, SRCCOPY);
    SetStretchBltMode(hFlipDC, oldFlipStretchMode);

    int oldStretchMode = SetStretchBltMode(hDC, COLORONCOLOR);
    if (bTrans)
      TransparentBlt(hDC, x, y, wDest, hDest, hFlipDC, 0, 0, wPart, hPart,
        crTransColor);
    else
      StretchBlt(hDC, x, y, wDest, hDest, hFlipDC, 0, 0, wPart, hPart, SRCCOPY);
    SetStretchBltMode(hDC, oldStretchMode);

    SelectObject(hFlipDC, hOldFlipBitmap);
    DeleteObject(hFlipBitmap);
    DeleteDC(hFlipDC);

    SelectObject(hSrcDC, hOldSrcBitmap);
    DeleteDC(hSrcDC);
  }
}

