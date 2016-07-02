rem Automate build of BIOSLIB/serial_flash_boot_loader.lib
rem Tom Collins - July 2009
rem Requires g++ (GNU C++ Compiler) and make (via cygwin or MinGW)

echo "Updating coldload_serflash_boot.bin"
cd ..\..\DistribCommon
dccl_cmp ..\ColdBoot\coldload_serflash_boot.c -pf ..\ColdBoot\compile_coldload_serflash_boot.dcp
cd ..\ColdBoot\triplets
echo "Updating BL_Triplets.exe"
make
echo "Building serial_flash_boot_loader.lib"
BL_triplets.exe ..\COLDLOAD_serflash_boot ..\..\DistribCommon\Lib\Rabbit4000\BIOSLIB\serial_flash_boot_loader.lib