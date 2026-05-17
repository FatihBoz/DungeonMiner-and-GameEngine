#include "../SkeletonProjectileMath.h"
#include <stdio.h>

static int Fail(const char* szMessage)
{
  printf("FAIL: %s\n", szMessage);
  return 1;
}

int main()
{
  POINT ptStep = ComputeStraightLineStep({ 0, 0 }, { 8, 4 }, 6);
  if (ptStep.x <= 0 || ptStep.y <= 0)
    return Fail("expected positive diagonal step");

  POINT ptAxis = ComputeStraightLineStep({ 10, 10 }, { 10, 30 }, 6);
  if (ptAxis.x != 0 || ptAxis.y <= 0)
    return Fail("expected vertical step");

  POINT ptZero = ComputeStraightLineStep({ 5, 5 }, { 5, 5 }, 6);
  if (ptZero.x != 0 || ptZero.y != 0)
    return Fail("expected zero step when aligned");

  printf("PASS\n");
  return 0;
}
