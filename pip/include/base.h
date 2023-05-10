#pragma once

#include <algorithm>
#include <stdint.h>
#include <x86intrin.h>

#pragma region median selection

#pragma region minmax

using F32 = float_t;
using U32 = uint32_t;
using U16 = uint16_t;
using U8 = uint8_t;

template <U16 simdWidth, typename T> void MinMax(T &a, T &b);

template <> inline void MinMax<1, U8>(U8 &a, U8 &b) {
    U8 t = a;
    a = std::min(a, b);
    b = std::max(t, b);
}
template <> inline void MinMax<1, U16>(U16 &a, U16 &b) {
    U16 t = a;
    a = std::min(a, b);
    b = std::max(t, b);
}
template <> inline void MinMax<1, F32>(F32 &a, F32 &b) {
    F32 t = a;
    a = std::min(a, b);
    b = std::max(t, b);
}

// template <> inline void MinMax<64, __m512i>(__m512i &a, __m512i &b) {
//     __m512i t = a;
//     a = _mm512_min_epu8(a, b);
//     b = _mm512_max_epu8(b, t);
// }
// template <> inline void MinMax<32, __m512i>(__m512i &a, __m512i &b) {
//     __m512i t = a;
//     a = _mm512_min_epu16(a, b);
//     b = _mm512_max_epu16(b, t);
//     //__m512i t = _mm512_subs_epu16(a, b);
//     // a = _mm512_subs_epu16(a, t);
//     // b = _mm512_adds_epu16(b, t);
// }
// template <> inline void MinMax<16, __m512>(__m512 &a, __m512 &b) {
//     __m512 t = a;
//     a = _mm512_min_ps(a, b);
//     b = _mm512_max_ps(b, t);
// }

template <> inline void MinMax<32, __m256i>(__m256i &a, __m256i &b) {
    __m256i t = a;
    a = _mm256_min_epu8(a, b);
    b = _mm256_max_epu8(b, t);
}

template <> inline void MinMax<16, __m256i>(__m256i &a, __m256i &b) {
    __m256i t = _mm256_subs_epu16(a, b);
    a = _mm256_subs_epu16(a, t);
    b = _mm256_adds_epu16(b, t);
}

template <> inline void MinMax<8, __m256>(__m256 &a, __m256 &b) {
    __m256 t = a;
    a = _mm256_min_ps(a, b);
    b = _mm256_max_ps(b, t);
}

template <> inline void MinMax<16, __m128i>(__m128i &a, __m128i &b) {
    __m128i t = a;
    a = _mm_min_epu8(a, b);
    b = _mm_max_epu8(b, t);
}
template <> inline void MinMax<8, __m128i>(__m128i &a, __m128i &b) {
    __m128i t = _mm_subs_epu16(a, b);
    a = _mm_subs_epu16(a, t);
    b = _mm_adds_epu16(b, t);
}

template <> inline void MinMax<4, __m128>(__m128 &a, __m128 &b) {
    __m128 t = a;
    a = _mm_min_ps(a, b);
    b = _mm_max_ps(b, t);
}

#pragma endregion

// pop out extrma value to end of array
// ex: [2,5,1,7,9,6] => [1,x,x,x,x,9]
template <U16 simdWidth, typename T> void ExtremaToEnd(T *arr, U32 left, U32 right) {
    for (U32 i = left; i < right; i += 2)
        MinMax<simdWidth>(arr[i], arr[i + 1]);

    for (U32 step = 2; step < (right - left + 1); step *= 2) {
        for (U32 i = left; (i + step) <= right; i += 2 * step) {
            MinMax<simdWidth>(arr[i], arr[i + step]);
            MinMax<simdWidth>(arr[i + step - 1], arr[std::min(right, i + 2 * step - 1)]);
        }
    }
}

template <U16 simdWidth, typename T> U32 RankSelection(T *arr, U32 begin, U32 end, U32 rank) {
    U32 target = begin + rank;

    do {
        ExtremaToEnd<simdWidth>(arr, begin, end);
        ++begin, --end;
    } while (begin <= target && end >= target);

    return rank;
}

struct RankOp {
    const U16 rank;

    template <U16 simdWidth, typename T, typename U>
    inline void Compute(T *arr, U32 maskH, U32 maskW, U *out, U16 outWidth) {
        U32 loc = RankSelection<simdWidth>(arr, 0, maskH * maskW - 1, rank - 1);
        *(T *)out = arr[loc];
    }
};

#pragma endregion