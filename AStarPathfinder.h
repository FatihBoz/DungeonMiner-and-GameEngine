#pragma once

#include <windows.h>
#include <vector>

typedef BOOL (*AStarTileBlockedFn)(int row, int col, void* pContext);

class AStarPathfinder
{
public:
  static BOOL FindPath(int iStartRow, int iStartCol, int iGoalRow, int iGoalCol,
    int iRows, int iCols, AStarTileBlockedFn pBlockedFn, void* pContext,
    std::vector<POINT>& vPath);
};
