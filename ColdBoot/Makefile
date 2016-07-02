# Make file for Cold Loaders and Pilots
# Original version created December 2009 by Tom Collins

# Designed for Borland MAKE Version 5.2

# touch precoldload.c - tell make that precoldload.bin is stale and needs to be rebuilt
# make precoldload.bin - rebuild a single .bin file
# make all - rebuild all .bin files using Dynamic C's command-line compiler
# make install - copy .bin files to DistribCommon's BIOS directory, tripletting as necessary
# 	make install also builds BIOSLIB\serial_flash_boot_loader.lib
# make clean - delete all .bin files created from previous builds

# Changes to a .c file will cause MAKE to rebuild the .bin file.  
# Changes to a .dcp file won't trigger a rebuild of the .bin file.

.noautodepend

DCCL=..\DistribCommon\dccl_cmp.exe
TRIP=triplets\triplets.exe
BLTRIP=triplets\BL_triplets.exe
BIOSDIR=..\DistribCommon\BIOS
BIOSLIB=..\DistribCommon\Lib\Rabbit4000\BIOSLIB

# list of coldloaders that need to be tripletted
COLD=precoldload.bin checkramcs0.bin checkcs04mem.bin coldload.bin coldload16bit.bin coldloadserflash.bin

# list of pilots that are copied directly
PILOT=pilot.bin pilot16bit.bin pilotserflash.bin

# default target -- build all .bin files
all : $(COLD) $(PILOT) coldload_serflash_boot.bin

# explicit cases first -- .c files with .dcp files of a non-standard name
checkcs04mem.bin : checkcs04mem.c compile_coldload.dcp
	$(DCCL) $*.c -pf compile_coldload.dcp

coldloadserflash.bin : coldloadserflash.c compile_coldload_serflash.dcp
	$(DCCL) $*.c -pf compile_coldload_serflash.dcp

pilot16bit.bin : pilot16bit.c compile_pilot16.dcp
	$(DCCL) $*.c -pf compile_pilot16.dcp
	
pilotserflash.bin : pilotserflash.c pilotserflash.dcp
	$(DCCL) $*.c -pf pilotserflash.dcp
	
# create dependencies on correct .dcp files for default build rule
checkramcs0.bin : compile_checkramcs0.dcp
coldload.bin : compile_coldload.dcp
coldload16bit.bin : compile_coldload16bit.dcp
coldload_serflash_boot.bin : compile_coldload_serflash_boot.dcp
pilot.bin : compile_pilot.dcp
precoldload.bin : compile_precoldload.dcp

# implicit rule for building a .bin from a .c/.dcp with the same name
.c.bin : 
	$(DCCL) $< -pf compile_$*.dcp

clean :
	del *.bin
	
install : all
	# the leading - means to ignore errors
	-for %f in ($(PILOT)) do copy %f $(BIOSDIR)\%f
	-for %f in ($(COLD)) do $(TRIP) %~nf $(BIOSDIR)\%~nf
	-$(BLTRIP) COLDLOAD_serflash_boot $(BIOSLIB)\serial_flash_boot_loader.lib
