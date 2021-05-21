#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>

static inline uint8_t get_bit(uint8_t array[], uint32_t bit) {
    return (array[bit / 8] >> (bit % 8)) & 1;
}

static inline void set_bit(uint8_t array[], uint32_t bit) {
    array[bit / 8] |= (1 << (bit % 8));
}

static inline void clr_bit(uint8_t array[], uint32_t bit) {
    array[bit / 8] &= ~(1 << (bit % 8));
}

#endif
