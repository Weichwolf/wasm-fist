/* Stub of DOSBox adlib.h for standalone dbopl compile: dbopl.h's DBOPL::Handler derives from
 * Adlib::Handler and its Generate() takes a MixerChannel*.  We never instantiate DBOPL::Handler
 * (the port drives DBOPL::Chip directly), so trivial stubs suffice + keep dbopl.{cpp,h} unmodified. */
#ifndef FIST_OPL_ADLIB_COMPAT_H
#define FIST_OPL_ADLIB_COMPAT_H
#include "dosbox.h"
struct MixerChannel {
    void AddSamples_m32(Bitu /*len*/, Bit32s* /*data*/) {}
    void AddSamples_s32(Bitu /*len*/, Bit32s* /*data*/) {}
};
namespace Adlib { struct Handler { virtual ~Handler() {} }; }
#endif
