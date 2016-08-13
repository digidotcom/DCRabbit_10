@echo off
rem Automate build of BIOSLIB/serial_flash_boot_loader.lib
rem Tom Collins - July 2009
rem Requires g++ (GNU C++ Compiler) and make (via cygwin or MinGW)

echo Updating coldload_serflash_boot.bin
cd ..
..\dccl_cmp coldload_serflash_boot.c -pf compile_coldload_serflash_boot.dcp
cd triplets
echo Updating BL_Triplets.exe
g++ -O2 -Wall -o BL_triplets.exe BL_triplets.cpp
echo Building serial_flash_boot_loader.lib
BL_triplets.exe ..\COLDLOAD_serflash_boot ..\..\Lib\Rabbit4000\BIOSLIB\serial_flash_boot_loader.lib