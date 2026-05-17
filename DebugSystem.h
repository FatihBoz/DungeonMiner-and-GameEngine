#pragma once

#include <windows.h>
#include <tchar.h>

BOOL DebugSystemIsEnabled();
void DebugSetEnabled(BOOL bEnabled);

void DebugInitializeImpl();
void DebugShutdownImpl();
void DebugSetPhaseImpl(const TCHAR* szPhase);
void DebugLogEventImpl(const TCHAR* szEvent);
void DebugLogFormatImpl(const TCHAR* szFormat, ...);
void DebugFrameHeartbeatImpl(int iLevel, POINT ptPlayer, POINT ptCamera,
  size_t nSprites, size_t nEnemies, size_t nFloatingTexts);
DWORD DebugGetFrameNumberImpl();
LONG WINAPI DebugUnhandledExceptionFilterImpl(EXCEPTION_POINTERS* pExceptionInfo);
LONG WINAPI DebugVectoredExceptionHandlerImpl(EXCEPTION_POINTERS* pExceptionInfo);

#define DebugInitialize() DebugInitializeImpl()
#define DebugShutdown() DebugShutdownImpl()
#define DebugSetPhase(szPhase) \
  do { if (DebugSystemIsEnabled()) DebugSetPhaseImpl(szPhase); } while (0)
#define DebugLogEvent(szEvent) \
  do { if (DebugSystemIsEnabled()) DebugLogEventImpl(szEvent); } while (0)
#define DebugLogFormat(...) \
  do { if (DebugSystemIsEnabled()) DebugLogFormatImpl(__VA_ARGS__); } while (0)
#define DebugFrameHeartbeat(iLevel, ptPlayer, ptCamera, nSprites, nEnemies, nFloatingTexts) \
  do { if (DebugSystemIsEnabled()) DebugFrameHeartbeatImpl(iLevel, ptPlayer, ptCamera, \
    nSprites, nEnemies, nFloatingTexts); } while (0)
#define DebugGetFrameNumber() (DebugSystemIsEnabled() ? DebugGetFrameNumberImpl() : 0UL)
