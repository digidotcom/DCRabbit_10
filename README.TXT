*****************************************************************
   README for Dynamic C Version 10.72
*****************************************************************

- FCC-labeled versions of RCM6600W family boards are locked to
  Americas region code.

- The Lib\Rabbit4000\NandFlash\nflash.lib driver library has been
  updated with support for Micron/Numonyx/ST NANDxxxW3A nand
  flash devices. Previously supported nand flash devices, used on
  RCM4000 and RCM4050 boards, have been discontinued and are no
  longer available. As of 01-Dec-2012, new production RCM4000 and
  RCM4050 boards will have a Micron/Numonyx/ST NAND256W3A (32MB)
  nand flash device installed.

  Custom applications which use nand flash, whether based on
  RCM4000, RCM4050 boards or a custom nand flash board design,
  should be recompiled using Dynamic C 10.72 in order to support
  the new, increased selection of small-block nand flash devices.

- The Rabbit Field Utility (GUI version at Utilities\RFU.exe,
  command line version at Utilities\clRFU.exe) now features
  program auto-start after successful loading of a binary image
  to a Rabbit target. For details, see the NEW FEATURES notes in
  ReleaseNotes_DynamicC.txt.

- The command line Rabbit Field Utility (Utilities\clRFU.exe)
  features new options for displaying information about the
  connected Rabbit target board and for specifying boot options
  when the Rabbit target board is already known. For details, see
  the NEW FEATURES notes in ReleaseNotes_DynamicC.txt.

- New XBee/ZigBee driver.

  A new XBee/ZigBee driver and standard sample programs are
  provided. See Samples\XBee\ReadMe.txt and the latest version of
  "Dynamic C - An Introduction To ZigBee" for more information.

  Important note: Dynamic C's original XBee support libraries
  (xbee_api.lib, etc.) and samples are obsolete and are removed
  from the Dynamic C 10.70 release. Users are encouraged to port
  legacy XBee applications to the new XBee/ZigBee driver, however,
  where porting is not practical the application should remain on
  Dynamic C 10.66 or prior version.

- MiniCore RCM6600W and RCM6650W Americas (FCC) and International
  versions support.

  The MiniCore RCM6600W family supports both WiFi (on-core) and
  Ethernet (off-core) network interfaces.

  The MiniCore RCM6600W has a 1 MB serial boot flash while the
  the RCM6650W has a 4MB serial boot flash.  Using the API
  from sflash.lib, on the RCM6650W up to 3MB is available for
  data storage.  See sample programs in Samples\RCM6600W.

  To provide improved application stability and lower EMI/RFI
  noise, the MiniCore RCM6600W family's recommended, default PLL
  output frequency is 325 MHz (162.5 MHz Rabbit clock).

- ANSI-C file scoping.

  Each C file in a project now has its own scope independent of
  the other C files in the project. #use still works but the
  interaction with the new file scoping is different. See the
  Dynamic C User's Manual for more information.

- ANSI-C block (nested) scoping.

  As per the ANSI/ISO specification, every curly-brace-delimited
  block now defines a new scope that is a child of the function
  or block in which the block occurs. Note that this also allows
  the declaration of variables at the beginning of any block.

- ANSI C89/ISO C90 compliant support for 'const'.

  Dynamic C now handles the 'const' modifier in a manner
  consistent with the ANSI/ISO specification, unlike previous
  versions that  limited the use of 'const' to initialized
  variables. See the Dynamic C User's Manual for more
  information.

- Signed character support.

  Added support for 'signed char' type and use of 'signed' to
  describe 'int' and 'long int' types.

- ANSI-C standard libraries.

  Updated libraries for compliance with ANSI C89/ISO C90 Standard.
  Define the macro __ANSI_STRICT__ to get ANSI behavior instead
  of legacy Dynamic C behavior.

  Notable additions and changes include:

  stdarg.h: Standard macros for accessing variable arguments.

  stdio.h: Includes full support of streams (FILE *) via standard
           functions like fopen(), fread(), fwrite(), etc.

           Includes scanf functions (fscanf, sscanf, vfscanf,
           etc.) with support for most of C99 format specifiers.

           Includes formatted print functions (printf, fprintf,
           snprintf, vfprintf, etc.) with support for most of
           C99 format specifiers.

           If __ANSI_PRINTF__ is defined, field widths in *printf
           functions grow to display full value (instead of
           displaying "***" to indicate overflow).

           If __ANSI_PUTS__ is defined, puts() appends a newline
           to the string sent to STDOUT (instead of just sending
           the string).

  stdlib.h: Adds atexit() to register functions that will auto-
            matically run when a program exits.

            Adds _n_bsearch() and _f_bsearch() for performing
            binary searches on sorted lists.

            Limited recursion in _n_qsort() and _f_qsort() to
            improve performance and eliminate possible stack
            overflow when sorting a large sorted list.

            Adds div() and ldiv() for getting quotient and
            remainder with a single division call.

  time.h: Includes strftime(), ctime() and asctime() for
          formatted time printing.

          If __ANSI_TIME__ is defined, uses ANSI-compliant
          'struct tm' structure.

- When using userblock API functions, you must now add
  the following LIB to the application program.

  #use idblock_api.lib

- When using sfbxxx serial flash API functions, you must now add
  the following LIB to the application program.

  #use bootdev_sf_api.lib

*****************************************************************

- Function pointer prototype checking

  Function pointers can now have prototypes.  For example, the following
  declares a function pointer foo that returns an int and takes an int, a
  long, and a float as parameters.

      int (*foo)(int, long, float);

  When assigning to a function pointer, the parameters of the right-hand
  side of the assignment are checked against the prototype of the function
  pointer.  No warnings are issued if either side has an empty () parameter
  list.

- Power-fail safe Remote Program Update

  RabbitCore modules that boot from serial flash can take advantage of a
  power-fail-safe version of the Remote Program Update library.  Define
  BU_ENABLE_SECONDARY in the project defines to enable the feature.  View
  Samples\RemoteProgramUpdate\download_firmware.c for an example of how it
  is used.

- TLS/SSL security on any TCP/IP socket

  TLS/SSL security is now available for use with any TCP/IP socket, as a
  general facility, via the new sock_secure() and sock_unsecure() API
  functions.  Previously, TLS was only available as an internal component
  when using the HTTP server library, or as part of Wi-Fi authentication.
  It is now available for general use, in client or server roles as
  applicable.  HTTP, SMTP and POP3 are all TLS-capable, with minor
  modifications to existing applications.

  Once a socket is secured, the same set of API functions that are used for
  ordinary TCP sockets may be used with the secure socket.  This makes it
  very easy to convert an existing TCP socket application to use SSL/TLS
  for enhanced security.

  In particular, TCP/IP client libraries HTTP_CLIENT.LIB, SMTP.LIB and
  POP3.LIB now support TLS encryption.  HTTP client supports the "https://"
  scheme.  SMTP allows use of the STARTTLS command (RFC 3207), and POP3
  allows use of POP3 tunneled via a TLS connection.  New sample programs
  demonstrate the TLS features of these libraries.

  The TLS/SSL API is compatible with products that have more than 128k of RAM
  and a Rabbit 4000 or newer CPU.

*****************************************************************

- #include support.

  In addition to Dynamic C's #use (which is designed to
  work with specially formatted source files), Dynamic C 10.60
  introduces the standard C #include, which follows the ANSI
  C 1989/ISO C 1990 specification.

- Multiple C file projects.

  The new Project Explorer Window allows
  for the creation of projects utilizing multiple C files, and
  several menu options have been added to allow compilation of
  the current project.

-- Variable Initializer Support

   Variables can now be initialized at declaration.  For example,

   void foo(int bar) {
      int i = 0, j = bar * 2;
      struct FOO {float pi, e, sqrt2;} fvals =
                        {3.14159, 2.71828, 1.41421};
      static far char carray[] = "String literal initializer";
       ...
   }

   Simple variables (integer and floating point types, pointers) can
   be initialized with any appropriate expression that evaluates to
   the correct type.

   Aggregate types (structs, arrays, unions) can only be initialized
   with constant values (i.e., anything that can be used as an
   initializer for a constant).

   Variable initialization follows the ANSI C 1989/ISO C 1990
   specification:
   -  Initializers for auto (non-static local) variables are
      evaluated at each function entry.
   -  Initializers for static and global variables are evaluated once
      at boot (much like Dynamic C's #GLOBAL_INIT statements).

-- Preprocessor Support for "defined" Keyword

   The "defined" keyword can be used in #if/#elif expressions to
   determine whether a macro is already defined.  If MACRO is
   defined, "defined MACRO" evaluates to 1, otherwise it evaluates
   to 0.  The macro can be optionally surrounded by parentheses for
   clarity:

   Example:

      #define FOO
      #if defined FOO && ! defined(BAR)
      ...
      #endif

-- ADDP

  Added library, sample and Windows utilities demonstrating ADDP
  (Advanced Device Discovery Protocol), a UDP multicast protocol
  used for discoverying and configuring devices on a LAN.  Look
  at Samples/tcpip/addp.c for sample Rabbit code using the ADDP
  server library, and the folder Utilities/ADDP for programs and
  a DLL for running an ADDP client under Windows.

-- Link-local

  Added support for automatic selection of a link-local IP
  address (169.254.1.0 to 169.254.254.255) as a fallback when
  DHCP is not available, or for zero-configuration static IP
  networks.  Inclue [#define USE_LINKLOCAL] in your program
  before [#use "dcrtcp.lib"], and then use any 169.254.x.x address
  for a static IP or DHCP fallback IP.  The Rabbit will
  automatically select an available, valid address as outlined in
  RFC 3927.

-- MiniCore RCM5650W Support

  The MiniCore RCM5650W uses a 4MB serial boot flash in place
  of the 1MB flash used on the RCM5600W.  Using the API from
  sflash.lib, 3MB are available for data storage.  See sample
  programs in Samples\RCM5600W.

-- Cloning is deprecated

   Cloning is not supported on the RCM5750, RCM5760, and RCM5650W,
   and is deprecated on all other Rabbit 4000 and Rabbit 5000 based
   products.  The Rabbit Field Utility (RFU) has been enhanced to
   handle loading firmware onto many devices at once.  Please use
   RFU.exe located in the utilities directory instead of cloning.

-- Blank Array Dimensions

   As per the C specification, array dimensions must be completely
   specified, with the exception that the first dimension can be
   left blank when an initializer is given to the array.  Prior to
   10.60, other dimensions in an initialized array could be left
   blank.  The size of an unspecified dimension was determined by
   the first instance of that dimension in the initializer, causing
   unexpected behavior in some cases.

-- MiniCore RCM5750/RCM5760 Support

   The MiniCore RCM5750 and RCM5760 add an additional 512k fast SRAM
   and a 2 MB serial flash to the RCM5700 and RCM5710 respectively.

-- Rabbit Embedded Security Pack

   The Rabbit Embedded Security Pack is now included with Dynamic C,
   adding enterprise level Wi-Fi authentication, SSL, and AES to
   the Dynamic C standard set of libraries.

-- Remote Program Update

   Remote Program Update enables update of boot firmware without
   having physical access to the hardware.  Supported on the
   RCM4200, RCM4300-series, RCM4400W-series, RCM5400W-series,
   RCM5600W, RCM5750/60, BL4S100-series, BL4S200 and BL5S220.
   Please read Application Note AN421 for details on the use
   of this new functionality.

-- Wi-Fi Authentication

   The Rabbit Embedded Security Pack now supports enterprise
   level authentication for Wi-Fi networks.  Authentication
   protocols supported include EAP-TLS and EAP-PEAP-MSCHAPv2.

-- MiniCore RCM5600W Support

   The MiniCore RCM5600W is a low-cost, 802.11b/g (Wi-Fi) core
   module with a small form factor.  It is pin compatible with
   the RCM5700 and uses the Rabbit 5000 microprocessor.

-- Run-time Pointer Checking Disabled

   Run-time pointer checking has been permanantly disabled in
   Dynamic C 10.50 and will likely be removed in future
   releases.

-- LZ Compression now supports I/O streams (including FAT access)

   The LZ Compression library now supports stdio streams for both input
   to and output from compression/decompression operations.  This stream
   support allows FAT files to be used for input and output as well.

-- ZB Firmware support for XBee

   The library xbee_api.lib now supports ZB firmware for the
   XBee.  ZB firmware is ZigBee Pro compliant and should be
   used for new development.  ZNet 2.5 firmware is still
   supported on the RCM4510W.  Please see the function help
   for changes to the API for the new firmware.


-- BL4S200, BL4S210, BL5S220, and BL4S230 Support

   The BL5S220 and BL4S200 are highly configurable single board
   computers based on the Rabbit 5000 and Rabbit 4000 processors.
   The BL4S200 has 10/100 Ethernet and SD card support.  The
   BL4S210 has 10Base-T Ethernet.  The BL5S220 has 802.11g Wi-Fi.
   The BL4S230 has a ZB XBee installed for Zigbee networking.

-- BL4S100, BL4S110, BL4S150, and BL4S160 Support

   The BL4S100, BL4S110, BL4S150 and BL4S160 are low-cost single
   board computers based on the Rabbit 4000 microprocessor.  All
   four versions have 10Base-T Ethernet. The BL4S150 and BL4S160
   have a larger memory configuration.  The BL4S100 and
   BL4S150 have a ZB XBee installed for Zigbee networking.

-- MiniCore RCM5700 Support

   The MiniCore RCM5700 is the first core module supporting both
   the Rabbit 5000 microprocessor and 10/100 Ethernet; it is a
   low-cost core module with a small form factor.

-- RCM5400W Support

   The RCM5400W and RCM5450W are the first Rabbit Core Modules
   (RCM) to support 802.11g Wi-Fi.

-- RCM43xx and RCM4050 Support

   The RCM4300 and the RCM4050 are the first Rabbit Core Modules
   (RCM) to support addressable memory beyond 1 megabyte
   utilizing the Rabbit 4000 microprocessor's extended memory
   capabilities.  The software to support the new larger memories
   is included in Dynamic C version 10.21 and later.

   The RCM4310, also supported with Dynamic C 10.21 and later,
   is a scaled-down version of the RCM4300 with a total
   of 1MB of RAM.

-- RCM4300 SD flash support

   Dynamic C now contains a driver for supporting SD flash.  This
   driver is also supported by the FAT File System.

-- RCM4510W Improvements

   The ZigBee support of the RCM4510W is improved so that
   incoming messages are queued in a buffer pool and removes some
   of the calling restrictions within cluster functions and the
   general message handler.  See Samples\ZigBee\README_ZIGBEE.TXT
   for more information on ZigBee.

   IMPORTANT: Dynamic C 10.21 and later support the updated
   FCC-approved XBee Series 2 radios.  If you have upgraded
   to Dynamic C 10.21 (or later) and have preview hardware, you
   must upgrade the firmware to version 1x21 or later for both
   the Digi XBee USB and the RCM4510W XBee radios.

   Dynamic C 10.40 ships with version 1x41 of the XBee ZNet
   firmware.  It is compatible with existing 1x21 devices, but
   we recommend that you update the XBee modules on all RCM4510W
   core modules and the Digi XBee USB to the 1x41 release.

   To upgrade firmware on the Digi XBee USB, use the X-CTU
   utility included in Utilities\X-CTU.  To upgrade the firmware
   on the RCM4510W, use Samples\ZigBee\ModemFWLoad.c. Please
   refer to Technical Note 256 for specific instructions.

-- Flash selection options

   The Dynamic C IDE now lets you specify an additional program
   flash type: 8-bit serial flash. By default the IDE is configured
   to automatically detect among 8-bit parallel, 16-bit parallel,
   and 8-bit serial program flash. This adds some delay to the
   compile/download/execute cycle, and therefore if you wish to
   circumvent detection you may specify the program flash type
   directly through the "Options->Project Options" menu item in
   the "Compiler" tab.

-- Software updates for the larger memories

   In order to support larger memories, some changes to the
   Dynamic C environment were necessary. Primarily, the changes
   are in two places:

   - The \LIB directory now contains 2 separate directories,
     Rabbit2000_3000 and Rabbit4000. Each directory contains
     the same structure that \LIB used to, but the libraries
     have been updated for the Rabbit 4000 processor in the
     \Lib\Rabbit4000 directory.  Note that Dynamic C 10.21
     and later only support Rabbit 4000 based products.

   - Memory mapping. Memory is mapped in Dynamic C using the
     "origin" mechanism. The mechanism has been updated to be
     easier to use and is now more effective at identifying
     collisions and producing useful error messages.

     All memory mapping directives are now located in a file
     called "memory_layout.lib" instead of the BIOS.

     Documentation for the new mechanism is forthcoming in the
     Rabbit 4000 Microprocessor Designer's manual.  Please
     check the Rabbit Semiconductor website for updates.

-- Far data and the Xmem API

   Products based on the Rabbit 2000 and 3000 processors utilized
   xmem through the xalloc function. Access to xmem was provided
   through a user API (xmem2root, root2xmem, xgetint, etc...).

   The xmem API is still needed for the Rabbit 2000 and 3000
   processors, but the Rabbit 4000 (with DC 10.xx) now supports
   the "far" qualifier, which is easier to use and more
   efficient.

   Far data is backward-compatible with the xmem API (with casts
   of longs to far pointers), but it is recommended that far data
   be used for any new development, and any xmem functions should
   be updated to use the far variants ("_f_" prefix) instead.
   Examples:

   - _f_memset instead of xmemset
   - _f_memcpy instead of xmem2xmem
   - For simple variables, direct assignment from far data to
     root data is possible, as is assignment from root to far
   - For root2xmem and xmem2root, use _f_memcpy and call PADDR
     on the root address, as in:
      _f_memcpy(PADDR(root_var), far_var, len)

-- Character assembly during break

   The RS232_NOCHARASSYINBRK macro now works for STDIO_DEBUG_SERIAL mode
   (part of the STDIO.LIB library).  This suppresses the generation of
   multiple null characters when no cable is attached and the RX pin is
   not pulled up.  It is now the default when STDIO_DEBUG_SERIAL is
   defined to SADR, but must still be explicitly defined for other ports,
   or when using RS232.LIB.  (Pin PC7/RXA is pulled up on Rabbit 2000 and
   3000 core modules, suppressing null character generation. Rabbit 4000
   core modules do not pull up PC7/RXA.  RS232_NOCHARASSYINBRK allows one
   null character to be generated.)

-- Wi-Fi fixes and enhancements
   The wifi_ioctl() API function has slightly different semantics for some
   commands.  In particular, many of the commands require that the Wi-Fi
   network interface is brought down before the command is accepted.  See the
   function help (Ctrl-H) for wifi_ioctl() for details.  Application code
   should ensure that the return code from wifi_ioctl() is checked after
   each call.

   In particular, note that you must bring the interface down using the
   ifdown() function before beginning a Wi-Fi scan.  The interface should be
   kept down while the scan is being performed.  Once the scan has been
   completed, then the interface can be brought back up with ifup().  See
   the samples wifiscan.c and wifiscanassociate.c in Samples\WiFi\ for
   demonstrations of how a scan is performed.

   Wi-Fi Protected Access (WPA), with Pre-Shared Key (PSK), is now available
   for applications which require robust privacy and integrity.  In order to
   use WPA/PSK, simply define WIFI_USE_WPA plus a suitable ascii passphrase
   or hexadecimal key, and define _WIFI_WEP_FLAG to WIFICONF_WEP_TKIP.
   See samples\rcm4400w\tcpip\pingled_wpa_psk.c for more details.  More
   documentation on the relevant macros may be found through typing
   Ctrl-H on the macro TCPCONFIG in a Dynamic C editor window.

   In order to be acceptable to various countries' regulatory
   authorities, there is an improved API for customizing the Wi-Fi library to
   conform to the radio regulations.  The wifi_ioctl() API function has new
   commands to query and set the channel list and transmit power.  The
   commands are WIFI_MULTI_DOMAIN which allows automatic configuration via
   802.11d capable access points; WIFI_COUNTRY_SET which allows one of
   several known regulatory domains to be configured; WIFI_COUNTRY_GET
   which queries the current regulatory domain information.

-- Wi-Fi Roaming
   The Wi-Fi libraries now support roaming.  The Wi-Fi core modules
   can now follow an access point when it switches to a new channel,
   and can switch to a different access point if the current access
   point becomes unavailable.

   View the ifconfig() function help for details on the new
   IFS_WIFI_ROAM_* and IFG_WIFI_ROAM_* commands.  View the function
   help for TCPCONFIG for information on the new IFC_WIFI_ROAM_*
   compile-time macros.

-- Known Issues.

1. Debugger issues

-  When debugging a program using either F7 or F8 into or over a
   call to kbhit() will cause the debug cursor to disappear. Pressing
   F7 or F8 two more times will make the debug cursor reappear
   following the kbhit() call.

-  The breakpoint highlight for the closing brace of a function
   does not display correctly if the brace is not in the first column
   of the editor window.

-  The debugger currently only supports 128 breakpoints.  Setting more than
   128 breakpoints will result in the loss of target communications.

-  The debugger does not support assembly language single stepping
   over function calls that do not return to the instruction following
   the call such as "_switch". Any attempt to do so will result in
   loss of target communication. Stepping _into_ the function in
   assembly is supported, as is stepping over such functions in C.

-  If an ipset or ipres instruction is followed immediately by an rst 0x28,
   the debug kernel will not step over the rst 0x28 correctly, but will end up
   in an unexpected location.  When either of these instructions are followed
   by instructions other than rst 0x28, the debug kernel behaves correctly.

2. Software modules.

   RabbitWeb gives a cryptic error message for buffer overflow when
   using the print_select() statement.  The workaround is to make
   HTTP_MAXBUFFER large enough to hold all of the OPTION statements
   generated by the print_select() statement.

3. Complex casts

   The compiler will not accept multidimensional array types or
   derivatives thereof as cast expressions, even if the cast expression
   is a typedef name. Use a union to work around this problem.

4. Advanced 16-bit memory mode CPU bug

   The Rabbit 4000 CPU's advanced 16-bit memory mode has a defect
   which affects ioe instructions (auxiliary I/O, external I/O)
   and self-timed chip select.  The Dynamic C BIOS has been
   updated to work around this ioe defect on affected boards, all
   of the RCM40xx family.  If absolute top performance is required
   and the User is certain their application is unaffected by the
   ioe bug, the work around can be disabled by adding the
   __ALLOW_16BIT_AUXIO_DEFECT macro into Dynamic C's Project
   Options Defines Box.  See the Rabbit 4000 Users Manual Appendix
   B (errata section) or TN255 for complete details.

5. In separate I/D mode, an array declared as

      far char bar[] = {0,1,2,3,...};

   will not compile correctly when the size of the array exceeds the
   size of the root constant space and will give an error that the
   '}' character is missing. The work-around is to specify the size
   of the array as follows:

      far char bar[32000] = {0,1,2,3,...};

6. Highlighting a block of text while in Stop mode

   In some cases, depending on the specific text, highlighting a block of text
   can result in a flyover watch which returns a copy of that text in the hint
   window.

7. Using #pragma SIZEOF32 can result in a sizeof problem for a large struct

   Using #pragma SIZEOF32 allows the argument of sizeof to be up to 4GB but
   will return an incorrect value without warning or error if the argument
   involves a struct of size 32K or larger. When using #pragma SIZEOF32 always
   restore to the default with #pragma SIZEOF16 following the use of sizeof.

8. Initialized global and static variables and #GLOBAL_INIT expressions run
   with a limited stack (256 bytes).  Deeply nested functions (including
   printf()) will cause a loss of target communications.

9. Cast ternary operator(s) if both values do not have the same type and type
   qualifiers - example:    condition ? &aFarChar : (far char *)NULL.
   In the example, if condition is true and NULL is not cast, a near address
   is erroneously returned.

10. As of 10.66, symbols are distinguished using the first 63 characters of the
    name.  Older versions used 31 characters, the minimum per the ANSI
    specification.  If a long symbol name is used as a library module label
    (i.e., in a BeginHeader statement), the library's .MD1 file will contain
    the short version of the name, and the module will not be found.  For
    example:

      /*** BeginHeader A12345678911234567892123456789312 ***/
      extern int A12345678911234567892123456789312;
      /*** EndHeader ***/
      int A12345678911234567892123456789312;

    will generate the error "line  XXX : ERROR MY_LIB.LIB : Undefined
    (but used) global label A12345678911234567892123456789312." The
    workaround is to change the timestamp on the library file (e.g.,
    modify it and save it) and recompile.  The .MD1 file will be
    regenerated with the correct (longer) module label.

11. More than one return statement in a cofunction will result in unpredictable
    behavior.

12. For auto int i, the expression (far*)&i should be (far int*)&i, but the
    first form will produce confusing error messages:
        Invalid expression.
        Missing character ';'.
        Missing character ')'.
        Not a pointer, cannot dereference.

13. A "c return" statement in an assembly block can produce an error like
    "Undefined (but used) global label .DCLAB__ZW00000190" in large programs.
    The assembly block can be ended followed by the return statement in C
    followed by the remaining assembly in a new assembly block.

14. The active status of hardware breakpoints is not retained when a program
    restarts. Each breakpoint must be modified in some way such as toggling a
    checkbox so that the “Update” button becomes active.

15. Setting a hardware breakpoint on some internal I/O addresses can lead to
    a target communication error. Since setting a breakpoint mask to 0xffffff
    will include all internal I/O address, the address and mask should be set
    to include only the intended range of addresses.

16. Programs larger than 512K will not compile to RAM successfully on an
    RCM5450W core module. Compiling such programs to flash works fine.

17. Number of dimensions in array initializer are not checked against the
    array declaration.

    Dynamic C will compile an initializer with too few dimensions without
    complaining:

        // two few dimensions in initializer
        const static int a[][3][2] = {{11, 12}, {21, 22}};

    The results are undefined.  If there are too many dimensions in the
    initializer, Dynamic C will indicate an error "} is missing/expected."

18. For functions with both a near and far syntax, function help is available
    only using the unadorned name.  For instance, placing the cursor over
    _f_memset will not find the function help for memset.  In order to find
    function help for a function prepended with either _n_ or _f_, such as
    _f_memset, separate the main part of the function name before pressing
    Ctrl+h.

19. A function that returns a struct by value does not work correctly within a
    cofunction. A work around is to have the function return a static struct
    by reference and then dereference the function result on assignment in the
    cofunction, e.g.:

       struct point { int x; int y; };

       struct point *foop(void)
       {
          static struct point tmp;

          tmp.x = 3;
          tmp.y = 4;
          return &tmp;
       }

       cofunc void co_func(void)
       {
          struct point tmp;

          tmp = *foop();   // dereference the result at time of assignment
          printf("cofunction by reference, x=%d, y=%d\n", tmp.x, tmp.y);
       }

       void main(void)
       {
          struct point tmp;

          costate {
             wfd co_func();
          }
       }

20. An index variable used in calling an indexed cofunction cannot be auto. The
    index variable must have static (local) or global storage, e.g.:

       #define INDICES 5

       cofunc void my_icofunc[INDICES](void)
       {
          ;
       }

       void main(void)
       {
          static int i;

          while (1) {
             for (i = 0; i < INDICES; ++i) {
                costate {
                   wfd my_icofunc[i]();
                }
             }
          }
       }

21. Auto pointers to CoData structures cannot be used in named costatements. The
    CoData structure pointer must have static (local) or global storage, e.g.:

       #define NUMTASKS 5

       void main(void)
       {
          auto int i;
          static CoData task[NUMTASKS];
          static CoData *ptr2codata;

          for (i = 0; i < NUMTASKS; ++i)
             CoBegin(&task[i]);

          while (1) {
             for (i = 0; i < NUMTASKS; ++i) {
                ptr2codata = &task[i];
                costate ptr2codata always_on {
                   ;
                }
             }
          }
       }

22. Dynamic C currently does not allow function pointers to return structures
    by value. The following code snippet will generate an error:

      typedef struct { int a; } my_struct_t;
      my_struct_t (*func_ptr)();
      main() {
         my_struct_t s;
         s = func_ptr();
      }

23. Assembly labels within #asm blocks inside C functions are treated as if
    they are local C-labels, rather than existing within their own scopes.
    In the following code, the expected behavior would be an "unknown label"
    error on the jp to foo since there is no global label foo defined. However,
    the linker associates the local label with the jp instruction and the
    code compiles.

    Note that this behavior only occurs between #asm blocks within C functions.
    It does not exist between stand-alone #asm blocks or between blocks in
    separate C functions.

    void func(void) {
       #asm
          jp foo  ; This jumps to the label in the next block
       #endasm
       // Some C code
       #asm
          foo:    ; The jp above lands here
       #endasm
   }

24. The strtod function's underflow detection fails when converting strings of
    the following forms and lengths:
      ".00000000000000000000000000000000000001"
      ".000000000000000000000000000000000000001"
      . . .
      ".00000000000000000000000000000000000000000001"
      ".1e-38
      ".01e-38"
      . . .
      ".00000000000000000000000000000000000001e-38"

25. Dynamic C's assembler does not support embedded C return statements (e.g.
    "c return;" or "c return X;") within an asm block that is within a C
    function. All such occurrences have been removed from standard libraries
    (i.e. DMA.LIB, FFT.LIB) and no occurrences exist in standard samples code.
    Custom asm code with embedded C return statements as described above
    should be rewritten such that any explicit or implicit return from the C
    function is done in standard C code.

26. Complex macros inside a one-line for-loop may fail to compile, with an
    internal error ("Internal error: input stream non-existant. Compiler
    confused. Look for previous syntax error.") that requires Dynamic C to be
    restarted.

    Work-around: replace

       for(i=0;i<max;i++){complex_macro;}

    with

       for(i=0;i<max;i++){
          complex_macro;
       }

27. When compiling an initialized array of unspecified dimension(s) in a nested
    scope, Dynamic C does not generate a jump over the initializer data, which
    results in errant execution of the initializer data. One work around is to
    avoid declaration of such arrays in nested scope. Another work around is to
    ensure all initialized arrays in nested scope have fully specified
    dimensions.

    Work-around example: replace

       while (1)
       {
          char *foo[] = { "123", "abc" };
          printf("%s   %s\n", foo[0], foo[1]);
       }

    with

       while (1)
       {
          char *foo[2] = { "123", "abc" };
          printf("%s   %s\n", foo[0], foo[1]);
       }

28. The compiler incorrectly throws an error ("Duplicate 'const' keywords not
    allowed in this context.") if a function contains multiple, non-pointer
    `const` parameters.

    Work-around example: replace

        void foo(const int bar, const int baz) {}

    with

        typedef const int const_int;
        void foo(const_int bar, const_int baz) {}

