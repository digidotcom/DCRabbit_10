/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/***********************************************************************
	FAT_SHELL.C

   Requires that you run this on a board with a compatible
   storage medium (serial flash, NAND flash or SD card).

   Gives a UNIX like shell for the FAT file system.  This sample can
   format an empty device, or unformatted FAT partitions.  The shell
   has the ability to re-format existing partitions once it is running
   to give a fast way to delete all files during experimentation.  If
   formatting an empty device, it will create one large FAT partition
   and format that partition.  Before formatting of an empty device,
   a prompt will be presented asking permission to proceed.  Formatting
   of partitions (formatted or unformatted) is done through the shell's
   format command.
   
   Note that default settings in Lib/Rabbit4000/BIOSLIB/fatconfig.lib
   impose a limit of 2 partitions mounted at any one time.  If you want
   to mount all 4 primary partitions on a device, you need to add
   "FAT_MAXPARTITIONS=4" to your Project Options.

************************************************************************/

#define MAX_DEVICES	2	// must be >= actual number of configured FAT devices
#define MAX_FILES		20

// Uncomment following macro definition(s) to turn on Debug options
//#define FAT_DEBUG
//#define FAT_VERBOSE
//#define FATFTC_DEBUG
//#define FATFTC_DEBUG_INIT
//#define FATFTC_VERBOSE
//#define FTL_DEBUG
//#define NFLASH_DEBUG
//#define NFLASH_VERBOSE
//#define NFLASH_FAT_DEBUG
//#define NFLASH_FAT_VERBOSE
//#define SFLASH_DEBUG
//#define SDFLASH_DEBUG
//#define SDFAT_DEBUG
//#define SDFLASH_VERBOSE
//#define PART_DEBUG

// Map program to xmem if not compiling to separate I&D space or debugging.
#if __SEPARATE_INST_DATA__
	#ifdef FAT_DEBUG
		#memmap xmem
	#endif
#else
	#memmap xmem
#endif

// Always use near strings, even if FAR strings enabled
#ifdef USE_FAR_STRING_LIB
  #define NS_strcat(A,B)    _n_strcat(A,B)
  #define NS_strchr(A,B)    _n_strchr(A,B)
  #define NS_strcpy(A,B)    _n_strcpy(A,B)
  #define NS_strrchr(A,B)   _n_strrchr(A,B)
  #define NS_strtol(A,B,C)  _n_strtol(A,B,C)
#else
  #define NS_strcat(A,B)    strcat(A,B)
  #define NS_strchr(A,B)    strchr(A,B)
  #define NS_strcpy(A,B)    strcpy(A,B)
  #define NS_strrchr(A,B)   strrchr(A,B)
  #define NS_strtol(A,B,C)  strtol(A,B,C)
#endif


// Set FAT library to blocking mode
#define FAT_BLOCK

// Set file system to use forward slash as directory separator
#define FAT_USE_FORWARDSLASH

// Load the FAT filesystem library
#use "fat16.lib"

// Required control structures to operate the FAT filesystem
FATfile  file;
FATfile  files[MAX_FILES];

char buf[128];
char path[300];
char mpath[300];
char pwd[MAX_DEVICES * FAT_MAX_PARTITIONS][257];
int active_part;

// Prints a help listing of valid shell commands
void help()
{
	printf("FAT_Shell commands:\n");
   printf("p:                      Set partition where p is partition id\n");
   printf("ls                      List current directory (alias=dir)\n");
   printf("cd [dirname]            Change directory [root]\n");
   printf("pwd                     Print current directory\n");
   printf("touch filename [alc]    Create file [1 cluster alloc]\n");
	printf("mtouch n filename [alc] Create n files [1 cluster each]\n");
   printf("wr filename [bytes]     Write to file [1k]\n");
	printf("mwr n filename [bytes]  Write to n files [1k each]\n");
   printf("ap filename [bytes]     Append to file [1k]\n");
	printf("map n filename [bytes]  Append to n files [1k each]\n");
   printf("mkdir dirname           Create directory\n");
   printf("mmkdir n dirname        Create n directories\n");
   printf("rd filename [bytes]     Read from file [first 1k max]\n");
   printf("split filename newfile  Split excess allocation to newfile\n");
   printf("trunc filename [bytes]  Truncate file [length] (Free Prealloc.)\n");
   printf("del filename            Delete the file (alias=rm)\n");
   printf("rmdir dirname           Remove the directory (must be empty)\n");
   printf("tail filename [bytes]   Read last bytes from file [last 1k max]\n");
   printf("pdump                   Print partition info\n");
   printf("fat [startx [endx]]     Print FAT table [2 [64]]\n");
   printf("stat filename           Print file/directory info\n");
   printf("format [p]              Erase partition or device a,b,...,0,...\n");
   printf("h[elp]                  Print this help message\n");
   printf("exit                    Exit this program\n\n");
}

// Dumps partition and device information to the Stdio window
void pdump()
{
	int i;
   FATfile * f;

	printf("partition number %d, type %d\n",
		fat_part_mounted[active_part]->pnum,
		fat_part_mounted[active_part]->type);
	printf("serialnumber = %lu (0x%08lx)\n",
		fat_part_mounted[active_part]->serialnumber,
		fat_part_mounted[active_part]->serialnumber);
	printf("journal number (ftc_prt) = %d\n",
		fat_part_mounted[active_part]->ftc_prt);

	printf("\n");

	printf("%10u FATs (fat_cnt)\n",
		fat_part_mounted[active_part]->fat_cnt);
	printf("%10lu bytes in a FAT (fat_len * 2)\n",
		2L * fat_part_mounted[active_part]->fat_len);
	printf("%10lu sectors per FAT (sec_fat)\n",
		fat_part_mounted[active_part]->sec_fat);
	printf("%10u entries in root directory (root_cnt)\n",
		fat_part_mounted[active_part]->root_cnt);
	printf("%10u reserved sectors (res_sect)\n",
		fat_part_mounted[active_part]->res_sec);

	printf("\n");

	printf("%10u sectors per cluster (sec_clust)\n",
		fat_part_mounted[active_part]->sec_clust);
	printf("%10u bytes per sector (byte_sec)\n",
		fat_part_mounted[active_part]->byte_sec);
	printf("%10lu bytes per cluster (clustlen)\n",
		fat_part_mounted[active_part]->clustlen);

	printf("%10lu total clusters (totcluster) (%.3fMB)\n",
		fat_part_mounted[active_part]->totcluster,
		(float) fat_part_mounted[active_part]->totcluster
		* fat_part_mounted[active_part]->clustlen / (1024L * 1024));

	printf("%10lu bad clusters (badcluster) (%.3fKB)\n",
		fat_part_mounted[active_part]->badcluster,
		(float) fat_part_mounted[active_part]->badcluster
		* fat_part_mounted[active_part]->clustlen / 1024);

	printf("%10lu free clusters (freecluster) (%.3fMB)\n",
		fat_part_mounted[active_part]->freecluster,
		(float) fat_part_mounted[active_part]->freecluster
		* fat_part_mounted[active_part]->clustlen / (1024L * 1024));

	printf("%10lu is next cluster (nextcluster)\n\n",
		fat_part_mounted[active_part]->nextcluster);

	printf("%10lu is first sector of first FAT (fatstart)\n",
		fat_part_mounted[active_part]->fatstart);
	printf("%10lu is first sector of root dir (rootstart)\n",
		fat_part_mounted[active_part]->rootstart);
	printf("%10lu is first sector of data area (datastart)\n",
		fat_part_mounted[active_part]->datastart);

   for (f = fat_part_mounted[active_part]->first, i = 0; f; f = f->next, ++i);
   printf("%10d open files\n", i);

   printf("\nmpart.bootflag = 0x%02X\n",
          fat_part_mounted[active_part]->mpart->bootflag);
   printf("mpart.starthead = %u\n",
          fat_part_mounted[active_part]->mpart->starthead);
   printf("mpart.startseccyl = %u\n",
          fat_part_mounted[active_part]->mpart->startseccyl);
   printf("mpart.parttype = %u\n",
          fat_part_mounted[active_part]->mpart->parttype);
   printf("mpart.endhead = %u\n",
          fat_part_mounted[active_part]->mpart->endhead);
   printf("mpart.endseccyl = %u\n",
          fat_part_mounted[active_part]->mpart->endseccyl);
   printf("mpart.startsector = %lu\n",
          fat_part_mounted[active_part]->mpart->startsector);
   printf("mpart.partsecsize = %lu\n",
          fat_part_mounted[active_part]->mpart->partsecsize);
   printf("mpart.status = %04X\n\n",
          fat_part_mounted[active_part]->mpart->status);

   printf("dev.seccount = %lu\n",fat_part_mounted[active_part]->dev->seccount);
   printf("dev.heads = %u\n", fat_part_mounted[active_part]->dev->heads);
   printf("dev.cylinder = %u\n", fat_part_mounted[active_part]->dev->cylinder);
   printf("dev.sec_track = %u\n",
                              fat_part_mounted[active_part]->dev->sec_track);
   printf("dev.byte_sec = %d\n", fat_part_mounted[active_part]->dev->byte_sec);
   printf("dev.byte_page = %d\n",
                              fat_part_mounted[active_part]->dev->byte_page);
   printf("dev.drv_num = %d\n", fat_part_mounted[active_part]->dev->dev_num);
   printf("dev.ftc_dev = %d\n", fat_part_mounted[active_part]->dev->ftc_dev);

   printf("\n");
}

// Prints out actual FAT table cluster entries in the range specified.
// Leave in for Beta, should be removed for release
void fat_print(long startx, long endx)
{
	long cluster;
   unsigned long nclust;
   int rc;

   for (cluster = startx; cluster <= endx; ++cluster) {
   	printf("%u (0x%04X)\t", (word)cluster, (word)cluster);
   	nclust = cluster;
		rc = _fat_next_clust(fat_part_mounted[active_part], &nclust, FTC_WAIT);
      if (rc == -EEOF)
      	printf("<EOC>\n");
      else if (rc == -ENODATA)
      	printf("<free>\n");
      else if (rc < 0)
      	printf("<BAD: %d>\n", rc);
      else
      	printf("%u (0x%04X)\n", (word)nclust, (word)nclust);
   }

}

// Prints out attributes of a file or directory.
int stat(char * filename)
{
   fat_dirent dent;
   fat_location loc;
	int rc;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   // Call fat_Status to fill in directory entry structure
   rc = fat_Status( fat_part_mounted[active_part], path, &dent );
   if (rc) {
   	if (rc == -ENOENT) {
      	printf("'%s' does not exist\n", path);
         return 0;
      }
   	printf("Status '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   printf("%s '%s' attributes are %c%c%c%c%c%c \n",
         	dent.attr & 0x10 ? "Directory" : "File",
            path,
            dent.attr & FATATTR_READ_ONLY ? 'R' : 'r',
            dent.attr & FATATTR_HIDDEN ? 'H' : 'h',
            dent.attr & FATATTR_SYSTEM ? 'S' : 's',
            dent.attr & FATATTR_VOLUME_ID ? 'V' : 'v',
            dent.attr & FATATTR_DIRECTORY ? 'D' : 'd',
            dent.attr & FATATTR_ARCHIVE ? 'A' : 'a');
   return 0;
}

// Touch create an empty but pre-allocated file of specified size.
int touch(char * filename, long alloc, int type)
{
	int rc;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   // Call fat_Open with MUST_CREATE mode and allocation size
   rc = fat_Open(fat_part_mounted[active_part], path, type, FAT_MUST_CREATE,
                 &file, &alloc);
   if (rc) {
   	if (rc == -EEXIST) {
      	printf("'%s' already exists\n", path);
         return 0;
      }
   	printf("Open '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   printf("%s '%s' created with %ld bytes\n",
		   				type == FAT_FILE ? "File" : "Directory", path, alloc);
	return fat_Close(&file);
}

// Deletes given file or directory
int del(char * filename, int type)
{
	int rc;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   rc = fat_Delete(fat_part_mounted[active_part], type, path);
   if (rc) {
   	if (rc == -EPERM) {
      	printf("'%s' is read-only, hidden etc. - NOT deleted\n", path);
         return 0;
      }
   	printf("Delete '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   printf("%s deleted.\n", path);
}

// Splits file 'filename' and re-assigns any unused pre-allocated clusters
// to newly created 'newfile'
int split(char * filename, char * newfile)
{
	int rc;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   // Open file to get a file handle
   rc = fat_Open(fat_part_mounted[active_part], path, FAT_FILE,0, &file, NULL);
   if (rc) {
   	if (rc == -ENOENT) {
      	printf("'%s' does not exist\n", path);
         return 0;
      }
   	printf("Open '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   // Put together working directory and newfile
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, newfile);
   // Call fat_Split with file handle, split position and new file's path/name
   rc = fat_Split(&file, file.de.fileSize, path);
   if (rc) {
   	fat_Close(&file);
   	if (rc == -EPERM) {
      	printf("'%s' is read-only, hidden etc. - NOT Split\n", filename);
         return 0;
      }
   	if (rc == -EEOF) {
      	printf("'%s' has no extra clusters - NOT Split\n", filename);
         return 0;
      }
   	printf("Split '%s' Error: %ls\n", filename, error_message(rc));
      return rc;
   }
   printf("%s split.\n", filename);
	return fat_Close(&file);
}

// Truncates the given file at position 'bytes' and frees any unused clusters.
int trunc(char * filename, long bytes)
{
	int rc;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   // Open file to get a file handle
   rc = fat_Open(fat_part_mounted[active_part], path, FAT_FILE,0, &file, NULL);
   if (rc) {
   	if (rc == -ENOENT) {
      	printf("'%s' does not exist\n", path);
         return 0;
      }
   	printf("Open '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   // Call fat_Truncate with file handle and desired size
   rc = fat_Truncate(&file, bytes);
   if (rc) {
      fat_Close(&file);
   	if (rc == -EPERM) {
      	printf("'%s' is read-only, hidden etc. - NOT Truncated\n", path);
         return 0;
      }
   	printf("Truncate '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   printf("%s truncated to %ld bytes.\n", path, file.de.fileSize);
	return fat_Close(&file);
}

// Writes or appends a file with 'bytes' characters from a repetetive fill
// string.
int wr(char * filename, long bytes, int append)
{
	auto char fox[128];
	int rc;
   long writ;
   long len;
   int ltw;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   len = 0;
   // Open file to get a file handle
   rc = fat_Open(fat_part_mounted[active_part], path, FAT_FILE, FAT_CREATE,
                 &file, &len);
   if (rc) {
   	printf("Open '%s' Error: %ls\n", path, error_message(rc));
      return rc;
   }
   // If appending, seek end of file before writing.
   if (append) {
   	rc = fat_Seek(&file, 0, SEEK_END);
      if (rc) {
	      printf("Seek '%s' Error: %ls\n", path, error_message(rc));
	      fat_Close(&file);
	      return rc;
      }
   }
   writ = 0;
   // Create the fill string by inserting filename and bytes
   sprintf(fox, "%s %ld The quick brown fox jumps over the lazy dog\n",
   					filename, bytes);
   len = strlen(fox);
   rc = 0;
   // Keep writing string to file until 'bytes' characters have been written
   while (writ < bytes) {
   	if (len < (bytes-writ))
      	ltw = (int)len;
      else {
			ltw = (int)(bytes-writ);
         fox[ltw-1] = '\n';
      }
      rc = fat_Write(&file, fox, ltw);
      if (rc > 0)
      	writ += rc;
      if (rc < ltw)
      	break;
   }
   if (rc < 0)
   	printf("Write Error: %ls\n", error_message(rc));
   printf("File '%s' %s with %ld bytes out of %ld\n", path,
			   						append ? "appended" : "written", writ, bytes);
	return fat_Close(&file);
}

// Read 'bytes' characters from either the beginning or end of file 'filename'
int rd(char * filename, long bytes, int tail)
{
	auto char b[81];
   char *ptr;
	int rc;
   int len;
   long red;
   int ltr;

   // Put together working directory and filename
   NS_strcpy(path, pwd[active_part]);
   NS_strcat(path, FAT_SLASH_STR);
   NS_strcat(path, filename);
   // Open file to get a file handle
   rc = fat_Open(fat_part_mounted[active_part], path, FAT_FILE,0, &file, NULL);
   if (rc) {
   	printf("Open '%s' failed rc=%d\n", path, rc);
      return rc;
   }
   // If reading from end, seek (end - bytes) before reading.
   if (tail) {
   	rc = fat_Seek(&file, -bytes, SEEK_END);
      if (rc) {
	      printf("Seek '%s' Error: %ls\n", path, error_message(rc));
	      fat_Close(&file);
	      return rc;
      }
   }
   red = 0;
   rc = 0;
   b[80] = 0;
   // Read in 80 characters at a time until 'bytes' characters read.
   while (red < bytes) {
   	if ((sizeof(b) - 1) < (bytes-red))
      	ltr = sizeof(b) - 1;
      else
			ltr = (int)(bytes-red);
      rc = fat_Read(&file, b, ltr);
      if (rc < 0)
      	break;
      b[rc] = 0;
  	   for (ptr = b; ptr < (b + rc); ptr += strlen(ptr) + 1)
     	   printf("%s", ptr);
     	red += rc;
   }
  	if (rc == -EEOF) {
      if (!red)
	     	printf("'%s' has no data.\n", filename);
   }
   else
	   if (rc < 0)
   		printf("Read Error: %ls\n", error_message(rc));
   printf("\nRead %ld bytes out of %ld\n", red, bytes);
	return fat_Close(&file);
}

// Print out directory listing of current working directory
int ls()
{
	int rc;
   fat_dirent dent;
   char fname[13];
   int del, lent;
   unsigned long clust;
   struct tm t;
   word files, dirs;
   unsigned long bytes;

   // Open directory to get a file handle
	rc = fat_Open(fat_part_mounted[active_part], pwd[active_part], FAT_DIR, 0,
	              &file, NULL);
   if (rc) {
   	printf("Open Directory '%s' Error: %ls\n",
                           pwd[active_part], error_message(rc));
      return rc;
   }
   printf("\n\tDirectory of %c:%s (dir length %lu)\n\n",
   	active_part + 'A', pwd[active_part], file.de.fileSize);

   del = lent = 0;
   bytes = files = dirs = 0;
	for (;;) {
   	// Use fat_ReadDir to read directory entries into structure 'dent'
   	rc = fat_ReadDir(&file, &dent, FAT_INC_DEF + FAT_INC_DEL );
      if (rc)
      	break;
		if (!dent.name[0])
      	break;
      else if (dent.name[0] == 0xE5)
      	++del;
      else if ((dent.attr & FATATTR_LONG_NAME) == FATATTR_LONG_NAME)
      	++lent;
      else {
      	if (del) {
         	printf("<%d deleted entries>\n", del);
            del = 0;
         }
      	if (lent) {
         	printf("<%d long-name entries>\n", lent);
            lent = 0;
         }
			// Looks OK
			bytes += dent.fileSize;

         _fat_Dir2Clust((char *)&dent, &clust);
         fat_GetName(&dent, fname, FAT_LOWERCASE);

         fat_CreateTime(&dent, &t);
         // new-style printout with date/time
         printf( "%04u-%02u-%02u %02u:%02u:%02u ",
            t.tm_year + 1900, t.tm_mon, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec);
         if (dent.attr & FATATTR_DIRECTORY)
         {
            printf( "%-10s", "<DIR>");
            dirs++;
         }
         else
         {
            printf( "%10lu", dent.fileSize);
            files++;
         }
         printf( " %12s (%c%c%c%c%c) clust=%lu\n", fname,
            dent.attr & FATATTR_READ_ONLY ? 'R' : 'r',
            dent.attr & FATATTR_HIDDEN ? 'H' : 'h',
            dent.attr & FATATTR_SYSTEM ? 'S' : 's',
            dent.attr & FATATTR_VOLUME_ID ? 'V' : 'v',
            dent.attr & FATATTR_ARCHIVE ? 'A' : 'a',
            clust);
      }
   }
   if (del)
      printf("<%d deleted entries>\n", del);
   if (lent)
      printf("<%d long-name entries>\n", lent);
   fat_Close(&file);

   printf ("\n\t%u file(s), %u dir(s), %lu bytes\n", files, dirs, bytes);
   return rc;
}

// format mounted partition or device w/ one partition
int format(char option, word base_flags)
{
	int i, rc, selected_dev, selected_part;
	word flags_to_use;

	// initialize impossible device and partition numbers
	selected_dev = selected_part = -1;

   // check for option, as partition letter or device number
	if (!option) {
		selected_part = active_part;
	}
	else if ((selected_part = tolower(option) - 'a', 0 <= selected_part) &&
	         selected_part < num_fat_devices * FAT_MAX_PARTITIONS)
	{
		if (fat_part_mounted[selected_part]) {
			// set up format of a previously mounted partition
			option = 0;
		} else {
			// set up unconditional format of unmounted partition
			flags_to_use = FDDF_UNCOND_PART_FORMAT | FDDF_COND_PART_FORMAT;
			if (0 <= selected_part && FAT_MAX_PARTITIONS > selected_part) {
				flags_to_use |= FDDF_MOUNT_DEV_0;
				flags_to_use |= FDDF_MOUNT_PART_0 << selected_part;
			}
			else if (FAT_MAX_PARTITIONS <= selected_part &&
			         FAT_MAX_PARTITIONS * 2 > selected_part)
			{
				flags_to_use |= FDDF_MOUNT_DEV_1;
				flags_to_use |= FDDF_MOUNT_PART_0 <<
				                (selected_part - FAT_MAX_PARTITIONS);
			}
		}
		// Reset the working directory to root
	   NS_strcpy(pwd[selected_part], "");
	}
	else if ((selected_dev = tolower(option) - '0', 0 <= selected_dev) &&
	         selected_dev < num_fat_devices)
	{
		// set up unconditional format of device and all partitions
		flags_to_use =  FDDF_UNCOND_DEV_FORMAT | FDDF_UNCOND_PART_FORMAT |
		                FDDF_MOUNT_PART_ALL;
		if (0 == selected_dev) {
			flags_to_use |= FDDF_MOUNT_DEV_0;
		}
		else if (1 == selected_dev) {
			flags_to_use |= FDDF_MOUNT_DEV_1;
		}
		for (i = 0; i < FAT_MAX_PARTITIONS; ++i) {
			// Reset the working directories to root
		   NS_strcpy(pwd[selected_dev - selected_dev % FAT_MAX_PARTITIONS + i], "");
		}
	} else {
		// out of range format option, report error!
		printf("Error, device or partition '%c' is not available to format.\n",
		       option);
		return -EINVAL;
	}

	if (!option) {
		// format the selected mounted partition
		printf("Unmounting the Partition . . . \n");
		rc = fat_UnmountPartition(fat_part_mounted[selected_part]);
		if (rc) {
			printf("Error: %ls\n", error_message(rc));
		}
		if (!rc) {
			printf("Formatting the Partition . . . \n");
			rc = fat_FormatPartition(
                &(_fat_device_table[selected_part/FAT_MAX_PARTITIONS]),
		           fat_part_mounted[selected_part], selected_part & 3,
                                    FAT_TYPE_16, NULL, NULL);
			if (rc) {
				printf("Error: %ls\n", error_message(rc));
			}
		}
		if (!rc) {
			printf("Mounting the Partition . . . \n");
			rc = fat_MountPartition(fat_part_mounted[selected_part]);
			if (rc) {
				printf("Error: %ls\n", error_message(rc));
			}
		}
	   NS_strcpy(pwd[selected_part], "");	// Reset the working directory to root
	} else {
		// format the selected device and partition
		rc = fat_AutoMount(flags_to_use);
		if (rc) {
			printf("Format %c Error: %ls\n", option, error_message(rc));
		} else {
			printf("format %c succeeded\n", option);
		}
      // Turn off any formatting flags & turn on all mount partition flags
      flags_to_use &= FDDF_MOUNT_DEV_ALL;
		flags_to_use |= FDDF_MOUNT_PART_ALL;
      // Remount any available FAT partitions on given device
      rc = fat_AutoMount(flags_to_use);
	}

	printf("\n");
	// Scan the populated mounted partitions list to find all mounted partitions
	//  and if necessary, select the first mounted partition.
	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS; ++i) {
		if (fat_part_mounted[i] != NULL) {
			printf("Partition %c is mounted.\n", i + 'A');
			if (!fat_part_mounted[active_part]) {
				active_part = i;
			}
		}
	}
	printf("\n");

   return rc;
}

// Main program loop, does command parsing
int main()
{
	int i, rc;
	long prealloc;
	char * p, * q;
	char * cmd;
	int len;
	int type;
	long count, c, sx, ex;
	int max_part, temp_part;
	word flags_to_use;

#ifdef FATFTC_DEBUG_INIT
   _fatftc_init();
#endif

	// ensure that we have sufficient pwd[] entries (if not, increase the value
	//  defined for the MAX_DEVICES macro at the top of this sample)
	assert(MAX_DEVICES >= num_fat_devices);

	// set the maximum possible partition number allowed
	max_part = num_fat_devices * FAT_MAX_PARTITIONS - 1;

	// initialize all possible partitions' pwd[] to empty string
	for (i = 0; i <= max_part; ++i) {
	   NS_strcpy(pwd[i], "");
	}

	// Auto-mount the FAT filesystem
	rc = fat_AutoMount(FDDF_USE_DEFAULT);
   if (rc == -EIO || rc == -ENOMEDIUM) {
		printf("Fatal device initialization error!  Exiting now.\n");
		exit(rc);
   }

	// Scan the populated mounted partitions list to find the first mounted
	// partition for the initial active selection.  The number of configured FAT
	// devices, as well as the mounted partition list, are provided for us in
	// FAT_CONFIG.LIB.
	for (active_part = 0; active_part < num_fat_devices * FAT_MAX_PARTITIONS;
	     ++active_part)
	{
		if (fat_part_mounted[active_part] != NULL) {
			// found a mounted partition, so use it
			break;
		}
	}

	// Check if a mounted partition was found
	if (active_part >= num_fat_devices * FAT_MAX_PARTITIONS) {
      printf("No partition(s) mounted by fat_AutoMount!\n");
      for (i = 0; i < num_fat_devices; i++) {
         if (rc = fat_UnsupportedPartition(i)) {
            printf("\nFound FAT%2d partition on device %d, THIS MAY HOLD DATA!\n",
                   (rc == FAT_TYPE_12 ? 12 : (rc == FAT_TYPE_16 ? 16 : 32)), i);
            if (rc == FAT_TYPE_12) {
               printf("By adding \"#define FAT_FAT12\" you can enable FAT12 support.\n");
               printf("This will allow you to retreive files before re-formatting.\n");
               printf("It is recommended you re-format all media to FAT16, as FAT12\n");
               printf("support will no longer be available after this version.\n\n");
            }
         }
      }
      printf("\nFormat partition 0 on available FAT devices? (y/N)  ");
      gets(buf);
      printf("\n");
      if (tolower(buf[0]) == 'y') {
         // Assumes that we have at most six devices: 0,1,2,3...
         //  And, we're formatting only partition 0 on each device: a,e,i,m,...
         for (i = 0; i < num_fat_devices; ++i) {
            rc = format('0' + i, 0);  // Format device with single partition
            // For backwards compatiblity, pre-2.13 FAT versions would not
            if (rc) { // auto-format partitions, instead would give -EUNFORMAT
              rc = format('A' + (i * FAT_MAX_PARTITIONS), 0);
              if (rc) {
                printf("Format Device Error: %ls.\nExiting now.\n",
                           error_message(rc));
                exit(rc);
              }
            }
         }
      } else {
         // No mounted partition(s) found and format all option refused
         //  ensure rc is set to a FAT error code.
         rc = (rc < 0) ? rc : -ENOPART;
         printf("fat_AutoMount Error: %ls.\nExiting now.\n", error_message(rc));
         exit(rc);
      }
   } else {
      // It is possible that a non-fatal error was encountered and reported,
      // even though fat_AutoMount() succeeded in mounting at least one
      // FAT partition.
      printf("fat_AutoMount succeeded with return code %d.\n\n", rc);
   }

   help();

	// Scan the populated mounted partitions list to find all mounted partitions.
	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS; ++i)
	{
		if (fat_part_mounted[i] != NULL) {
			printf("Partition %c is mounted.\n", i + 'A');
			// select a new active partition, if necessary
			if (active_part >= num_fat_devices) {
				active_part = i;
			}
		}
	}
	printf("\n");

   // Command Parsing Loop - Calls routines above with parsed parameters.
   for (;;) {
   	if (fat_part_mounted[active_part]->first)
      	printf("--- warning: part[%d].first not null, it's 0x%04X\n",
      	       active_part, fat_part_mounted[active_part]->first);
   	printf("%c> ", active_part + 'A' );
      gets(buf);
      p = buf;
      while (isspace(*p)) ++p;
      cmd = p;
      while (isalnum(*p)) ++p;
      q = NULL;
      if (isspace(*p)) {
      	*p = 0;
         q = ++p;
         while (isspace(*q)) ++q;
         if (!*q)
         	q = NULL;
         else
         {
	         p = q + strlen(q) - 1;
   	      while (isspace(*p)) --p;
      	   ++p;
         	*p = 0;	// Zap trailing space
         }
      }
      else {
	      temp_part = tolower(cmd[0]) - 'a';
			if ((strlen(cmd) == 2) && (cmd[1] == ':') && (temp_part >= 0)
      	          && (temp_part <= max_part))
			{
				if (fat_part_mounted[temp_part]) {
					active_part = temp_part;
				}
            else {
					printf("? partition %c is not mounted\n", temp_part + 'A');
				}
            continue;
			}
         else if (*p) {
	      	printf("? unrecognized command\n");
   	      continue;
      	}
     	}
      if (!*cmd)
      	continue;

      // cmd is command string, q is parameter (or NULL).

		if (!strcmpi(cmd, "ls") || !strcmpi(cmd, "dir")) {
      	ls();
      }
      else if (!strcmpi(cmd, "cd")) {
      	if (!q)
            NS_strcpy(pwd[active_part], "");
         else if (*q != '.' || *(q+1) == '.')
         {
          	if (*q == FAT_SLASH_CH)
         	   NS_strcpy(pwd[active_part], q+1);
	         else if (*q == '.' || *(q+1) == '.') {
					q = NS_strrchr(pwd[active_part], FAT_SLASH_CH);
      	      if (q)
         	   	*q = 0;
            	else
            		printf("No previous dir level\n");
	         }
				else {
      	   	len = strlen(q);
         	   if (strlen(pwd[active_part]) + len + 1 < sizeof(pwd[0])) {
            	   NS_strcat(pwd[active_part], FAT_SLASH_STR);
                  NS_strcat(pwd[active_part], q);
	            }
   	         else
      	      	printf("Total dir name too long\n");
         	}
         }
         printf("PWD = '%s'\n", pwd[active_part]);
      }
      else if (!strcmpi(cmd, "pwd")) {
         printf("%s\n", pwd[active_part]);
      }
      else if (!strcmpi(cmd, "touch")) {
      	type = FAT_FILE;
         count = 1;
      __touch:
      	// parms "filename [alc]"
         if (!q || !*q) {
         	printf("? touch/mkdir expects filename [alc]\n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         prealloc = 0;
         if (*p) {
         	prealloc = NS_strtol(p, NULL, 0);
            *p = 0;
         }
         if (count > 1) {
         	p = NS_strchr(q, '.');

         	for (c = 1; c <= count; ++c) {
					if (p) {
               	memcpy(mpath, q, p-q);
                  sprintf(mpath + (p-q), "%ld", c);
                  NS_strcat(mpath, p);
               }
               else {
	               NS_strcpy(mpath, q);
	               sprintf(mpath + strlen(mpath), "%ld", c);
               }
               touch(mpath, prealloc, type);
            }
         }
         else
         	touch(q, prealloc, type);
      }
      else if (!strcmpi(cmd, "mtouch")) {
      	count = NS_strtol(q, &q, 0);
			if (count <= 0) {
         	printf("? mtouch expects repetition count >= 1\n");
            continue;
         }
			while (isspace(*q)) ++q;
         type = FAT_FILE;
         goto __touch;
      }
      else if (!strcmpi(cmd, "wr")) {
         count = 1;
         type = 0;
      __wr:
      	// parms "filename [bytes]"
         if (!q || !*q) {
         	printf("? wr/ap expects filename [bytes]\n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         prealloc = 1024;
         if (*p) {
         	prealloc = NS_strtol(p, NULL, 0);
            *p = 0;
         }
         if (count > 1) {
         	p = NS_strchr(q, '.');

         	for (c = 1; c <= count; ++c) {
					if (p) {
               	memcpy(mpath, q, p-q);
                  sprintf(mpath + (p-q), "%ld", c);
                  NS_strcat(mpath, p);
               }
               else {
	               NS_strcpy(mpath, q);
	               sprintf(mpath + strlen(mpath), "%ld", c);
               }
               wr(mpath, prealloc, type);
            }
         }
         else
         	wr(q, prealloc, type);
      }
      else if (!strcmpi(cmd, "mwr")) {
         type = 0;
      __mwr:
      	count = NS_strtol(q, &q, 0);
			if (count <= 0) {
         	printf("? mwr expects repetition count >= 1\n");
            continue;
         }
			while (isspace(*q)) ++q;
         goto __wr;
      }
      else if (!strcmpi(cmd, "ap")) {
         count = 1;
         type = 1;
         goto __wr;
      }
      else if (!strcmpi(cmd, "map")) {
         type = 1;
         goto __mwr;
      }
      else if (!strcmpi(cmd, "mkdir")) {
      	type = FAT_DIR;
         count = 1;
         goto __touch;
      }
      else if (!strcmpi(cmd, "mmkdir")) {
      	count = NS_strtol(q, &q, 0);
			if (count <= 0) {
         	printf("? mmkdir expects repetition count >= 1\n");
            continue;
         }
			while (isspace(*q)) ++q;
         type = FAT_DIR;
         goto __touch;
      }
      else if (!strcmpi(cmd, "rd")) {
         count = 1;
         type = 0;
      __rd:
      	// parms "filename [bytes]"
         if (!q || !*q) {
         	printf("? rd expects filename [bytes]\n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         prealloc = 1024;
         if (*p) {
         	prealloc = NS_strtol(p, NULL, 0);
            *p = 0;
         }
         if (count > 1) {
         	p = NS_strchr(q, '.');

         	for (c = 1; c <= count; ++c) {
					if (p) {
               	memcpy(mpath, q, p-q);
                  sprintf(mpath + (p-q), "%ld", c);
                  NS_strcat(mpath, p);
               }
               else {
	               NS_strcpy(mpath, q);
	               sprintf(mpath + strlen(mpath), "%ld", c);
               }
               rd(mpath, prealloc, type);
            }
         }
         else
         	rd(q, prealloc, type);
      }
      else if (!strcmpi(cmd, "del") || !strcmpi(cmd, "rm")) {
         count = 1;
         type = FAT_FILE;
         if (!q || !*q) {
         	printf("? del expects filename \n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         *p = 0;
         del(q, type);
      }
      else if (!strcmpi(cmd, "rmdir")) {
         count = 1;
         type = FAT_DIR;
         if (!q || !*q) {
         	printf("? rmdir expects directory name \n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         *p = 0;
         del(q, type);
      }
      else if (!strcmpi(cmd, "split")) {
      	// parms "filename newfile"
         if (!q || !*q) {
         	printf("? split expects filename newfile\n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         if (*p) {
	         *p++ = 0;
   	      while (*p && isspace(*p)) ++p;
         }
         if (!*p) {
         	printf("? split expects filename newfile\n");
            continue;
         }
        	split(q, p);
      }
      else if (!strcmpi(cmd, "trunc")) {
      	// parms "filename newfile"
         if (!q || !*q) {
         	printf("? trunc expects filename [bytes]\n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         c = FAT_BRK_END;
         if (*p) {
            *p++ = 0;
         	while (isspace(*p)) ++p;
         	c = NS_strtol(p, NULL, 0);
            if (!c && *p != '0')
            	c = FAT_BRK_END;
         }
        	trunc(q, c);
      }
      else if (!strcmpi(cmd, "tail")) {
         count = 1;
         type = 1;
         goto __rd;
      }
      else if (!strcmpi(cmd, "pdump")) {
      	pdump();
      }
      else if (!strcmpi(cmd, "fat")) {
			sx = 2;
         ex = 64;
         if (q) {
         	sx = NS_strtol(q, &q, 0);
            while (isspace(*q)) ++q;
            if (*q)
            	ex = NS_strtol(q, &q, 0);
         }
      	fat_print(sx, ex);
      }
      else if (!strcmpi(cmd, "stat")) {
         if (!q || !*q) {
         	printf("? stat expects filename \n");
            continue;
         }
         p = q;
         while (*p && !isspace(*p)) ++p;
         *p = 0;
         stat(q);
      }
      else if (!strcmpi(cmd, "format")) {
      	// format partition or device with no additional override flags
      	if (q) {
	      	format(*q, 0);
      	} else {
	      	format(0, 0);
      	}
      }
      else if ((!strcmpi(cmd, "h")) || (!strcmpi(cmd, "help"))){
      	help();
      }
      else if (!strcmpi(cmd, "exit")) {
      	// must unmount all of the mounted FAT partitions & devices
      	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS;
         											 i += FAT_MAX_PARTITIONS) {
	      	if (fat_part_mounted[i]) {
	      		rc = fat_UnmountDevice(fat_part_mounted[i]->dev);
               if (rc < 0)
                  printf("Unmount Error on %c: %ls\n", 'A' + i,
                                 error_message(rc));
	      	}
	      }

      	break;
      }
      else
      	printf("? unrecognized command\n");
   }

   return 0;
}