#pragma once

#define DEGREES_PER_RAD 57.2958

/* returns the arctan in integer degrees */
uint16_t px_iatan2(uint16_t y, uint16_t x) {
  double rads = atan2(y, x);
  return static_cast<uint16_t>(rads * DEGREES_PER_RAD);
}

/* returns the square of a number */
static float square(float a) {
  return a * a;
}
