#include "../AStarPathfinder.h"
#include <stdio.h>

static BOOL IsBlocked(int row, int col, void* pContext)
{
  int* pGrid = (int*)pContext;
  return pGrid[row * 5 + col] != 0;
}

static int Fail(const char* szMessage)
{
  printf("FAIL: %s\n", szMessage);
  return 1;
}

int main()
{
  int grid[25] =
  {
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    0, 0, 0, 0, 0,
    0, 1, 1, 1, 1,
    0, 0, 0, 0, 0
  };

  std::vector<POINT> path;
  if (!AStarPathfinder::FindPath(0, 0, 4, 4, 5, 5, IsBlocked, grid, path))
    return Fail("expected path to exist");

  if (path.empty())
    return Fail("expected non-empty path");

  if (path[0].x != 1 || path[0].y != 0)
    return Fail("expected first step to move along valid shortest route");

  POINT last = path[path.size() - 1];
  if (last.x != 4 || last.y != 4)
    return Fail("expected path to end at goal");

  for (size_t i = 0; i < path.size(); i++)
  {
    if (IsBlocked(path[i].y, path[i].x, grid))
      return Fail("path crossed blocked tile");
  }

  printf("PASS\n");
  return 0;
}
