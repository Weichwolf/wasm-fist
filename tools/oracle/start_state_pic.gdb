set pagination off
set confirm off
break VGA_VerticalTimer
condition 1 PIC_Ticks < 40
commands 1
silent
printf "PIC_VERTICAL tick=%llu lag=%.12g bits=%x cycle=%d period=%.12g\n", PIC_Ticks, srv_lag, *(unsigned int *)&srv_lag, CPU_CycleMax-CPU_CycleLeft-CPU_Cycles, vga.draw.delay.vtotal
continue
end
run
