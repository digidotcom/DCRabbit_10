This directory contains the source code for the binary files found in
`BIOS/`.  Use `build.bat` to compile a single file at a time.  If you
have Borland MAKE Version 5.2 installed, the `Makefile` should work for
building and installing all binary files.

The following attempts to document the "cold boot" sequence Dynamic C 10
(DC) and the Rabbit Field Utility (RFU) use on Rabbit 4000, 5000 and 6000
boards.  Read the processor documentation for details on bootstrapping
from serial port A.

DC and RFU reset the processor with the following sequence:

1. Set pins `SMODE1=1` and `SMODE0=1` to enable asynchronous serial
   bootstrapping at 2400 baud.
2. Hold `/RESET` low (via DTR pin of serial connection) for 500ms.
3. Set `/RESET` high and wait another 500ms.
4. Send a triplet to set `GOCR=0x30`, pulling `STATUS` high.

The compiler and RFU monitor the processor's `STATUS` line via the `DSR`
pin of the serial connection, and use small programs to discover target
capabilities and toggle the `STATUS` pin in response.  Each detection
routine typically starts by setting `GOCR = 0x20` and verifies that
`STATUS` goes low, then checks whether `STATUS` goes high.

1. If processor verification is turned on, make sure a Rabbit is connected
   to the programming cable, using the following sequence (sent as
   triplets: `0x80`, the register address, and the register value):
   - `WDTTR = 0x51`
   - `WDTTR = 0x54`
   - `GOCR = 0x30` (expect `STATUS` to be high)
   - `GOCR = 0x20` (expect `STATUS` to be low)
2. Probe for internal RAM on `/CS3` by setting `MB0CR` to `0x43`, then
   loading a small program at address 0 (`ld a, 0x30; ioi ld (GOCR), a;
   jr -2`) and starting it.  If `STATUS` goes low, the board has RAM
   on `/CS3` and is either a Rabbit 5000 or Rabbit 6000.  Note that
   including `Detect Internal Memory=0` in the project file will skip
   this step and force the use of RAM on `/CS1` for bootstrapping.
3. On a Rabbit 4000, detect memory type (either 8-bit or 16-bit) using
   `precoldload.bin`.  DC/RFU skips this step on Rabbit 5000/6000 since
   it will use the internal RAM with an 8-bit interface.
4. Detect flash type using `checkRamCS0.bin`.  A Rabbit 4000 with 16-bit
   memory skips this step and assumes parallel flash.  Other boards
   configure `MB2CR` for `CS0/OE0` which will map either parallel flash
   or RAM to that quadrant.  (Note that DC/RFU no longer make use of
   `checkCS04mem.bin`.)

After identifying characteristics of the hardware, DC/RFU sends the
appropriate cold loader to the target (via triplets):

- Parallel flash (8-bit RAM): `coldload.bin`
- Parallel flash (16-bit RAM): `coldload16.bin`
- Serial flash: `coldloadserflash.bin`

That cold loader can then receive the Pilot BIOS at 57600bps into RAM
starting at 0x6000, and jump into it to receive the regular BIOS and
program into RAM or flash.

- Parallel flash with 8-bit RAM: `pilot.bin`
- Parallel flash with 16-bit RAM: `pilot16.bin`
- Serial flash: `pilotserflash.bin`

(The `COLDLOAD_serflash_boot.c` program is the source of the triplets
stored in `Lib/Rabbit4000/BIOSLIB/serial_flash_boot_loader.lib` and
included in `Lib/Rabbit4000/BIOSLIB/StdBios.c` to bootstrap Rabbit 5000
and 6000 boards from a serial flash.)



