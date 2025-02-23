#pragma once

/* floating point functions */

/* IEEE 754.8 16-bit floating point */

#define NAN 0x7E00 // what is 0x7FFF vs 0x7E00
#define NEG_INFINITY 0xFC00
#define POS_INFINITY 0x7C00

///* Take a floating point value and return the encoded 16 bits in 754.8 hp*///
uint16_t encodeHalf(float value) {
  uint32_t bits = *reinterpret_cast<uint32_t*>(&value); // 32-bit

  uint16_t sign = (bits >> 31) & 0x1;                   // sign bit
  int32_t exponent = ((bits >> 23) & 0xFF) - 127;       // exponent and unbias from single-precision
  uint32_t mantissa = bits & 0x7FFFFF;                  // mantissa (23 bits)

  // Handle special cases
  if (exponent <= -15) { // underflow
    if (exponent < -24) {
      return sign << 15; // < e = 0
    }
    mantissa |= 0x800000;  //implicit leading 1
    mantissa >>= (-exponent - 14); // mantissa for denormalized
    return (sign << 15) | (mantissa >> 13);
  }
  else if (exponent > 15) { // overflow
    return (sign << 15) | 0x7C00; // exponent is max, mantissa is 0
  }

  // normalize
  uint16_t halfExponent = exponent + 15;   // re-bias
  uint16_t halfMantissa = mantissa >> 13;  // mantissa -> 10 bits
  return (sign << 15) | (halfExponent << 10) | halfMantissa;
}

///* Take a 16 bit half precision and return the floating point value *///
float decodeHalf(uint16_t half) {
  uint16_t sign = (half >> 15) & 0x1;         // sign bit           0b1000 0000 0000 0000
  uint16_t exponent = (half >> 10) & 0x1F;    // exponent (5 bits)  0b0111 1100 0000 0000
  uint16_t mantissa = half & 0x3FF;           // mantissa (10 bits) 0b0000 0011 1111 1111

  uint32_t resultBits = 0;

  if (exponent == 0) {
    if (mantissa == 0) {
      // Zero
      resultBits = sign << 31;
    }
    else {
      // Subnormal number
      float normalizedMantissa = mantissa / 1024.0f; // Divide by 2^10
      float value = std::ldexp(normalizedMantissa, -14); // ldexp(m, e) = m * 2^e
      return sign ? -value : value;
    }
  }
  else if (exponent == 0x1F) {
    // Infinity or NaN
    resultBits = (sign << 31) | 0x7F800000 | (mantissa << 13);
  }
  else {
    // Normalized number
    uint32_t singleExponent = exponent + (127 - 15); // Re-bias exponent
    uint32_t singleMantissa = mantissa << 13;        // Align mantissa to 23 bits
    resultBits = (sign << 31) | (singleExponent << 23) | singleMantissa;
  }

  return *reinterpret_cast<float*>(&resultBits);
}

/* helper functions */

/* Take a 16 bit floating point value and convert it into a 16 bit integer value */
uint16_t halfToInt(uint16_t word) {
  uint16_t intv = decodeHalf(word);
  return intv;
}

/* Take a 16 bit integer value and convert it into a 16 bit floating point value */
uint16_t intToHalf(uint16_t word) {
  float fpv = static_cast<float>(word); // convert int to float
  return encodeHalf(fpv);
}