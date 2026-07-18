/* Minimal DOSBox-compat shim so the UNMODIFIED DOSBox 0.74-3 dbopl.cpp/.h compile standalone for the
 * Armored Fist port's OPL emulation.  Only the types/macros dbopl.{cpp,h} actually reference. */
#ifndef FIST_OPL_DOSBOX_COMPAT_H
#define FIST_OPL_DOSBOX_COMPAT_H
#include <stdint.h>
typedef uint8_t   Bit8u;
typedef int8_t    Bit8s;
typedef uint16_t  Bit16u;
typedef int16_t   Bit16s;
typedef uint32_t  Bit32u;
typedef int32_t   Bit32s;
typedef uint64_t  Bit64u;
typedef int64_t   Bit64s;
typedef uintptr_t Bitu;
typedef intptr_t  Bits;
#ifndef INLINE
#define INLINE inline
#endif
#ifndef DB_FASTCALL
#define DB_FASTCALL
#endif
#ifndef GCC_UNLIKELY
#define GCC_UNLIKELY(x) (x)
#endif
#ifndef GCC_LIKELY
#define GCC_LIKELY(x) (x)
#endif
#endif
