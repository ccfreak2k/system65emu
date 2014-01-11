system65emu: An emulator for the System65 6502-based computer system

This is system65emu, an emulator for a fictional computer system based on the
6502 CPU.

To build system65emu, you will need SFML. You will then need to link against:

 * sfml-graphics
 * sfml-window
 * sfml-system

in that order (if using gcc).

Also included is a test program, named adctest.asm. This test program is used to
test that the ADC and SBC instructions function correctly. It can be assembled
with any 6502 assembler. I assemble it using cc65, a 6502-compatible toolchain.
There is currently no makefile for assembling it, but if you are using cc65, you
can run these two steps to generate a suitable binary:

    ca65 -DERROR=$8000 -DU1=$8002 -DU2=$8004 -DS1=$8006 -DS2=$8008 -W2 -l \
	--cpu 6502 -t none -o adctest.o adctest.asm
	ld65 -C system65.cfg -t none -m adctest.map -o adctest.bin adctest.o

The emulator will automatically locate the program to 0x0200 in memory.