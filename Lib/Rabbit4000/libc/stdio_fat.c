/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Portions of stdio.h specific to the FAT API.


	DEVNOTE: Need to add timeouts to some of these busy loops.
*/

/*** BeginHeader */
#include <stdio.h>

#ifdef STDIO_FAT_DEBUG
	#define _stdio_fat_debug __debug
#else
	#define _stdio_fat_debug __nodebug
#endif

/*

*/
#define _downcast_FATfile(faddr)	((FATfile *)(unsigned int)(faddr))
/*** EndHeader */


/*** BeginHeader fopen */
// define masking macro
#define fopen( filename, mode)	freopen( filename, mode, _stdio_FILE_alloc())
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fopen                                                            <stdio.h>

SYNTAX:	FILE far *fopen( const char *filename, const char *mode)

DESCRIPTION:	Open a file in the FAT filesystem as a stream.

PARAMETER 1:	Name of file to open.

PARAMETER 2:	A string beginning with one of the following sequences
					(additional characters may follow):

					r					Open text file for reading.
					w					Create (or truncate to zero length) a text file
												for writing.
					a					Open or create a text file for writing at
												end-of-file.

					rb					Open binary file for reading.
					wb					Create (or truncate to zero length) a binary file
												for writing.
					ab					Open or create a binary file for writing at
												end-of-file.

					r+					Open text file for update (read and write).
					w+					Create (or truncate to zero length) a text file
												for update.
					a+					Open or create a text file for update, writing
												at end of file.

					r+b or rb+		Open binary file for update (read and write).
					w+b or wb+		Create (or truncate to zero length) a binary file
												for update.
					a+b or ab+		Open or create a binary file for update, writing
												at end of file.

				Opening a file with read mode ('r' as the first character in the
				mode argument) fails if the file does not exist or cannot be read.

				Opening a file with append mode ('a' as the first character in the
				mode argument) causes all subsequent writes to the file to be forced
				to the then current end-of-file, regardless of intervening calls to
				the fseek function.

				When a file is opened with update mode ('+' as the second or third
				character in the mode argument), both read and write may be
				performed on the associated stream.  However, write may not be
				directly followed by input without an intervening call to the fflush
				function or to a file positioning function (fseek, fsetpos, or
				rewind), and read may not be directly followed by write without an
				intervening call to a file positioning function, unless the input
				operation encounters end-of-file.

				When opened, a stream is fully buffered if and only if it can be
				determined not to refer to an interactive device (e.g., stdin,
				stdout).  The error and end-of-file indicators for the stream are
				cleared.

RETURN VALUE:	Returns a pointer (FILE far *) to the object controlling the
					stream.  On error, returns NULL.

SEE ALSO:	freopen, fread, fwrite, fseek, fclose

END DESCRIPTION **********************************************************/
/*
	DEVNOTE: We plan to eventually support other types of streams, possibly by
				passing a URL to fopen or creating a new fopen_url.

				If we have fopen() do the dispatching, then the function below
				would become fopen_fat() and handle any filename that uses the
				"file://" scheme or is clearly a file path and not a URL.
*/
// parens around function name are necessary due to masking macro
_stdio_debug
FILE __far *(fopen)( const char *filename, const char *mode)
{
	// Find an available entry in _stdio_files[] using a sub-function.
	// Pass that stream into freopen and have it do all of the work.
	return freopen( filename, mode, _stdio_FILE_alloc());
}


/*** BeginHeader freopen */
/*** EndHeader */
#ifndef __FAT16_LIB
	#fatal This function requires FAT.  #use "fat16.lib" in your program.
#endif
/* START FUNCTION DESCRIPTION ********************************************
freopen                                                          <stdio.h>

SYNTAX:	FILE far *freopen( const char *filename, const char *mode,
																				FILE far *stream)

DESCRIPTION:	Opens <filename> and associates it to <stream>.

PARAMETER 1:	Name of file to open.

PARAMETER 2:	Identical to the mode parameter to fopen().

PARAMETER 3:	Stream to associate with open file.  This should be a value
					returned from a previous call to fopen() or one of the macros
					stdin, stderr or stdout.

RETURN VALUE:	NULL if opening the file fails, <stream> on success.

SEE ALSO:	fopen, fread, fwrite, fseek, fclose

END DESCRIPTION **********************************************************/
_stdio_debug
FILE __far *freopen( const char *filename, const char *mode, FILE __far *stream)
{
	fat_part			*part;
	char				*localfn;
	int				error;
	FATfile			*fatfile;
	unsigned int	streamflags;
	int				fatflags;
	int				m1, m2;

	if (! (stream && filename && mode))
	{
		return NULL;
	}

	// close the file if already open, ignoring any errors
	if (stream->flags & _FILE_FLAG_OPEN)
	{
		// fclose() resets the FILE object to all zeros
		fclose( stream);
	}

	streamflags = _FILE_FLAG_OPEN | _FILE_FLAG_USED | _FILE_FLAG_BUF_FULL;
	switch (*mode)
	{
		case 'r':		// read
			streamflags |= _FILE_FLAG_OPEN_READ;
			break;

		case 'w':		// write
			streamflags |= _FILE_FLAG_OPEN_WRITE;
			break;

		case 'a':		// append
			streamflags |= _FILE_FLAG_OPEN_WRITE | _FILE_FLAG_OPEN_APPEND;
			break;

		default:			// invalid mode
			return NULL;
	}

	m1 = mode[1];
	m2 = mode[2];
	if (m1 == '+' || m2 == '+')		// update (read & write)
	{
		streamflags |= _FILE_FLAG_OPEN_RW;
	}
	if (m1 == 'b' || m2 == 'b')
	{
		streamflags |= _FILE_FLAG_OPEN_BINARY;
	}

	fatflags = (streamflags & _FILE_FLAG_OPEN_WRITE) ? FAT_CREATE : FAT_READONLY;

	fatfile = _root_calloc( 1, sizeof *fatfile);

	// try to open the file with the fat_Open, return error on failure,
	// spin on -EBUSY return
	error = fat_GetPartition( &part, (const char **)&localfn, filename);
	#ifdef STDIO_FAT_VERBOSE
		if (error)
		{
			fprintf( stderr, "%s: %s returned %d\n",
				__FUNCTION__, "fat_GetPartition", error);
		}
	#endif
	if (! error)
	{
		do {
			error = fat_Open( part, localfn, FAT_FILE, fatflags, fatfile, NULL);
		} while (error == -EBUSY);
		#ifdef STDIO_FAT_VERBOSE
			if (error)
			{
				fprintf( stderr,
					"%s: %s returned %d for FATfile @ 0x%04x\n",
					__FUNCTION__, "fat_Open", error, fatfile);
			}
		#endif
	}

	// if fat_Open is OK and the basic access mode is 'w' then truncate,
	// spin on -EBUSY return
	if (!error && 'w' == *mode)
	{
		do {
			error = fat_Truncate(fatfile, 0L);
		} while (error == -EBUSY);
		#ifdef STDIO_FAT_VERBOSE
			if (error)
			{
				fprintf(stderr,
					"%s: %s returned %d for FATfile @ 0x%04x\n",
					__FUNCTION__, "fat_Truncate", error, fatfile);
			}
		#endif
	}

	if (error)
	{
		return NULL;
	}

	stream->flags = streamflags;
	_STDIO_FILE_UPDATE_CAN_RW(stream);

	stream->cookie = fatfile;

	// Populate the function pointers.  Assign NULL to read or write if the file
	// is opened write-only or read-only.
	if (streamflags & _FILE_FLAG_OPEN_READ)
	{
		stream->read = _stream_fat_read;
	}
	if (streamflags & _FILE_FLAG_OPEN_WRITE)
	{
		stream->write = _stream_fat_write;
	}
	stream->seek = _stream_fat_seek;
	stream->close = _stream_fat_close;

	return stream;
}


/*** BeginHeader tmpfile */
/*** EndHeader */
#ifndef __FAT16_LIB
	#fatal This function requires FAT.  #use \"fat16.lib\" in your program.
#endif
/* START FUNCTION DESCRIPTION ********************************************
tmpfile                                                          <stdio.h>

SYNTAX:	FILE far *tmpfile( void)

DESCRIPTION:	Creates a temporary binary file (in "wb+" mode) that is
					automatically deleted when it is closed.

RETURN VALUE:	Returns a pointer to the opened file or NULL if the file cannot
					be created.

SEE ALSO: tmpnam

END DESCRIPTION **********************************************************/
_stdio_fat_debug
FILE __far *tmpfile( void)
{
	char name[L_tmpnam];
	FILE __far *stream;

	// get a filename using tmpnam() and local var
	tmpnam( name);

	// open it with fopen
	stream = fopen( name, "wb+");

	// on success, copy the name into the tmpnam member of the FILE structure
	if (stream)
	{
		_f_strcpy( stream->tmpnam, name);
	}

	return stream;
}


/*** BeginHeader tmpnam */
/*** EndHeader */
#ifndef __FAT16_LIB
	#fatal This function requires FAT.  #use "fat16.lib" in your program.
#endif
/* START FUNCTION DESCRIPTION ********************************************
tmpnam                                                           <stdio.h>

SYNTAX:	char *tmpnam( char *s)

DESCRIPTION:	Generates a string that is a valid file name and that is not the
					same as the name of an existing file.

					The tmpnam function generates a different string each time it
					is called, up to TMP_MAX times.

					In the current implementation, uses the pattern A:TEMP####.TMP
					to generate filenames.

PARAMETER 1:	Buffer to hold the filename.  Must be at least L_tmpnam
					characters.

					If NULL, tmpnam() will store the name in a static buffer.
					Subsequent calls to tmpnam() may modify that buffer, making
					it a less-robust method than passing in a buffer to use.

RETURN VALUE:	Buffer containing filename (either the first parameter or
					a static buffer if the first parameter is NULL).

END DESCRIPTION **********************************************************/
_stdio_fat_debug
char *tmpnam( char *s)
{
	static char static_name[L_tmpnam];
	static word lastnum = 0xFFFF;

	int 			status;
	fat_part		*part;
	char			*fn;
	fat_dirent	dirent;		// dummy entry used during check for file

	// use a static var to keep track of last number used to generate file
	// first time through, use lower bits of RTC to seed random generator
	if (lastnum == 0xFFFF)
	{
		WrPortI( RTC0R, NULL, 0);
		lastnum = RdPortI16( RTC0R) & 0x03FF;	// lastnum 0 to 1023
	}

	if (s == NULL)
	{
		s = static_name;
	}

	// <s> will be "A:TEMP####.TMP".  <part> is the partition, <fn> is the
	// filename (the "TEMP####.TMP" part of <s>).
	part = fat_part_mounted[0];
	strcpy( s, "A:");
	fn = &s[2];

	do {
		// lastnum ranges from 0 to 9999
		if (++lastnum > 9999)
		{
			lastnum = 0;
		}
		sprintf( fn, "TEMP%04u.TMP", lastnum);
		do {
			status = fat_Status( part, fn, &dirent);
		} while (status == -EBUSY);
		#ifdef STDIO_FAT_VERBOSE
			fprintf( stderr, "%s: %s %s\n", __FUNCTION__, s,
				status ? "is available" : "exists");
		#endif
	} while (status == 0);

	return s;
}

/*** BeginHeader remove */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
remove                                                           <stdio.h>

SYNTAX:	int remove( const char *filename)

DESCRIPTION:	Deletes a file from the FAT filesystem.

PARAMETER 1:	Full pathname of file to delete (e.g., "A:/file.txt").

RETURN VALUE:	0 for success, non-zero on failure
              -EIO on device IO error
				  -EINVAL or -EPATHSTR if filename is NULL or invalid
				  -EPERM if the file is open, write protected, hidden or system
				  -ENOENT if file does not exist

					-NOSYS if FAT support has not been compiled into the program

SEE ALSO:	rename, fat_Delete, fat_GetPartition

END DESCRIPTION **********************************************************/
_stdio_fat_debug
int remove( const char *filename)
{
	#ifndef __FAT16_LIB
		return -ENOSYS;
	#else
		fat_part			*part;
		char				*localfn;
		int				error;

		// call fat_GetPartition to split filename into partition and name
		error = fat_GetPartition( &part, (const char **)&localfn, filename);
		if (error)
		{
			#ifdef STDIO_FAT_VERBOSE
				fprintf( stderr, "%s: %s returned %d\n",
					__FUNCTION__, "fat_GetPartition", error);
			#endif
			return error;
		}

		// spin on fat_Delete until it returns something other than -EBUSY
		// we will likely need to add timeouts
		do
		{
			error = fat_Delete( part, FAT_FILE, localfn);
		} while (error == -EBUSY || error == -EPSTATE);
		#ifdef STDIO_FAT_VERBOSE
			if (error)
			{
				fprintf( stderr, "%s: %s returned %d\n",
					__FUNCTION__, "fat_Delete", error);
			}
		#endif

		return error;
	#endif
}

/*** BeginHeader rename */
/*** EndHeader */
#warns "rename() is not implemented, and always returns -ENOSYS."
/* START FUNCTION DESCRIPTION ********************************************
rename                                                           <stdio.h>

SYNTAX:	int rename( const char *old, const char *new)

DESCRIPTION:	Rename a file in the FAT filesystem.

PARAMETER 1:	Full pathname of file to rename.

PARAMETER 2:	New name for file.  Path must be on the same partition, and
					target directory must already exist.

					New name can either be a bare filename ("newfile.txt") if the
					file should remain in the current directory, or a fully
					qualified path ("A:/dirname/newfile.txt") to move the file
					to another directory.

RETURN VALUE:	Until Dynamic C's FAT library supports file renaming, this
					function will always return -ENOSYS.

					0 on success, non-zero on failure.
					(possible errors depend on how this function is implemented)

SEE ALSO:	remove

END DESCRIPTION **********************************************************/
_stdio_fat_debug
int (rename)( const char *old, const char *new)
{
	// need to create new function in FAT16.LIB

	// fail if <new> already exists

	// If directory name doesn't change, simply overwrite the filename in
	// the directory entry with the new name.

	// If moving to another directory, create a hard link to the file (two
	// directory entries pointing to the same clusters) and then remove the
	// old directory entry.

	/*
					-EINVAL if <old> or <new> are NULL
					-ENOENT if file <old> doesn't exist
					-EEXIST if there is already a file named <new>
					-EPERM if the file is open, write protected, hidden or system
					-E??? if <old> and <new> aren't on the same partition.
	*/
	return -ENOSYS;
}



/*** BeginHeader _stream_fat_read, _stream_fat_write,
						_stream_fat_seek, _stream_fat_close */
size_t _stream_fat_read( void __far *cookie, void __far *buffer, size_t bytes);
size_t _stream_fat_write( void __far *cookie, const void __far *buffer,
																						size_t bytes);
int _stream_fat_seek( void __far *cookie, long int *offset, int whence);
int _stream_fat_close( void __far *cookie);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_stream_fat_read                                             <stdio_fat.c>

	Standard .read function for FILE object.  See _stream_read() for API.

END DESCRIPTION **********************************************************/
_stdio_fat_debug
size_t _stream_fat_read( void __far *cookie, void __far *buffer, size_t bytes)
{
	int len;
	int read;

	// limit reads to 8KB (arbitrary limit) at a time
	len = (bytes > 8192) ? 8192 : bytes;
	do
	{
		read = fat_xRead( _downcast_FATfile( cookie), buffer, len);
	} while (read == 0);
	#ifdef STDIO_FAT_VERBOSE
		fprintf( stderr, "%s: read %d bytes\n", __FUNCTION__, read);
	#endif

	if (read == -EEOF)
	{
		return 0;			// EOF isn't really an error, just return 0 bytes
	}

	// return number of bytes read, or error
	return read;
}


/* START _FUNCTION DESCRIPTION ********************************************
_stream_fat_write                                            <stdio_fat.c>

	Standard .write function for FILE object.  See _stream_write() for API.

END DESCRIPTION **********************************************************/
_stdio_fat_debug
size_t _stream_fat_write( void __far *cookie, const void __far *buffer,
																						size_t bytes)
{
	int len;
	int wrote;

	// limit writes to 4KB (arbitrary limit) at a time
	len = (bytes > 4096) ? 4096 : bytes;
	do
	{
		wrote = fat_xWrite( _downcast_FATfile( cookie), (long) buffer, len);
	} while (wrote == 0 || wrote == -EBUSY);
	#ifdef STDIO_FAT_VERBOSE
		if (wrote < 0)
		{
			fprintf( stderr, "%s: error %d (%ls)\n", __FUNCTION__, wrote,
				strerror( wrote));
		}
		else
		{
			fprintf( stderr, "%s: wrote %d bytes\n", __FUNCTION__, wrote);
		}
	#endif

	// return number of bytes written, or error returned from fat_xWrite
	return wrote;
}


/* START _FUNCTION DESCRIPTION ********************************************
_stream_fat_seek                                             <stdio_fat.c>

	Standard .seek function for FILE object.  See _stream_seek() for API.

END DESCRIPTION **********************************************************/
_stdio_fat_debug
int _stream_fat_seek( void __far *cookie, long int *offset, int whence)
{
	int error;
	unsigned long pos;

	do
	{
		error = fat_Seek( _downcast_FATfile( cookie), *offset, whence);
	} while (error == -EBUSY);

	#ifdef STDIO_FAT_VERBOSE
		if (error)
		{
			fprintf( stderr,
				"%s: %s returned %d for FATfile @ 0x%04x\n",
				__FUNCTION__, "fat_Seek", error, _downcast_FATfile( cookie));
		}
	#endif

	if (! error)
	{
		error = fat_Tell( _downcast_FATfile( cookie), &pos);
		#ifdef STDIO_FAT_VERBOSE
			if (error)
			{
				fprintf( stderr,
					"%s: %s returned %d for FATfile @ 0x%04x\n",
					__FUNCTION__, "fat_Tell", error, _downcast_FATfile( cookie));
			}
		#endif
	}

	if (error)
	{
		return -1;
	}

	*offset = pos;
	return 0;
}

/* START _FUNCTION DESCRIPTION ********************************************
_stream_fat_close                                            <stdio_fat.c>

	Standard .close function for FILE object.  See _stream_close() for API.

END DESCRIPTION **********************************************************/
_stdio_fat_debug
int _stream_fat_close( void __far *cookie)
{
	int error;

	do
	{
		error = fat_Close( _downcast_FATfile( cookie));
	} while (error == -EBUSY);

	#ifdef STDIO_FAT_VERBOSE
		if (error)
		{
			fprintf( stderr,
				"%s: %s returned %d for FATfile @ 0x%04x\n",
				__FUNCTION__, "fat_Close", error, _downcast_FATfile( cookie));
		}
	#endif

	// free root memory allocated for FATfile
	_root_free( _downcast_FATfile( cookie));
}


