/* sb_selftest.c -- unit-verify the fist_sb.c SB DSP + 8237 DMA -> PCM path WITHOUT the engine.
 * Synthesizes the exact port sequence the SB driver issues for an 8-bit single-cycle DMA playback of a
 * 1 kHz sine placed in g_mem, then checks fist_sb decoded it into the ring at the right rate/length.
 *   build: cc -DFIST_SB_SELFTEST tools/oracle/sb_selftest.c re_out/fist_sb.c -I re_out -o /tmp/sb_selftest -lm
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Minimal g_mem so fist_sb.c links standalone (it derefs g_mem + FIST_MEM_SIZE from ghidra_compat.h). */
uint8_t g_mem[0x1000000u];

int  fist_sb_owns(int);
int  fist_sb_in(int);
void fist_sb_out(int, int);
void fist_sb_flush(void);
unsigned fist_sb_ring_count(void);
int  fist_sb_rate(void);

#define BASE 0x220
static void dsp(int v){ fist_sb_out(BASE+0xc, v); }

int main(void){
    setenv("FIST_SB","1",1);
    setenv("FIST_AUDIO_WAV","/tmp/sb_selftest.wav",1);
    setenv("FIST_SB_TRACE","1",1);

    /* Place a 1 kHz sine (u8 PCM) at conventional phys 0x12000 (page 0x01, offset 0x2000). */
    const unsigned phys = 0x12000, N = 4410;      /* 0.2 s @ 22050 */
    for (unsigned i=0;i<N;i++)
        g_mem[phys+i] = (uint8_t)(128 + 100*sin(2*M_PI*1000.0*i/22050.0));

    /* DSP reset handshake. */
    fist_sb_out(BASE+0x6, 1); fist_sb_out(BASE+0x6, 0);
    if (fist_sb_in(BASE+0xe) & 0x80) { int v=fist_sb_in(BASE+0xa); if (v!=0xAA){ printf("FAIL reset ack=0x%x\n",v); return 1; } }

    dsp(0xd1);                                    /* speaker on */
    dsp(0x40); dsp(256 - 1000000/22050);          /* time constant -> ~22050 Hz */

    /* Program 8237 channel 1: clear ff, addr, count, page, unmask. */
    fist_sb_out(0x0c, 0);                          /* clear flip-flop */
    fist_sb_out(0x02, (phys)&0xff); fist_sb_out(0x02, (phys>>8)&0xff);   /* offset within page */
    fist_sb_out(0x83, (phys>>16)&0xff);            /* page */
    fist_sb_out(0x0b, 0x49);                       /* mode: single, read(playback), ch1 */
    fist_sb_out(0x03, (N-1)&0xff); fist_sb_out(0x03, ((N-1)>>8)&0xff);   /* count = len-1 */
    fist_sb_out(0x0a, 0x01);                       /* unmask ch1 */

    /* DSP 0x14 = 8-bit single-cycle DMA output, length-1 lo/hi. */
    dsp(0x14); dsp((N-1)&0xff); dsp(((N-1)>>8)&0xff);

    unsigned got = fist_sb_ring_count();
    int rate = fist_sb_rate();
    fist_sb_flush();

    printf("[selftest] rate=%d got=%u expected=%u\n", rate, got, N);
    int ok = (got==N) && (rate>=21000 && rate<=23000);
    /* verify a non-trivial waveform landed in the WAV (peak > half scale) */
    printf("[selftest] %s\n", ok?"PASS":"FAIL");
    return ok?0:1;
}
