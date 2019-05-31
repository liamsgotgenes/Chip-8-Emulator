# Chip-8-Emulator
An emulator of the CHIP-8, 8-bit microcomputer, written in C. 

This emulator implements a 4 KB memory space, 16 byte data register, a 2 byte instruction register, 32 byte stack and a 64*32 byte graphics buffer.

The CHIP-8 interpertered language contains 35 operation codes, all of which are two bytes.

A further, more in-depth description of the system can be found here: https://en.wikipedia.org/wiki/CHIP-8
Games/ROMS can be downloaded here: https://github.com/dmatlack/chip8/tree/master/roms

Require SDL to run. 
`sudo apt-get install libsdl-dev1.2` for Debian based distros.
`sudo yum install libsdl-dev1.2` for Red Hat based distros.
