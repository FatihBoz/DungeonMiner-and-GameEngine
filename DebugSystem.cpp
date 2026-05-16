#include "DebugSystem.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

static FILE* s_pLogFile = NULL;
static TCHAR s_szLogPath[MAX_PATH] = TEXT("");
static TCHAR s_szLastPhase[128] = TEXT("not initialized");
static DWORD s_dwFrameNumber = 0;
static BOOL s_bInitialized = FALSE;
static LPTOP_LEVEL_EXCEPTION_FILTER s_pPreviousFilter = NULL;
static PVOID s_pVectoredHandler = NULL;

#ifdef _DEBUG
static int DebugCrtReportHook(int iReportType, char* szMessage, int* pReturnValue)
{
  const TCHAR* szType = TEXT("CRT_UNKNOWN");
  switch (iReportType)
  {
    case _CRT_WARN:
      szType = TEXT("CRT_WARN");
      break;
    case _CRT_ERROR:
      szType = TEXT("CRT_ERROR");
      break;
    case _CRT_ASSERT:
      szType = TEXT("CRT_ASSERT");
      break;
  }

#ifdef UNICODE
  WCHAR szWideMessage[1024];
  MultiByteToWideChar(CP_ACP, 0, szMessage, -1, szWideMessage, 1024);
  DebugLogFormat(TEXT("%s report=%s"), szType, szWideMessage);
#else
  DebugLogFormat(TEXT("%s report=%s"), szType, szMessage);
#endif

  if (pReturnValue != NULL)
    *pReturnValue = 0;

  return FALSE;
}
#endif

static void DebugWritePrefix()
{
  if (s_pLogFile == NULL)
    return;

  SYSTEMTIME st;
  GetLocalTime(&st);
  _ftprintf(s_pLogFile, TEXT("[%04d-%02d-%02d %02d:%02d:%02d.%03d][frame=%lu][phase=%s] "),
    st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
    st.wMilliseconds, s_dwFrameNumber, s_szLastPhase);
}

void DebugInitialize()
{
  if (s_bInitialized)
    return;

  CreateDirectory(TEXT("DebugLogs"), NULL);

  SYSTEMTIME st;
  GetLocalTime(&st);
  _stprintf_s(s_szLogPath, MAX_PATH,
    TEXT("DebugLogs\\session_%04d%02d%02d_%02d%02d%02d.log"),
    st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  _tfopen_s(&s_pLogFile, s_szLogPath, TEXT("wt"));
  s_bInitialized = TRUE;
  lstrcpyn(s_szLastPhase, TEXT("DebugInitialize"), 128);
  s_pPreviousFilter = SetUnhandledExceptionFilter(DebugUnhandledExceptionFilter);
  s_pVectoredHandler = AddVectoredExceptionHandler(1, DebugVectoredExceptionHandler);

#ifdef _DEBUG
  _CrtSetReportHook(DebugCrtReportHook);
#endif

  DebugLogFormat(TEXT("Debug session started. logPath=%s"), s_szLogPath);
}

void DebugShutdown()
{
  if (!s_bInitialized)
    return;

  DebugLogEvent(TEXT("Debug session shutting down"));

#ifdef _DEBUG
  _CrtSetReportHook(NULL);
#endif

  if (s_pVectoredHandler != NULL)
  {
    RemoveVectoredExceptionHandler(s_pVectoredHandler);
    s_pVectoredHandler = NULL;
  }

  if (s_pLogFile != NULL)
  {
    fclose(s_pLogFile);
    s_pLogFile = NULL;
  }
  s_bInitialized = FALSE;
}

void DebugSetPhase(const TCHAR* szPhase)
{
  if (szPhase == NULL)
    return;

  lstrcpyn(s_szLastPhase, szPhase, 128);
}

void DebugLogEvent(const TCHAR* szEvent)
{
  DebugLogFormat(TEXT("%s"), (szEvent != NULL) ? szEvent : TEXT("(null event)"));
}

void DebugLogFormat(const TCHAR* szFormat, ...)
{
  if (s_pLogFile == NULL || szFormat == NULL)
    return;

  DebugWritePrefix();

  va_list args;
  va_start(args, szFormat);
  _vftprintf(s_pLogFile, szFormat, args);
  va_end(args);

  _ftprintf(s_pLogFile, TEXT("\n"));
  fflush(s_pLogFile);
}

void DebugFrameHeartbeat(int iLevel, POINT ptPlayer, POINT ptCamera,
  size_t nSprites, size_t nEnemies, size_t nFloatingTexts)
{
  s_dwFrameNumber++;
  if ((s_dwFrameNumber % 30) != 0)
    return;

  DebugLogFormat(TEXT("heartbeat level=%d player=(%ld,%ld) camera=(%ld,%ld) sprites=%u enemies=%u floatingTexts=%u"),
    iLevel, ptPlayer.x, ptPlayer.y, ptCamera.x, ptCamera.y,
    (unsigned int)nSprites, (unsigned int)nEnemies,
    (unsigned int)nFloatingTexts);
}

DWORD DebugGetFrameNumber()
{
  return s_dwFrameNumber;
}

LONG WINAPI DebugVectoredExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo)
{
  DWORD dwCode = 0;
  PVOID pAddress = NULL;

  if (pExceptionInfo != NULL && pExceptionInfo->ExceptionRecord != NULL)
  {
    dwCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
    pAddress = pExceptionInfo->ExceptionRecord->ExceptionAddress;
  }

  if (dwCode == EXCEPTION_BREAKPOINT ||
    dwCode == EXCEPTION_ACCESS_VIOLATION ||
    dwCode == 0xC0000374)
  {
    DebugLogFormat(TEXT("FIRST_CHANCE_EXCEPTION code=0x%08lX address=0x%p lastPhase=%s"),
      dwCode, pAddress, s_szLastPhase);
  }

  if (s_pLogFile != NULL)
    fflush(s_pLogFile);

  return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI DebugUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
  DWORD dwCode = 0;
  PVOID pAddress = NULL;

  if (pExceptionInfo != NULL && pExceptionInfo->ExceptionRecord != NULL)
  {
    dwCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
    pAddress = pExceptionInfo->ExceptionRecord->ExceptionAddress;
  }

  DebugLogFormat(TEXT("UNHANDLED_EXCEPTION code=0x%08lX address=0x%p lastPhase=%s"),
    dwCode, pAddress, s_szLastPhase);

  if (s_pLogFile != NULL)
    fflush(s_pLogFile);

  if (s_pPreviousFilter != NULL)
    return s_pPreviousFilter(pExceptionInfo);

  return EXCEPTION_EXECUTE_HANDLER;
}
