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
static BOOL s_bEnabled = FALSE;
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
  DebugLogFormatImpl(TEXT("%s report=%s"), szType, szWideMessage);
#else
  DebugLogFormatImpl(TEXT("%s report=%s"), szType, szMessage);
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

static BOOL DebugReadEnabledSetting()
{
#ifdef _DEBUG
  BOOL bDefaultEnabled = TRUE;
#else
  BOOL bDefaultEnabled = FALSE;
#endif

  TCHAR szValue[32];
  DWORD dwLength = GetEnvironmentVariable(TEXT("ROIDS_DEBUGSYSTEM"), szValue, 32);
  if (dwLength == 0 || dwLength >= 32)
    return bDefaultEnabled;

  if (_tcsicmp(szValue, TEXT("0")) == 0 ||
    _tcsicmp(szValue, TEXT("false")) == 0 ||
    _tcsicmp(szValue, TEXT("off")) == 0 ||
    _tcsicmp(szValue, TEXT("no")) == 0)
    return FALSE;

  if (_tcsicmp(szValue, TEXT("1")) == 0 ||
    _tcsicmp(szValue, TEXT("true")) == 0 ||
    _tcsicmp(szValue, TEXT("on")) == 0 ||
    _tcsicmp(szValue, TEXT("yes")) == 0)
    return TRUE;

  return bDefaultEnabled;
}

static void DebugStartActiveSession()
{
  if (s_pLogFile != NULL)
    return;

  CreateDirectory(TEXT("DebugLogs"), NULL);

  SYSTEMTIME st;
  GetLocalTime(&st);
  _stprintf_s(s_szLogPath, MAX_PATH,
    TEXT("DebugLogs\\session_%04d%02d%02d_%02d%02d%02d.log"),
    st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  _tfopen_s(&s_pLogFile, s_szLogPath, TEXT("wt"));

  s_pPreviousFilter = SetUnhandledExceptionFilter(DebugUnhandledExceptionFilterImpl);
  s_pVectoredHandler = AddVectoredExceptionHandler(1, DebugVectoredExceptionHandlerImpl);

#ifdef _DEBUG
  _CrtSetReportHook(DebugCrtReportHook);
#endif

  DebugLogFormatImpl(TEXT("Debug session started. logPath=%s"), s_szLogPath);
}

static void DebugStopActiveSession()
{
  if (s_pLogFile == NULL)
    return;

  DebugLogEventImpl(TEXT("Debug session shutting down"));

#ifdef _DEBUG
  _CrtSetReportHook(NULL);
#endif

  if (s_pVectoredHandler != NULL)
  {
    RemoveVectoredExceptionHandler(s_pVectoredHandler);
    s_pVectoredHandler = NULL;
  }

  if (s_pPreviousFilter != NULL)
  {
    SetUnhandledExceptionFilter(s_pPreviousFilter);
    s_pPreviousFilter = NULL;
  }

  fclose(s_pLogFile);
  s_pLogFile = NULL;
}

BOOL DebugSystemIsEnabled()
{
  return s_bEnabled;
}

void DebugSetEnabled(BOOL bEnabled)
{
  if (s_bEnabled == bEnabled && s_bInitialized)
    return;

  s_bEnabled = bEnabled;
  if (!s_bInitialized)
    return;

  if (s_bEnabled)
    DebugStartActiveSession();
  else
    DebugStopActiveSession();
}

void DebugInitializeImpl()
{
  if (s_bInitialized)
    return;

  s_bInitialized = TRUE;
  s_bEnabled = DebugReadEnabledSetting();
  lstrcpyn(s_szLastPhase, TEXT("DebugInitialize"), 128);
  s_dwFrameNumber = 0;

  if (s_bEnabled)
    DebugStartActiveSession();
}

void DebugShutdownImpl()
{
  if (!s_bInitialized)
    return;

  DebugStopActiveSession();
  s_bInitialized = FALSE;
}

void DebugSetPhaseImpl(const TCHAR* szPhase)
{
  if (szPhase == NULL)
    return;

  lstrcpyn(s_szLastPhase, szPhase, 128);
}

void DebugLogEventImpl(const TCHAR* szEvent)
{
  DebugLogFormatImpl(TEXT("%s"), (szEvent != NULL) ? szEvent : TEXT("(null event)"));
}

void DebugLogFormatImpl(const TCHAR* szFormat, ...)
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

void DebugFrameHeartbeatImpl(int iLevel, POINT ptPlayer, POINT ptCamera,
  size_t nSprites, size_t nEnemies, size_t nFloatingTexts)
{
  s_dwFrameNumber++;
  if ((s_dwFrameNumber % 30) != 0)
    return;

  DebugLogFormatImpl(TEXT("heartbeat level=%d player=(%ld,%ld) camera=(%ld,%ld) sprites=%u enemies=%u floatingTexts=%u"),
    iLevel, ptPlayer.x, ptPlayer.y, ptCamera.x, ptCamera.y,
    (unsigned int)nSprites, (unsigned int)nEnemies,
    (unsigned int)nFloatingTexts);
}

DWORD DebugGetFrameNumberImpl()
{
  return s_dwFrameNumber;
}

LONG WINAPI DebugVectoredExceptionHandlerImpl(EXCEPTION_POINTERS* pExceptionInfo)
{
  if (!s_bEnabled)
    return EXCEPTION_CONTINUE_SEARCH;

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
    DebugLogFormatImpl(TEXT("FIRST_CHANCE_EXCEPTION code=0x%08lX address=0x%p lastPhase=%s"),
      dwCode, pAddress, s_szLastPhase);
  }

  if (s_pLogFile != NULL)
    fflush(s_pLogFile);

  return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI DebugUnhandledExceptionFilterImpl(EXCEPTION_POINTERS* pExceptionInfo)
{
  if (!s_bEnabled)
    return EXCEPTION_CONTINUE_SEARCH;

  DWORD dwCode = 0;
  PVOID pAddress = NULL;

  if (pExceptionInfo != NULL && pExceptionInfo->ExceptionRecord != NULL)
  {
    dwCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
    pAddress = pExceptionInfo->ExceptionRecord->ExceptionAddress;
  }

  DebugLogFormatImpl(TEXT("UNHANDLED_EXCEPTION code=0x%08lX address=0x%p lastPhase=%s"),
    dwCode, pAddress, s_szLastPhase);

  if (s_pLogFile != NULL)
    fflush(s_pLogFile);

  if (s_pPreviousFilter != NULL)
    return s_pPreviousFilter(pExceptionInfo);

  return EXCEPTION_EXECUTE_HANDLER;
}
