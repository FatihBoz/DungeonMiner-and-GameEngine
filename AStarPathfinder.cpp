#include "AStarPathfinder.h"
#include <algorithm>
#include <stdlib.h>

struct AStarNode
{
  int row;
  int col;
  int g;
  int h;
  int parent;
  BOOL open;
  BOOL closed;
};

static int GetNodeIndex(int row, int col, int cols)
{
  return row * cols + col;
}

static int GetDistance(int rowA, int colA, int rowB, int colB)
{
  return abs(rowA - rowB) + abs(colA - colB);
}

static BOOL IsInside(int row, int col, int rows, int cols)
{
  return row >= 0 && row < rows && col >= 0 && col < cols;
}

static int FindBestOpenNode(const std::vector<AStarNode>& vNodes)
{
  int best = -1;

  for (size_t i = 0; i < vNodes.size(); i++)
  {
    if (!vNodes[i].open || vNodes[i].closed)
      continue;

    if (best < 0 ||
      (vNodes[i].g + vNodes[i].h) < (vNodes[best].g + vNodes[best].h) ||
      ((vNodes[i].g + vNodes[i].h) == (vNodes[best].g + vNodes[best].h) &&
      vNodes[i].h < vNodes[best].h))
      best = (int)i;
  }

  return best;
}

static void BuildPath(const std::vector<AStarNode>& vNodes, int goalIndex,
  int startIndex, std::vector<POINT>& vPath)
{
  std::vector<POINT> vReversePath;
  int index = goalIndex;

  while (index >= 0 && index != startIndex)
  {
    POINT pt = { vNodes[index].col, vNodes[index].row };
    vReversePath.push_back(pt);
    index = vNodes[index].parent;
  }

  vPath.clear();
  for (std::vector<POINT>::reverse_iterator it = vReversePath.rbegin();
    it != vReversePath.rend(); ++it)
    vPath.push_back(*it);
}

BOOL AStarPathfinder::FindPath(int iStartRow, int iStartCol, int iGoalRow,
  int iGoalCol, int iRows, int iCols, AStarTileBlockedFn pBlockedFn,
  void* pContext, std::vector<POINT>& vPath)
{
  vPath.clear();

  if (iRows <= 0 || iCols <= 0 || pBlockedFn == NULL)
    return FALSE;

  if (!IsInside(iStartRow, iStartCol, iRows, iCols) ||
    !IsInside(iGoalRow, iGoalCol, iRows, iCols))
    return FALSE;

  if (pBlockedFn(iStartRow, iStartCol, pContext) ||
    pBlockedFn(iGoalRow, iGoalCol, pContext))
    return FALSE;

  if (iStartRow == iGoalRow && iStartCol == iGoalCol)
    return TRUE;

  std::vector<AStarNode> vNodes;
  vNodes.resize(iRows * iCols);

  for (int row = 0; row < iRows; row++)
  {
    for (int col = 0; col < iCols; col++)
    {
      int index = GetNodeIndex(row, col, iCols);
      vNodes[index].row = row;
      vNodes[index].col = col;
      vNodes[index].g = 999999;
      vNodes[index].h = GetDistance(row, col, iGoalRow, iGoalCol);
      vNodes[index].parent = -1;
      vNodes[index].open = FALSE;
      vNodes[index].closed = FALSE;
    }
  }

  int startIndex = GetNodeIndex(iStartRow, iStartCol, iCols);
  int goalIndex = GetNodeIndex(iGoalRow, iGoalCol, iCols);
  vNodes[startIndex].g = 0;
  vNodes[startIndex].open = TRUE;

  const int dRow[4] = { 0, 0, -1, 1 };
  const int dCol[4] = { 1, -1, 0, 0 };

  while (TRUE)
  {
    int currentIndex = FindBestOpenNode(vNodes);
    if (currentIndex < 0)
      return FALSE;

    if (currentIndex == goalIndex)
    {
      BuildPath(vNodes, goalIndex, startIndex, vPath);
      return TRUE;
    }

    vNodes[currentIndex].open = FALSE;
    vNodes[currentIndex].closed = TRUE;

    for (int i = 0; i < 4; i++)
    {
      int nextRow = vNodes[currentIndex].row + dRow[i];
      int nextCol = vNodes[currentIndex].col + dCol[i];
      if (!IsInside(nextRow, nextCol, iRows, iCols))
        continue;

      if (pBlockedFn(nextRow, nextCol, pContext))
        continue;

      int nextIndex = GetNodeIndex(nextRow, nextCol, iCols);
      if (vNodes[nextIndex].closed)
        continue;

      int newG = vNodes[currentIndex].g + 1;
      if (!vNodes[nextIndex].open || newG < vNodes[nextIndex].g)
      {
        vNodes[nextIndex].g = newG;
        vNodes[nextIndex].parent = currentIndex;
        vNodes[nextIndex].open = TRUE;
      }
    }
  }
}
