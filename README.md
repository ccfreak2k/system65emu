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

    ca65 -DERROR=$8000 -DU1=$8002 -DU2=$8004 -DS1=$8006 -DS2=$8008 -W2 \
	-l adctest.lst --cpu 6502 -t none -o adctest.o adctest.asm
	ld65 -t none -m adctest.map -o adctest.bin adctest.o

The emulator will automatically locate the program to 0x0200 in memory.

For emulator validation, "6502_functional_test.a65" and
"6502_functional_test_new.a65" are used for testing. To run them, you'll need
as65, found here: http://www.kingswood-consulting.co.uk/assemblers/
Then assemble the .a65 file(s) with this command (ignore the command in the
assembly file itself):

	as65 -l -m -w -h0 6502_functional_test_new.a65

This will produce a listing file (.lst), which you can use to track the
execution, and a program file (.bin) which is to be loaded in the emulator, as
above.