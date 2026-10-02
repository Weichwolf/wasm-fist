/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2002-2010 The DOSBox Team.
 * 8259 register/request/service state recovered from DOSBox 0.74-3 pic.cpp.
 * One owner supplies both targets. The clock owns budget requeue; the CPU
 * owner supplies effective flags and constructs the accepted interrupt frame.
 * Existing cooperative PIT/driver routes still need that CPU integration.
 */
#include "fist_pic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned masked, active, inservice, vector;
} FistIrq;
typedef struct {
    unsigned icw_index, icw_words, single, auto_eoi, rotate_auto_eoi;
    unsigned read_service, special;
} FistPic;
static FistIrq irqs[16];
static FistPic pics[2];
static unsigned initialized, pending, active_irq = 255;
static const unsigned priority[16] = {0,1,2,8,9,10,11,12,13,14,15,3,4,5,6,7};
static const unsigned rank[17] = {0,1,2,11,12,13,14,15,3,4,5,6,7,8,9,10,16};
extern void fist_clock_pic_requeue(void);

static void initialize(void)
{
    if (initialized) return;
    for (unsigned i = 0; i < 16; ++i) {
        irqs[i].masked = 1;
        irqs[i].vector = i < 8 ? 8+i : 0x70+i-8;
    }
    irqs[0].masked = irqs[1].masked = irqs[2].masked = irqs[8].masked = 0;
    initialized = 1;
}
static void unsupported(const char *reason)
{
    fprintf(stderr, "[pic] %s\n", reason);
    abort();
}
static void check_line(unsigned irq)
{
    if (irqs[irq].active && !irqs[irq].masked && (irq < 8 || !irqs[2].masked))
        pending |= 1u << irq;
    else pending &= ~(1u << irq);
}
static void recheck_cascade(unsigned previous)
{
    if (previous != irqs[2].masked)
        for (unsigned i = 8; i < 16; ++i) check_line(i);
}
void fist_pic_activate_irq(unsigned irq)
{
    initialize();
    if (irq < 16) { irqs[irq].active = 1; check_line(irq); }
}
void fist_pic_deactivate_irq(unsigned irq)
{
    initialize();
    if (irq < 16) { irqs[irq].active = 0; pending &= ~(1u << irq); }
}
void fist_pic_set_irq_mask(unsigned irq, int masked)
{
    initialize();
    assert(irq < 16);
    masked = !!masked;
    if (irqs[irq].masked == (unsigned)masked) return;
    unsigned previous = irqs[2].masked;
    irqs[irq].masked = masked;
    check_line(irq);
    recheck_cascade(previous);
    if (pending) fist_clock_pic_requeue();
}
int fist_pic_read(unsigned port)
{
    initialize();
    assert(port == 0x20 || port == 0x21 || port == 0xa0 || port == 0xa1);
    unsigned controller = port >= 0xa0, base = controller * 8, value = 0;
    for (unsigned i = 0; i < 8; ++i) {
        FistIrq *line = &irqs[base+i];
        unsigned bit = (port & 1) ? line->masked :
            pics[controller].read_service ? line->inservice : line->active;
        if (bit) value |= 1u << i;
    }
    if (port == 0x20 && !pics[0].read_service && (pending & 0xff00)) value |= 4;
    return value;
}
static void end_service(void)
{
    irqs[active_irq].inservice = 0;
    active_irq = 255;
    for (unsigned i = 0; i < 16; ++i)
        if (irqs[priority[i]].inservice) { active_irq = priority[i]; break; }
}
void fist_pic_write(unsigned port, unsigned value)
{
    initialize();
    assert(port == 0x20 || port == 0x21 || port == 0xa0 || port == 0xa1);
    value &= 255;
    unsigned controller = port >= 0xa0, base = controller * 8;
    FistPic *pic = &pics[controller];
    if (port & 1) {
        switch (pic->icw_index) {
        case 0: {
            unsigned previous = irqs[2].masked;
            for (unsigned i = 0; i < 8; ++i) {
                irqs[base+i].masked = !!(value & (1u << i));
                check_line(base+i);
            }
            recheck_cascade(previous);
            if (pending) fist_clock_pic_requeue();
            break;
        }
        case 1:
            for (unsigned i = 0; i < 8; ++i) irqs[base+i].vector = (value & 0xf8)+i;
            if (pic->icw_index++ >= pic->icw_words) pic->icw_index = 0;
            else if (pic->single) pic->icw_index = 3;
            break;
        case 2:
            if (pic->icw_index++ >= pic->icw_words) pic->icw_index = 0;
            break;
        case 3:
            pic->auto_eoi = !!(value & 2);
            if (!(value & 1)) unsupported("ICW4 8085 mode not handled");
            /* Original logs unsupported fully-nested mode and continues. */
            if (value & 0x10) fprintf(stderr, "[pic] special fully-nested mode not handled\n");
            if (pic->icw_index++ >= pic->icw_words) pic->icw_index = 0;
            break;
        }
    } else if (value & 0x10) {
        if (value & 4) unsupported("4-byte interval not handled");
        if (value & 8) unsupported("level-triggered mode not handled");
        if (value & 0xe0) unsupported("8080/8085 mode not handled");
        pic->single = !!(value & 2);
        pic->icw_index = 1;
        pic->icw_words = 2+(value & 1);
    } else if (value & 8) {
        if (value & 4) unsupported("poll command not handled");
        if (value & 2) pic->read_service = !!(value & 1);
        if (value & 0x40) {
            pic->special = !!(value & 0x20);
            if (pending) fist_clock_pic_requeue();
        }
    } else if (value & 0x20) {
        if (value & 0x80) unsupported("rotate mode not supported");
        if (value & 0x40) {
            if (active_irq == base+value-0x60) end_service();
        } else if (active_irq < base+8) end_service();
    } else if (!(value & 0x40)) pic->rotate_auto_eoi = !!(value & 0x80);
    else if (value & 0x80) fprintf(stderr, "[pic] set priority command not handled\n");
}
int fist_pic_take_irq(unsigned flags, int trap_decoder, unsigned *vector)
{
    initialize();
    if (!(flags & 0x200) || !pending || trap_decoder) return -1;
    unsigned current_rank = rank[active_irq == 255 ? 16 : active_irq];
    unsigned special = pics[0].special || pics[1].special;
    for (unsigned j = 0; j < (special ? 16 : current_rank); ++j) {
        unsigned i = priority[j];
        if (j >= current_rank && !pics[i/8].special) continue;
        if (irqs[i].masked || !irqs[i].active || (i > 7 && irqs[2].masked)) continue;
        irqs[i].active = 0;
        pending &= ~(1u << i);
        *vector = irqs[i].vector;
        if (!pics[i/8].auto_eoi) { active_irq = i; irqs[i].inservice = 1; }
        else if (pics[i/8].rotate_auto_eoi) unsupported("rotate on auto EOI not handled");
        return (int)i;
    }
    return -1;
}
