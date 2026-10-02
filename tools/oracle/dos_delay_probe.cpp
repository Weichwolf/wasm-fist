/* Tests extract this helper verbatim from the pinned DOSBox dos.cpp into their build directory. */
#include "cpu.h"
#include "dos_modify_cycles.h"

void source_dos_transfer(unsigned value) { modify_cycles(value); }
