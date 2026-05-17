//-----------------------------------------------------------------
// Game Engine Object
// C++ Source - GameEngine.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "GameEngine.h"
#include "DebugSystem.h"

//-----------------------------------------------------------------
// Static Variable Initialization
//-----------------------------------------------------------------
GameEngine *GameEngine::m_pGameEngine = NULL;

//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  PSTR szCmdLine, int iCmdShow)
{
  DebugInitialize();
  DebugSetPhase(TEXT("WinMain"));
  DebugLogEvent(TEXT("WinMain entered"));

  MSG         msg;
  static int  iTickTrigger = 0;
  int         iTickCount;

  if (GameInitialize(hInstance))
  {
    DebugLogEvent(TEXT("GameInitialize succeeded"));

    // Initialize the game engine
    if (!GameEngine::GetEngine()->Initialize(iCmdShow))
    {
      DebugLogEvent(TEXT("GameEngine::Initialize failed"));
      GameEnd();
      DebugShutdown();
      return FALSE;
    }

    DebugLogEvent(TEXT("GameEngine::Initialize succeeded"));

    // Enter the main message loop
    while (TRUE)
    {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        // Process the message
        if (msg.message == WM_QUIT)
          break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else
      {
        // Make sure the game engine isn't sleeping
        if (!GameEngine::GetEngine()->GetSleep())
        {
          // Check the tick count to see if a game cycle has elapsed
          iTickCount = GetTickCount();
          if (iTickCount > iTickTrigger)
          {
            iTickTrigger = iTickCount +
              GameEngine::GetEngine()->GetFrameDelay();
            DebugSetPhase(TEXT("HandleKeys"));
            HandleKeys();
            /*
            // Joystick input is disabled for now. Keep the old polling path here
            // so it can be restored without rewriting the joystick implementation.
            DebugSetPhase(TEXT("CheckJoystick"));
            GameEngine::GetEngine()->CheckJoystick();
            */
            DebugSetPhase(TEXT("GameCycle"));
            GameCycle();
          }
        }
      }
    }
    DebugLogFormat(TEXT("WinMain exiting normally code=%d"), (int)msg.wParam);
    GameEnd();
    DebugShutdown();
    return (int)msg.wParam;
  }

  // End the game
  DebugLogEvent(TEXT("GameInitialize failed"));
  GameEnd();
  DebugShutdown();

  return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DebugSetPhase(TEXT("WndProc"));

  // Route all Windows messages to the game engine
  return GameEngine::GetEngine()->HandleEvent(hWindow, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// Game Engine Helper Methods
//-----------------------------------------------------------------
BOOL GameEngine::CheckSpriteCollision(Sprite* pTestSprite)
{
  DebugSetPhase(TEXT("CheckSpriteCollision"));

  // Check if the sprite collided with map elements first
  if (MapCollision(pTestSprite))
    return TRUE;

  // See if the sprite has collided with any other sprites
  vector<Sprite*>::iterator siSprite;
  for (siSprite = m_vSprites.begin(); siSprite != m_vSprites.end(); siSprite++)
  {
    // Make sure not to check for collision with itself
    if (pTestSprite == (*siSprite))
      continue;

    // Test the collision
    if (pTestSprite->TestCollision(*siSprite))
      // Collision detected
      return SpriteCollision((*siSprite), pTestSprite);
  }

  // No collision
  return FALSE;
}

//-----------------------------------------------------------------
// GameEngine Constructor(s)/Destructor
//-----------------------------------------------------------------
GameEngine::GameEngine(HINSTANCE hInstance, LPTSTR szWindowClass,
  LPTSTR szTitle, WORD wIcon, WORD wSmallIcon, int iWidth, int iHeight)
{
  // Set the member variables for the game engine
  m_pGameEngine = this;
  m_hInstance = hInstance;
  m_hWindow = NULL;
  if (lstrlen(szWindowClass) > 0)
    lstrcpy(m_szWindowClass, szWindowClass);
  if (lstrlen(szTitle) > 0)
    lstrcpy(m_szTitle, szTitle);
  m_wIcon = wIcon;
  m_wSmallIcon = wSmallIcon;
  m_iWidth = iWidth;
  m_iHeight = iHeight;
  m_iFrameDelay = 50;   // 20 FPS default
  m_bSleep = TRUE;
  m_uiJoystickID = 0;
  m_vSprites.reserve(50);
  m_vPendingSprites.reserve(16);
  m_bUpdatingSprites = FALSE;
  m_uiMIDIPlayerID = 0;
}

GameEngine::~GameEngine()
{
}

//-----------------------------------------------------------------
// Game Engine General Methods
//-----------------------------------------------------------------
BOOL GameEngine::Initialize(int iCmdShow)
{
  WNDCLASSEX    wndclass;

  // Create the window class for the main window
  wndclass.cbSize         = sizeof(wndclass);
  wndclass.style          = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc    = WndProc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = m_hInstance;
  wndclass.hIcon          = LoadIcon(m_hInstance,
    MAKEINTRESOURCE(GetIcon()));
  wndclass.hIconSm        = LoadIcon(m_hInstance,
    MAKEINTRESOURCE(GetSmallIcon()));
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
  wndclass.lpszMenuName   = NULL;
  wndclass.lpszClassName  = m_szWindowClass;

  // Register the window class
  if (!RegisterClassEx(&wndclass))
    return FALSE;

  // Calculate the window size and position based upon the game size
  int iWindowWidth = m_iWidth + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
      iWindowHeight = m_iHeight + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
        GetSystemMetrics(SM_CYCAPTION);
  if (wndclass.lpszMenuName != NULL)
    iWindowHeight += GetSystemMetrics(SM_CYMENU);
  int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2,
      iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;

  // Create the window
  m_hWindow = CreateWindow(m_szWindowClass, m_szTitle, WS_POPUPWINDOW |
    WS_CAPTION | WS_MINIMIZEBOX, iXWindowPos, iYWindowPos, iWindowWidth,
    iWindowHeight, NULL, NULL, m_hInstance, NULL);
  if (!m_hWindow)
    return FALSE;

  // Show and update the window
  ShowWindow(m_hWindow, iCmdShow);
  UpdateWindow(m_hWindow);

  return TRUE;
}

LRESULT GameEngine::HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
  // Route Windows messages to game engine member functions
  switch (msg)
  {
    case WM_CREATE:
      // Set the game window and start the game
      SetWindow(hWindow);
      GameStart(hWindow);
      return 0;

    case WM_ACTIVATE:
      // Activate/deactivate the game and update the Sleep status
      if (wParam != WA_INACTIVE)
      {
        GameActivate(hWindow);
        SetSleep(FALSE);
      }
      else
      {
        GameDeactivate(hWindow);
        SetSleep(TRUE);
      }
      return 0;

    case WM_PAINT:
      HDC         hDC;
      PAINTSTRUCT ps;
      hDC = BeginPaint(hWindow, &ps);

      // Paint the game
      GamePaint(hDC);

      EndPaint(hWindow, &ps);
      return 0;

    case WM_LBUTTONDOWN:
      // Handle left mouse button press
      MouseButtonDown(LOWORD(lParam), HIWORD(lParam), TRUE);
      return 0;

    case WM_LBUTTONUP:
      // Handle left mouse button release
      MouseButtonUp(LOWORD(lParam), HIWORD(lParam), TRUE);
      return 0;

    case WM_RBUTTONDOWN:
      // Handle right mouse button press
      MouseButtonDown(LOWORD(lParam), HIWORD(lParam), FALSE);
      return 0;

    case WM_RBUTTONUP:
      // Handle right mouse button release
      MouseButtonUp(LOWORD(lParam), HIWORD(lParam), FALSE);
      return 0;

    case WM_MOUSEMOVE:
      // Handle mouse movement
      MouseMove(LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_DESTROY:
      // End the game and exit the application
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWindow, msg, wParam, lParam);
}

void GameEngine::ErrorQuit(LPTSTR szErrorMsg)
{
  MessageBox(GetWindow(), szErrorMsg, TEXT("Critical Error"), MB_OK | MB_ICONERROR);
  PostQuitMessage(0);
}

BOOL GameEngine::InitJoystick()
{
  // Make sure joystick driver is present
  UINT uiNumJoysticks;
  if ((uiNumJoysticks = joyGetNumDevs()) == 0)
    return FALSE;

  // Make sure the joystick is attached
  JOYINFO jiInfo;
  if (joyGetPos(JOYSTICKID1, &jiInfo) != JOYERR_UNPLUGGED)
    m_uiJoystickID = JOYSTICKID1;
  else
    return FALSE;

  // Calculate the trip values
  JOYCAPS jcCaps;
  joyGetDevCaps(m_uiJoystickID, &jcCaps, sizeof(JOYCAPS));
  DWORD dwXCenter = ((DWORD)jcCaps.wXmin + jcCaps.wXmax) / 2;
  DWORD dwYCenter = ((DWORD)jcCaps.wYmin + jcCaps.wYmax) / 2;
  m_rcJoystickTrip.left = (jcCaps.wXmin + (WORD)dwXCenter) / 2;
  m_rcJoystickTrip.right = (jcCaps.wXmax + (WORD)dwXCenter) / 2;
  m_rcJoystickTrip.top = (jcCaps.wYmin + (WORD)dwYCenter) / 2;
  m_rcJoystickTrip.bottom = (jcCaps.wYmax + (WORD)dwYCenter) / 2;

  return TRUE;
}

void GameEngine::CaptureJoystick()
{
  // Capture the joystick
  if (m_uiJoystickID == JOYSTICKID1)
    joySetCapture(m_hWindow, m_uiJoystickID, NULL, TRUE);
}

void GameEngine::ReleaseJoystick()
{
  // Release the joystick
  if (m_uiJoystickID == JOYSTICKID1)
    joyReleaseCapture(m_uiJoystickID);
}

void GameEngine::CheckJoystick()
{
  if (m_uiJoystickID == JOYSTICKID1)
  {
    JOYINFO jiInfo;
    JOYSTATE jsJoystickState = 0;
    if (joyGetPos(m_uiJoystickID, &jiInfo) == JOYERR_NOERROR)
    {
      // Check horizontal movement
      if (jiInfo.wXpos < (WORD)m_rcJoystickTrip.left)
        jsJoystickState |= JOY_LEFT;
      else if (jiInfo.wXpos > (WORD)m_rcJoystickTrip.right)
        jsJoystickState |= JOY_RIGHT;

      // Check vertical movement
      if (jiInfo.wYpos < (WORD)m_rcJoystickTrip.top)
        jsJoystickState |= JOY_UP;
      else if (jiInfo.wYpos > (WORD)m_rcJoystickTrip.bottom)
        jsJoystickState |= JOY_DOWN;

      // Check buttons
      if(jiInfo.wButtons & JOY_BUTTON1)
        jsJoystickState |= JOY_FIRE1;
      if(jiInfo.wButtons & JOY_BUTTON2)
        jsJoystickState |= JOY_FIRE2;
    }

    // Allow the game to handle the joystick
    HandleJoystick(jsJoystickState);
  }
}

void GameEngine::AddSprite(Sprite* pSprite)
{
  if (pSprite == NULL)
    return;

  if (m_bUpdatingSprites)
  {
    m_vPendingSprites.push_back(pSprite);
    return;
  }

  AddSpriteNow(pSprite);
}

void GameEngine::AddSpriteNow(Sprite* pSprite)
{
  // Add a sprite to the sprite vector
  if (pSprite != NULL)
  {
    // See if there are sprites already in the sprite vector
    if (m_vSprites.size() > 0)
    {
      // Find a spot in the sprite vector to insert the sprite by its z-order
      vector<Sprite*>::iterator siSprite;
      for (siSprite = m_vSprites.begin(); siSprite != m_vSprites.end(); siSprite++)
        if (pSprite->GetZOrder() < (*siSprite)->GetZOrder())
        {
          // Insert the sprite into the sprite vector
          m_vSprites.insert(siSprite, pSprite);
          return;
        }
    }

    // The sprite's z-order is highest, so add it to the end of the vector
    m_vSprites.push_back(pSprite);
  }
}

BOOL GameEngine::RemoveSprite(Sprite* pSprite)
{
  if (pSprite == NULL)
    return FALSE;

  for (vector<Sprite*>::iterator siPendingSprite = m_vPendingSprites.begin();
    siPendingSprite != m_vPendingSprites.end(); siPendingSprite++)
  {
    if (*siPendingSprite == pSprite)
    {
      delete (*siPendingSprite);
      m_vPendingSprites.erase(siPendingSprite);
      return TRUE;
    }
  }

  if (m_bUpdatingSprites)
  {
    pSprite->Kill();
    return FALSE;
  }

  for (vector<Sprite*>::iterator siSprite = m_vSprites.begin();
    siSprite != m_vSprites.end(); siSprite++)
  {
    if (*siSprite == pSprite)
    {
      delete (*siSprite);
      m_vSprites.erase(siSprite);
      return TRUE;
    }
  }

  return FALSE;
}

void GameEngine::FlushPendingSprites()
{
  for (vector<Sprite*>::iterator siSprite = m_vPendingSprites.begin();
    siSprite != m_vPendingSprites.end(); siSprite++)
  {
    AddSpriteNow(*siSprite);
  }

  m_vPendingSprites.clear();
}

void GameEngine::DrawSprites(HDC hDC)
{
  // Draw the sprites in the sprite vector
  vector<Sprite*>::iterator siSprite;
  for (siSprite = m_vSprites.begin(); siSprite != m_vSprites.end(); siSprite++)
    (*siSprite)->Draw(hDC);
}

void GameEngine::UpdateSprites()
{
  DebugSetPhase(TEXT("UpdateSprites"));

  // Update the sprites in the sprite vector
  RECT          rcOldSpritePos;
  SPRITEACTION  saSpriteAction;
  vector<Sprite*>::iterator siSprite;
  m_bUpdatingSprites = TRUE;
  for (siSprite = m_vSprites.begin(); siSprite != m_vSprites.end(); /* siSprite++ */ )
  {
    // Save the old sprite position in case we need to restore it
    rcOldSpritePos = (*siSprite)->GetPosition();

    // Update the sprite
    DebugSetPhase(TEXT("UpdateSprites.SpriteUpdate"));
    saSpriteAction = (*siSprite)->Update();

    // Handle the SA_KILL sprite action
    if (saSpriteAction & SA_KILL)
    {
      DebugLogFormat(TEXT("sprite kill sprite=0x%p"), *siSprite);

      // Notify the game that the sprite is dying
      SpriteDying(*siSprite);

      // Kill the sprite
      delete (*siSprite);
      siSprite=m_vSprites.erase(siSprite);
      //siSprite--;
      continue;
    }

    // See if the sprite collided with any others
    DebugSetPhase(TEXT("UpdateSprites.Collision"));
    if (CheckSpriteCollision(*siSprite))
      (*siSprite)->SetPosition(rcOldSpritePos);  // Restore the old sprite position
	siSprite++;
  }
  m_bUpdatingSprites = FALSE;
  FlushPendingSprites();
}

void GameEngine::CleanupSprites()
{
  for (vector<Sprite*>::iterator siPendingSprite = m_vPendingSprites.begin();
    siPendingSprite != m_vPendingSprites.end(); siPendingSprite++)
  {
    delete (*siPendingSprite);
  }
  m_vPendingSprites.clear();
  m_bUpdatingSprites = FALSE;

  // Delete and remove the sprites in the sprite vector
  vector<Sprite*>::iterator siSprite;
  for (siSprite = m_vSprites.begin(); siSprite != m_vSprites.end(); /* siSprite++ */)
  {
    delete (*siSprite);
    siSprite=m_vSprites.erase(siSprite);
    //siSprite--;
  }
}

Sprite* GameEngine::IsPointInSprite(int x, int y)
{
  // See if the point is in a sprite in the sprite vector
  vector<Sprite*>::reverse_iterator siSprite;
  for (siSprite = m_vSprites.rbegin(); siSprite != m_vSprites.rend(); siSprite++)
    if (!(*siSprite)->IsHidden() && (*siSprite)->IsPointInside(x, y))
      return (*siSprite);

  // The point is not in a sprite
  return NULL;
}

void GameEngine::PlayMIDISong(LPTSTR szMIDIFileName, BOOL bRestart)
{
  // See if the MIDI player needs to be opened
  if (m_uiMIDIPlayerID == 0)
  {
    // Open the MIDI player by specifying the device and filename
    MCI_OPEN_PARMS mciOpenParms;
    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = szMIDIFileName;
    if (mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
      (DWORD_PTR)&mciOpenParms) == 0)
      // Get the ID for the MIDI player
      m_uiMIDIPlayerID = mciOpenParms.wDeviceID;
    else
      // There was a problem, so just return
      return;
  }

  // Restart the MIDI song, if necessary
  if (bRestart)
  {
    MCI_SEEK_PARMS mciSeekParms;
    if (mciSendCommand(m_uiMIDIPlayerID, MCI_SEEK, MCI_SEEK_TO_START,
      (DWORD_PTR)&mciSeekParms) != 0)
      // There was a problem, so close the MIDI player
      CloseMIDIPlayer();
  }

  // Play the MIDI song
  MCI_PLAY_PARMS mciPlayParms;
  if (mciSendCommand(m_uiMIDIPlayerID, MCI_PLAY, 0,
    (DWORD_PTR)&mciPlayParms) != 0)
    // There was a problem, so close the MIDI player
    CloseMIDIPlayer();
}

void GameEngine::PauseMIDISong()
{
  // Pause the currently playing song, if possible
  if (m_uiMIDIPlayerID != 0)
    mciSendCommand(m_uiMIDIPlayerID, MCI_PAUSE, 0, NULL);
}

void GameEngine::CloseMIDIPlayer()
{
  // Close the MIDI player, if possible
  if (m_uiMIDIPlayerID != 0)
  {
    mciSendCommand(m_uiMIDIPlayerID, MCI_CLOSE, 0, NULL);
    m_uiMIDIPlayerID = 0;
  }
}
