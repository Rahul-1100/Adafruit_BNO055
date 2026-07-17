#ifndef QUANTIZE_H
#define QUANTIZE_H

#include <stdint.h>
#include <cmath>
#include <cstring>

struct quantize
{
    float scale;
    int8_t zero_point;

    quantize(float scale_value = 1.0f, int8_t zero_point_value = 0)
        : scale(scale_value), zero_point(zero_point_value) {}
        
    quantize(float min_range, float max_range, int8_t zero_point_value =0):zero_point(zero_point_value){
        scale = (max_range - min_range) / 255.0f;
    }

    static float halfToFloat(uint16_t h)
    {
        uint16_t sign = (h >> 15) & 0x1;
        uint16_t exp = (h >> 10) & 0x1f;
        uint16_t mant = h & 0x3ff;

        uint32_t f;
        if (exp == 0) {
            if (mant == 0) {
                f = (uint32_t)sign << 31;
            } else {
                while ((mant & 0x400) == 0) {
                    mant <<= 1;
                    exp--;
                }
                exp++;
                mant &= 0x3ff;
                uint32_t exp32 = (uint32_t)(exp + (127 - 15));
                uint32_t mant32 = (uint32_t)mant << 13;
                f = ((uint32_t)sign << 31) | (exp32 << 23) | mant32;
            }
        } else if (exp == 0x1f) {
            f = ((uint32_t)sign << 31) | 0x7f800000u | ((uint32_t)mant << 13);
        } else {
            uint32_t exp32 = (uint32_t)(exp + (127 - 15));
            uint32_t mant32 = (uint32_t)mant << 13;
            f = ((uint32_t)sign << 31) | (exp32 << 23) | mant32;
        }

        float result;
        std::memcpy(&result, &f, sizeof(result));
        return result;
    }

    int8_t operator()(uint16_t half) const
    {
        float value = halfToFloat(half);
        int32_t quantized = static_cast<int32_t>(std::round(value / scale)) + zero_point;
        if (quantized > 127) quantized = 127;
        else if (quantized < -128) quantized = -128;
        return static_cast<int8_t>(quantized);
    }

    static int8_t fromHalf(uint16_t half, float scale = 1.0f, int8_t zero_point = 0)
    {
        return quantize(scale, zero_point)(half);
    }
};

#endif // QUANTIZE_H