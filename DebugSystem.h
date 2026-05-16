#pragma once

#include <windows.h>
#include <tchar.h>

void DebugInitialize();
void DebugShutdown();
void DebugSetPhase(const TCHAR* szPhase);
void DebugLogEvent(const TCHAR* szEvent);
void DebugLogFormat(const TCHAR* szFormat, ...);
void DebugFrameHeartbeat(int iLevel, POINT ptPlayer, POINT ptCamera,
  size_t nSprites, size_t nEnemies, size_t nFloatingTexts);
DWORD DebugGetFrameNumber();
LONG WINAPI DebugUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo);
LONG WINAPI DebugVectoredExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo);
