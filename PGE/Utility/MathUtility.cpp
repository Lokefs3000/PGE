#include "pch.h"
#include "MathUtility.hpp"

uint32_t pge::MathUtility::RoundToPow2(uint32_t value)
{
    if (value == 0) return 0;

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;

    return value;
}
