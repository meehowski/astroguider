/*
 * CFile1.c
 *
 * Created: 06/11/2012 11:59:30 AM
 *  Author: michal
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

int32_t modulus_i(int32_t value, int32_t divisor)
{
  int32_t tmp;
  tmp = value / divisor;
  tmp = tmp - (value < 0);
  tmp = tmp * divisor;
  tmp = value - tmp;
  return (tmp);
}

double modulus_d(double value, double divisor)
{
  return (value - divisor * floor(value / divisor));
}

double abs_d(double value)
{
  if (value < 0)
  {
    return (-value);
  }

  return (+value);
}

double sign_d(double value)
{
  if (value < 0)
  {
    return (-1);
  }
  else if (value > 0)
  {
    return (+1);
  }

  return (0);
}

int8_t sign_i(double value)
{
  if (value < 0)
  {
    return (-1);
  }
  else if (value > 0)
  {
    return (+1);
  }

  return (0);
}

// 182,180=-178
// 350,180=-10
// normalizes a value to range of -limit ... +limit
double two_compl_d(double value, double limit)
{
  value = modulus_d(value, 2 * limit);

  if (value >= limit)
  {
    return (value - 2 * limit);
  }

  return (value);
}

int32_t two_compl_i(int32_t value, int32_t limit)
{
  value = modulus_i(value, 2 * limit);

  if (value >= limit)
  {
    return (value - 2 * limit);
  }

  return (value);
}

double limit_d(double value, double limit_low, double limit_high)
{
  if (value < limit_low)
  {
    return (limit_low);
  }
  else if (value > limit_high)
  {
    return (limit_high);
  }

  return (value);
}

int32_t limit_i(int32_t value, int32_t limit_low, int32_t limit_high)
{
  if (value < limit_low)
  {
    return (limit_low);
  }
  else if (value > limit_high)
  {
    return (limit_high);
  }

  return (value);
}

