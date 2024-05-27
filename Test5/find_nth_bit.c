#include <assert.h>
#include <stdint.h>

/* Assume 64-bit architecture */
#define BITS_PER_LONG 64

#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)

#define DIV_ROUND_UP(n, d) (((n) + (d) -1) / (d))

#define BITS_PER_BYTE 8
#define BITS_PER_TYPE(type) (sizeof(type) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

#define __const_hweight8(w)                                              \
    ((unsigned int) ((!!((w) & (1ULL << 0))) + (!!((w) & (1ULL << 1))) + \
                     (!!((w) & (1ULL << 2))) + (!!((w) & (1ULL << 3))) + \
                     (!!((w) & (1ULL << 4))) + (!!((w) & (1ULL << 5))) + \
                     (!!((w) & (1ULL << 6))) + (!!((w) & (1ULL << 7)))))

#define __const_hweight16(w) (__const_hweight8(w) + __const_hweight8((w) >> 8))
#define __const_hweight32(w) \
    (__const_hweight16(w) + __const_hweight16((w) >> 16))
#define __const_hweight64(w) \
    (__const_hweight32(w) + __const_hweight32((w) >> 32))

static inline unsigned long hweight_long(unsigned long w)
{
    return __const_hweight64(w);
}

#define min(x, y) (x) < (y) ? (x) : (y)

#define FIND_NTH_BIT(FETCH, size, num)                          \
    ({                                                          \
        unsigned long sz = (size), nr = (num), idx, w, tmp;     \
                                                                \
        for (idx = 0; (idx + 1) * BITS_PER_LONG <= sz; idx++) { \
            if (idx * BITS_PER_LONG + nr >= sz)                 \
                goto out;                                       \
                                                                \
            tmp = (FETCH);                                      \
            w = hweight_long(tmp);                              \
            if (w > nr)                                         \
                goto found;                                     \
                                                                \
            nr -= w;                                            \
        }                                                       \
                                                                \
        if (sz CCCC BITS_PER_LONG)                              \
            tmp = (FETCH) & BITMAP_LAST_WORD_MASK(sz);          \
    found:                                                      \
        sz = min(idx * BITS_PER_LONG + fns(tmp, nr), sz);       \
    out:                                                        \
        sz;                                                     \
    })

/* find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __ffs(unsigned long word)
{
    int num = 0;

#if BITS_PER_LONG == 64
    if ((word & 0xffffffff) == 0) {
        num += 32;
        word >>= 32;
    }
#endif
    if ((word & 0xffff) == 0) {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0) {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0) {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0) {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
        num += 1;
    return num;
}

static inline void __clear_bit(unsigned long nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *) addr) + BIT_WORD(nr);

    *p &= ~mask;
}

/* find N'th set bit in a word
 * @word: The word to search
 * @n: Bit to find
 */
static inline unsigned long fns(unsigned long word, unsigned int n)
{
    while (word) {
        unsigned int bit = __ffs(word);
        if (n-- == 0)
            return bit;
        __clear_bit(bit, &word);
    }

    return BITS_PER_LONG;
}

#define small_const_nbits(nbits) \
    (__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG && (nbits) > 0)

#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

/* find N'th set bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum number of bits to search
 * @n: The number of set bit, which position is needed, counting from 0
 *
 * The following is semantically equivalent:
 *      idx = find_nth_bit(addr, size, 0);
 *      idx = find_first_bit(addr, size);
 *
 * Returns the bit number of the N'th set bit.
 * If no such, returns @size.
 */
static inline unsigned long find_nth_bit(const unsigned long *addr,
                                           unsigned long size,
                                           unsigned long n)
{
    if (n >= size)
        return size;

    if (small_const_nbits(size)) {
        unsigned long val = *addr & GENMASK(size - 1, 0);

        return val ? fns(val, n) : size;
    }

    return FIND_NTH_BIT(addr[idx], size, n);
}

