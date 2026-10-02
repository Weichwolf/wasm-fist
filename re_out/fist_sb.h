#ifndef FIST_SB_H
#define FIST_SB_H

int fist_sb_enabled(void);
int fist_sb_owns(int port);
int fist_sb_in(int port);
void fist_sb_out(int port, int val);
void fist_sb_set_irq_cb(void (*cb)(void));
void fist_sb_pump(void);
/* Mono unsigned PCM8 input to the mixer. Demand is in DMA bytes, not milliseconds.
 * The caller provides 65536 bytes (single-cycle end demand may round up).
 * No output is consumed by starting DMA. */
unsigned fist_sb_read_pcm8(unsigned want, unsigned char *data);
unsigned fist_sb_dma_left(void);
unsigned fist_sb_ring_count(void);
int fist_sb_rate(void);
void fist_sb_flush(void);

#endif
