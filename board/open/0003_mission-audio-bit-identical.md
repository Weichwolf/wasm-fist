Type: feature
Area: audio

The mission + menu audio stream (SB/GUS + OPL, driven by the ported sound
engine) is bit-identical to the original given the same input and RNG state,
captured and compared against a DOSBox/QEMU reference and driven by a
tools/verify.sh flow. Currently unmeasured — no audio flow exists in the matrix.
