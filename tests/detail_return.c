#define main detail_service_backend_main
#include "detail_service.c"
#undef main

#ifdef FIST_TEST_DETAIL_LEGACY
/* The parent has no return packet. A fixture declaration permits observing its
 * reaching failure without supplying any missing producer or return value. */
typedef struct { uint32_t eax, ebx; } FistDetailRegisters;
#endif
FistDetailRegisters g_fist_detail_registers;
uint32_t g_fist_effects_eax;
void FUN_0000_6de2(uint32_t inbox);
static jmp_buf operation_boundary;
static unsigned gate_count;

static int observe_gateway(void) {
    uint16_t op = *(uint16_t *)(g_mem+0x2aa10);
    uint32_t inbox = *(uint32_t *)(g_mem+0x90000+0x3f2);
    if (op == 0x44) {
        ++gate_count;
        printf("op44 %08x\n", inbox);
        return detail_service_gate();
    }
    assert(op == 0x68 && gate_count == 1);
    printf("op68 %08x eax %08x config %u result %u dos-ebx %u sky %u mode %u\n",
           inbox, g_fist_effects_eax, *(uint16_t *)(g_mem+0x25fc8),
           *(uint32_t *)(g_mem+fist_ext_base+0x937), *(uint32_t *)(g_mem+0xf0024),
           *(uint32_t *)(g_mem+fist_ext_base+0x3958), g_mem[fist_ext_base+0x395c]);
    /* End at the actual op68 gateway entry. No adapter executes or invents the
     * missing effects/device service, its return, CPU flags or elapsed time. */
    longjmp(operation_boundary, 1);
}

code *fist_icall_far(uint32_t pointer) {
    assert((pointer>>16)*16+(pointer&65535) == FIST_EXTGATE_LIN);
    return (code *)observe_gateway;
}

int main(int argc, char **argv) {
    assert(argc == 9);
    unsigned sky = strtoul(argv[2], 0, 0), detail = strtoul(argv[3], 0, 0);
    unsigned first = strtoul(argv[4], 0, 0), second = strtoul(argv[5], 0, 0);
    unsigned peers = strtoul(argv[6], 0, 0);
    assert(peers <= 3);
    detail_service_prepare(argv[1], sky, detail);
    uint8_t *dg = g_mem+0x1c000;
    unsigned handles[3];
    memcpy(dg+0x5000, "PEER.DAT", 9);
    for (unsigned i=0; i<peers; ++i) {
        *(uint16_t *)(g_mem+0xf0000) = 0x3d00;
        *(uint16_t *)(g_mem+0xf0006) = 0x5000;
        *(uint16_t *)(g_mem+0xf000e) = 0x1c00;
        *(uint16_t *)(g_mem+0xf0014) = 0x21;
        fist_int_dispatch();
        assert(!*(uint16_t *)(g_mem+0xf0012));
        handles[i] = *(uint16_t *)(g_mem+0xf0000);
    }
    dg[0x8b46] = sky;
    *(uint16_t *)(dg+0x8b47) = detail;
    *(uint16_t *)(dg+0x8b49) = first;
    *(uint16_t *)(dg+0x8b4b) = second;
    *(uint32_t *)(dg+0xea16) = 0x07621179;
    *(uint16_t *)(g_mem+0x25fc8) = 0xa55a;
    g_fist_effects_eax = 0x89ab7654;
    g_fist_detail_registers.eax = g_fist_detail_registers.ebx = 0x89ab7654;
    count = 0;
    if (!setjmp(operation_boundary)) {
        FUN_0000_6de2(strtoul(argv[7], 0, 0));
        abort();
    }
    printf("dos");
    for (unsigned i=0; i<count; ++i) printf(" %02x", commands[i]);
    printf("\npeers");
    for (unsigned i=0; i<peers; ++i) {
        printf(" %u", handles[i]);
        *(uint16_t *)(g_mem+0xf0000) = 0x3f00;
        *(uint16_t *)(g_mem+0xf0002) = handles[i];
        *(uint16_t *)(g_mem+0xf0004) = 1;
        *(uint16_t *)(g_mem+0xf0006) = 0x5300;
        *(uint16_t *)(g_mem+0xf000e) = 0x1c00;
        *(uint16_t *)(g_mem+0xf0014) = 0x21;
        fist_int_dispatch();
        assert(!*(uint16_t *)(g_mem+0xf0012) && *(uint16_t *)(g_mem+0xf0000)==1);
        assert(dg[0x5300]=='S');
        *(uint16_t *)(g_mem+0xf0000) = 0x3e00;
        fist_int_dispatch();
        assert(!*(uint16_t *)(g_mem+0xf0012));
    }
    printf("\n");
    FILE *output = fopen(argv[8], "wb");
    assert(output && fwrite(g_mem+fist_ext_base+0x3a20-4,1,2052+8,output)==2052+8 && !fclose(output));
}
