// isqrt.c - Integer square root.

#include "utils/isqrt.h"

// Compute the integer square root of an integer.
//
// ulValue is the value whose square root is desired.
//
// This function will compute the integer square root of the given input
// value.  Since the value returned is also an integer, it is actually better
// defined as the largest integer whose square is less than or equal to the
// input value.
//
// Returns the square root of the input value.
unsigned long isqrt(unsigned long ulValue)
{
  unsigned long ulRem, ulRoot, ulIdx;
  //
  // Initialize the remainder and root to zero.
  //
  ulRem = 0;
  ulRoot = 0;

  //
  // Loop over the sixteen bits in the root.
  //
  for (ulIdx = 0; ulIdx < 16; ulIdx++)
  {
    // Shift the root up by a bit to make room for the new bit that is
    // about to be computed.
    ulRoot <<= 1;
    // Get two more bits from the input into the remainder.
    ulRem = ((ulRem << 2) + (ulValue >> 30));
    ulValue <<= 2;
    // Make the test root be 2n + 1.
    ulRoot++;

    // See if the root is greater than the remainder.
    if (ulRoot <= ulRem)
    {
      // Subtract the test root from the remainder.
      ulRem -= ulRoot;
      // Increment the root, setting the second LSB.
      ulRoot++;
    }
    else
    {
      // The root is greater than the remainder, so the new bit of the
      // root is actually zero.
      ulRoot--;
    }
  }

  // Return the computed root.
  return (ulRoot >> 1);
}
