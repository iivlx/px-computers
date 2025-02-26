import struct

def encodeFloat(f):
    """Convert a Python float to 16-bit half-precision floating point (IEEE 754) as hex"""
    f32 = struct.unpack('>I', struct.pack('>f', f))[0]  # convert to IEEE 754 32-bit float
    sign = (f32 >> 31) & 0x1
    exponent = (f32 >> 23) & 0xFF
    mantissa = f32 & 0x7FFFFF

    # convert to half-precision (16-bit)
    if exponent == 0xFF:  # special cases for Inf and NaN
        half_exponent = 0x1F
        half_mantissa = mantissa >> 13
        if mantissa:
            half_mantissa |= 0x200  # set the quiet NaN bit ?
    elif exponent > 0x8E:  # overflow to Inf
        half_exponent = 0x1F
        half_mantissa = 0
    elif exponent < 0x71:  # underflow to subnormal or zero
        if exponent < 0x67:
            half_exponent = 0
            half_mantissa = 0
        else:
            shift = 0x71 - exponent
            half_exponent = 0
            half_mantissa = (mantissa | 0x800000) >> (13 + shift)
    else:  # normalized
        half_exponent = exponent - 0x70
        half_mantissa = mantissa >> 13

    # pack into 16-bits
    half = (sign << 15) | (half_exponent << 10) | half_mantissa
    return half
    #return f"0x{half:04X}"  # 4-digit hex
