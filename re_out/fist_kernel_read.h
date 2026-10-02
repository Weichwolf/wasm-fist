/* Reached VCPI disk-read instruction blocks, not fitted elapsed delays.
 * FIST.RUN module offset = CS=0008/real-CS IP + 0xc9a. FIST.DAT wrapper
 * module offset = IP + 0xf690. MOV SS fetches are refunded separately.
 * Counts come from the complete original callback-88 instruction trace;
 * tools/oracle/kernel_read_cases.json records its hash and syscall boundaries. */
enum {
    K_READ_DISPATCH = 18, /* 197d..19ac: flags/segments/EAX, AH table dispatch */
    K_READ_SETUP = 12,    /* 18de..18fc: preserve registers, packet DS/FS, EDI=EDX */
    K_GATE_A = 22,        /* 1614..143a: VCPI->real, before MOV SS at 143d */
    K_GATE_B = 28,        /* 143f..1652, wrapper 2c5c..2c78, DOSBox STI 14a0 */
    K_DOS_RETURN = 3,     /* DOSBox IRET 14a5, wrapper DEC 2c7d / JC 2c82 */
    K_WRAPPER_OK = 9,     /* 2c84..2c9d: normal return, clear stacked CF */
    K_WRAPPER_ERROR = 5,  /* 2c9e..2ca6: set stacked CF */
    K_GATE_C = 30,        /* 1654..1553: real->VCPI, before MOV SS at 1558 */
    K_GATE_D = 15,        /* 155d..1572 / 1663..166c: restore regs/segments, RET */
    K_COPY_SETUP = 6,     /* 1923..1934: accumulate, preserve remaining/count */
    K_COPY_TAIL = 3,      /* 193c..1941: restore count, CL&3, zero extend */
    K_AFTER_COPY = 3,     /* 1948..194e: remaining -> ECX, test / JZ */
    K_READ_FINISH = 13    /* 195a..197b, excluding MOV EAX,EBP / XOR EBX,EBX */
};
