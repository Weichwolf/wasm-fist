# =============================================================================
# dump.gdb -- gdb driver for the Armored Fist runtime oracle (QEMU gdb stub)
# =============================================================================
# Use with:  run_oracle.sh <floppy> <game> gdb      (QEMU frozen at reset, -s -S)
# then:      gdb -q -x tools/oracle/dump.gdb
#
# Proven facts wired in below (see README.md):
#   * The decrypted FIST.RUN loader/CRT runs at LINEAR base 0x10000000.
#     - decrypt stub jmps to entry 0x10000d9d
#     - it installs an INT 0Dh handler, gets PSP, builds the "AF-FD:..." tag,
#       then DOS-EXECs the engine  ->  int 0x21 (AX=4B00) at 0x10000e80
#       with the filename "FIST.DAT" at [0x10000c2c].
#     - post-mortem "Tracking Number: <tag>" printed at 0x10000da2, exit at 0x10000b6e.
#   * The 16-bit engine (FIST.DAT) entry signature is  b8 00 1c 8e d8
#     (mov ax,0x1c00 ; mov ds,ax) at engine-image offset 4; DGROUP = seg 0x1c00.
#
# gdb addresses here are LINEAR (CS.base + EIP). QEMU exposes physical==linear
# for un-paged ranges; for paged ranges gdb follows the current CPU's CR3.
# Prefer the QEMU monitor `pmemsave 0 0x2000000 ram.bin` for a full *physical*
# snapshot (see qmon.py / poller.py) when you need all RAM regardless of paging.
# =============================================================================
set pagination off
set confirm off
set architecture i386
target remote :1234

# ---- reusable helpers -------------------------------------------------------
# dumplin ADDR LEN FILE : dump LEN bytes of (current-context) memory at ADDR
define dumplin
  dump binary memory $arg2 $arg0 ($arg0 + $arg1)
  printf "dumped %d bytes @ %#x -> %s\n", $arg1, $arg0, "$arg2"
end
document dumplin
  dumplin ADDR LEN FILE  -- dump LEN bytes from linear ADDR to FILE
end

# findsig : locate the 16-bit engine entry signature in the low 32 MB
define findsig
  find /1 0x00001000, 0x02000000, 0xb8, 0x00, 0x1c, 0x8e, 0xd8
end

# atexec : break where the loader DOS-EXECs the engine, print the filename
define atexec
  hbreak *0x10000e80
  continue
  printf "EXEC filename: "
  x/s 0x10000c2c
end

# ---- default action: stop at the EXEC of FIST.DAT ---------------------------
echo \n[oracle] connected to QEMU gdb stub. Loader base = 0x10000000.\n
echo [oracle] run 'atexec' to stop at the engine EXEC, 'findsig' to locate the engine,\n
echo [oracle] 'dumplin ADDR LEN FILE' to capture memory. See README.md.\n
