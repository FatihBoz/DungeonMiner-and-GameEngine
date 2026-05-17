#include "SkeletonProjectileMath.h"
#include <stdlib.h>

POINT ComputeStraightLineStep(POINT ptFrom, POINT ptTo, int iSpeed)
{
  POINT ptStep = { 0, 0 };
  int dx = ptTo.x - ptFrom.x;
  int dy = ptTo.y - ptFrom.y;

  if (dx == 0 && dy == 0 || iSpeed <= 0)
    return ptStep;

  if (abs(dx) > abs(dy))
  {
    ptStep.x = (dx > 0) ? iSpeed : -iSpeed;
    if (dy != 0)
      ptStep.y = (int)((double)dy / (double)abs(dx) * (double)iSpeed);
  }
  else if (abs(dy) > abs(dx))
  {
    ptStep.y = (dy > 0) ? iSpeed : -iSpeed;
    if (dx != 0)
      ptStep.x = (int)((double)dx / (double)abs(dy) * (double)iSpeed);
  }
  else
  {
    ptStep.x = (dx > 0) ? iSpeed : -iSpeed;
    ptStep.y = (dy > 0) ? iSpeed : -iSpeed;
  }

  if (ptStep.x == 0 && dx != 0)
    ptStep.x = (dx > 0) ? 1 : -1;
  if (ptStep.y == 0 && dy != 0)
    ptStep.y = (dy > 0) ? 1 : -1;

  return ptStep;
}
