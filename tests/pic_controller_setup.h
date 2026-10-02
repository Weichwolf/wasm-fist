/* Matched pre-entry fixture using real DMA/DSP completion and PIC acceptance.
 * Initialization is outside guest instruction time, shared by both endpoints.
 */
static void pic_controller_setup_sb_irq(void (*write_pic)(unsigned,unsigned),
                                       int (*take_irq)(unsigned,int,unsigned *))
{
    static const unsigned writes[][2] = {
        {0xc,0},{2,0xe0},{2,0x2d},{0x83,0},{3,0xff},{3,7},{0xb,0x59},{0xa,1},
        {0x22c,0x40},{0x22c,0xa6},{0x22c,0xd1},{0x22c,0x48},
        {0x22c,0xff},{0x22c,3},{0x22c,0x1c}
    };
    write_pic(0x21,0x78);
    for (unsigned i=0;i<sizeof writes/sizeof *writes;++i)
        fist_sb_out(writes[i][0],writes[i][1]);
    unsigned char data[1024];
    assert(fist_sb_read_pcm8(sizeof data,data)==sizeof data);
    unsigned vector;
    assert(take_irq(0x3202,0,&vector)==7 && vector==15);
}
