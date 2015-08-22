/*
	stdio.c

	Copyright (c)2008-2010 Digi International Inc., All Rights Reserved

	This software contains proprietary and confidential information of Digi
	International Inc.  By accepting transfer of this copy, Recipient agrees
	to retain this software in confidence, to prevent disclosure to others,
	and to make no use of this software other than that for which it was
	delivered.  This is a published copyrighted work of Digi International
	Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
	prohibited.

	Restricted Rights Legend

	Use, duplication, or disclosure by the Government is subject to
	restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
	Technical Data and Computer Software clause at DFARS 252.227-7031 or
	subparagraphs (c)(1) and (2) of the Commercial Computer Software -
	Restricted Rights at 48 CFR 52.227-19, as applicable.

	Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
*/

/*
	Some of the functions in this library are implemented with "masking macros".

		#define foo( a, b, c)		some_other_func( a, b, c, d, e)
		int (foo)( int a, int b, int c)
		{
			return some_other_func( a, b, c, d, e);
		}

	This method saves the extra function call to function foo by defining a
	macro foo.  But, if user code #undef's foo, or needs a function pointer to
	foo, they'll get to the underlying function.

	See section 7.1.7 of the C90 standard, or 4.1.6 of the C89 standard
	for details.
*/

/*** BeginHeader */

#ifdef STDIO_DEBUG
	#define _stdio_debug __debug
#else
	#define _stdio_debug __nodebug
#endif


// Macro to update the _CAN_READ and _CAN_WRITE flags based on _OPEN_READ and
// _OPEN_WRITE flags.  Used after flushing or seeking on a stream.
#define _STDIO_FILE_UPDATE_CAN_RW(stream)	\
	(stream)->flags |= (((stream)->flags & _FILE_FLAG_OPEN_RW) << 4)
/*** EndHeader */

/*
	The following function help represent the generic API for the four
	function pointers in the FILE object.  Function help for the actual
	functions should be marked private and include a reference to one
	of the following.
*/

/* START _FUNCTION DESCRIPTION ********************************************
_stream_read                                                      <stdio.h>

SYNTAX:	size_t _stream_read( void far *cookie, void far *buffer, size_t bytes)

DESCRIPTION:	Any function assigned to the "read" function pointer of the
					FILE structure must use this calling convention.  STDIO will
					call this function to read data into a buffer associated with
					the FILE structure.

					Code in the STDIO library that calls this function will update
					the stream offset and the EOF and ERROR flags.

					The "read" function pointer is NULL if the stream isn't readable
					(and as a result will always return EOF on attempts to read it).

PARAMETER 1:	The cookie stored in the FILE structure.

PARAMETER 2:	Location to store bytes read from the data source.

PARAMETER 3:	Number of bytes to read.

RETURN VALUE:	>= 0, number of bytes actually read.  If the stream is "busy",
							it may return 0 and must be called again.
					EOF if the stream has reached "end-of-file"
					Any other negative value for an error.

SEE ALSO:	_stream_write, _stream_seek, _stream_close

END DESCRIPTION **********************************************************/
/* START _FUNCTION DESCRIPTION ********************************************
_stream_write                                                    <stdio.h>

SYNTAX:	size_t _stream_write( void far *cookie, const void far *buffer,
																						size_t bytes)

DESCRIPTION:	Any function assigned to the "write" function pointer of the
					FILE structure must use this calling convention.  STDIO will
					call this function to write data to the stream from a buffer
					associated with the FILE structure.

					The "write" function pointer is NULL if the stream isn't
					writeable.

PARAMETER 1:	The cookie stored in the FILE structure.

PARAMETER 2:	Source of bytes to write to the data source.

PARAMETER 3:	Number of bytes to write.

RETURN VALUE:	>= 0, number of bytes actually written.  If the stream is "busy",
							the function may return 0 and must be called again.
					Any negative value for an error.

SEE ALSO:	_stream_read, _stream_seek, _stream_close

END DESCRIPTION **********************************************************/
/* START _FUNCTION DESCRIPTION ********************************************
_stream_seek                                                      <stdio.h>

SYNTAX:	int _stream_seek( void far *cookie, long int *offset, int whence)

DESCRIPTION:	Any function assigned to the "seek" function pointer of the
					FILE structure must use this calling convention.  STDIO will
					call this function to seek to a different position in the
					stream.

					The "seek" function pointer is NULL if a stream type is not
					seekable.

PARAMETER 1:	The cookie stored in the FILE structure.

PARAMETER 2:	Offset for seek.  Before returning, function will update
					*offset to the new, absolute stream offset.

PARAMETER 3:	SEEK_SET if <offset> is from the start of the stream.
					SEEK_CUR if <offset> is from the current position.
					SEEK_END if <offset> is from the end of the stream.

RETURN VALUE:	0 on success, -1 on failure (this may need to change)

SEE ALSO:	_stream_read, _stream_write, _stream_close

END DESCRIPTION **********************************************************/
/* START _FUNCTION DESCRIPTION ********************************************
_stream_close                                                     <stdio.h>

SYNTAX:	int _stream_close( void far *cookie)

DESCRIPTION:	Any function assigned to the "close" function pointer of the
					FILE structure must use this calling convention.  STDIO will
					call this function when closing a stream.

					The "close" function pointer is NULL if a stream doesn't need
					to do anything on close.

PARAMETER 1:	The cookie stored in the FILE structure.

RETURN VALUE:	0 on success, non-zero for error.

					Note that fclose() returns EOF for all non-zero return values.

SEE ALSO:	_stream_read, _stream_write, _stream_seek

END DESCRIPTION **********************************************************/


/*** BeginHeader _stdio_stream_name */
char *_stdio_stream_name( FILE __far *stream);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_stdio_stream_name                                               <stdio.c>

SYNTAX:	char *_stdio_stream_name( FILE far *stream)

DESCRIPTION:	Returns a description of the given stream:
						<stdin>, <stdout>, <stderr>
						<file #x> (if FILE is in the _stdio_files global)
						<FILE @123456> (far address of FILE object)

					Used for debugging.

PARAMETER 1:	Stream to identify.

RETURN VALUE:	Pointer to a string literal or static buffer with description
					of stream.

END DESCRIPTION **********************************************************/
_stdio_debug
char *_stdio_stream_name( FILE __far *stream)
{
	// debug function
	static char buffer[15];
	long filenum;

	filenum = (stream - &_stdio_files[0]);

	if (stream == stdin)				return "<stdin>";
	else if (stream == stdout)		return "<stdout>";
	else if (stream == stderr)		return "<stderr>";
	else if (filenum >= 0 && filenum < FOPEN_MAX)
	{
		snprintf( buffer, sizeof buffer, "<file #%d>", (int) filenum);
	}
	else
	{
		snprintf( buffer, sizeof buffer, "<FILE @%06lx>", stream);
	}

	return buffer;
}

/*** BeginHeader _stdio_file_invalid */
// Use this macro to verify a FILE object.  Could become more complex (by
// checking a magic marker?) if deemed necessary in the future.

#ifdef STDIO_VERBOSE
	// function call with verbose output
	#define _STDIO_FILE_INVALID(s)		_stdio_file_invalid(s, __FUNCTION__)
#else
	// simple macro
	#define _STDIO_FILE_INVALID(stream)		\
		(! (stream && stream->flags & _FILE_FLAG_USED))
#endif
int _stdio_file_invalid( FILE __far *stream, char *funcname);
/*** EndHeader */
int _stdio_file_invalid( FILE __far *stream, char *funcname)
{
	if (stream && stream->flags & _FILE_FLAG_USED)
	{
		return 0;
	}

	if (stream != stderr)
	{
		fprintf( stderr, "%s: %s invalid, returning error\n", funcname,
			_stdio_stream_name( stream));
	}
	return 1;
}

/*** BeginHeader _stdio_files, _stdio_files_init */
void _stdio_files_init(void);
/*** EndHeader */
#if FOPEN_MAX < 3
	#warns "FOPEN_MAX must be set to at least 3 (for stdin, stdout and stderr)"
	#undef FOPEN_MAX
	#define FOPEN_MAX 3
#endif

/*
Default Buffering modes, according to ANSI Standard:

	* stdin is always buffered
	* stderr is always unbuffered
	* if stdout is a terminal then buffering is automatically set to
		line buffered, else it is set to buffered

Our buffering modes:
	* stdin is read a character at a time
	* stdout and stderr are "semi-buffered"
		each printf, fwrite, etc. call is buffered and automatically flushed
*/

// buffers used for semi-buffered stdout
__far char _stdout_buf[128];
__far char _stderr_buf[128];

__far FILE _stdio_files[FOPEN_MAX];

// Note that stdio won't work until this function runs.
// It is called from premain() so GLOBAL_INIT functions can use stdio.
__nodebug
void _stdio_files_init(void)
{
	_f_memset( _stdio_files, 0, sizeof _stdio_files);

	// set up stdin
	_stdio_files[0].flags = _FILE_FLAG_USED
			| _FILE_FLAG_OPEN_READ
			| _FILE_FLAG_CAN_READ
			| _FILE_FLAG_OPEN;
	_stdio_files[0].read = _stream_stdin_read;

	// set up stdout and stderr
	_stdio_files[1].flags = _stdio_files[2].flags = _FILE_FLAG_USED
			| _FILE_FLAG_BUF_FULL
			| _FILE_FLAG_AUTOFLUSH
			| _FILE_FLAG_OPEN_WRITE
			| _FILE_FLAG_CAN_WRITE
			| _FILE_FLAG_OPEN;
	_stdio_files[1].write = _stdio_files[2].write = _stream_stdout_write;
	_stdio_files[1].bufsize = sizeof _stdout_buf;
	_stdio_files[1].buffer_base = _stdout_buf;
	_stdio_files[2].bufsize = sizeof _stderr_buf;
	_stdio_files[2].buffer_base = _stderr_buf;
}

/*** BeginHeader _stdio_FILE_alloc, _stdio_FILE_free */
FILE __far *_stdio_FILE_alloc();
int _stdio_FILE_free( FILE __far *f);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_stdio_FILE_alloc                                                <stdio.h>

SYNTAX:	FILE far *_stdio_FILE_alloc()

DESCRIPTION:	Find an unused FILE structure in the _stdio_files array.  This
					function should be always be used for getting a FILE from
					_stdio_files[].

PARAMETER:	None

RETURN VALUE:	Stream pointer or NULL if all entries are in use.  If necessary,
					increase FOPEN_MAX.

SEE ALSO:	_stdio_FILE_free

END DESCRIPTION **********************************************************/
_stdio_debug
FILE __far *_stdio_FILE_alloc()
{
	int i;
	FILE __far *f;

	for (f = _stdio_files; f < &_stdio_files[FOPEN_MAX]; ++f)
	{
		if (! (f->flags & _FILE_FLAG_USED))
		{
			f->flags = _FILE_FLAG_USED;
			return f;
		}
	}

	return NULL;
}

/* START _FUNCTION DESCRIPTION ********************************************
_stdio_FILE_free                                                 <stdio.h>

SYNTAX:	int _stdio_FILE_free( FILE far *f)

DESCRIPTION:	Marks the FILE structure in the _stdio_files array as unused.
					After calling _stdio_FILE_free, caller should not make use of
					stream <f>.

PARAMETER 1:	Stream to free.  Caller should close the stream before calling
					this function.

RETURN VALUE:	0 if stream <f> was freed
					-EINVAL if <f> was not a valid stream

END DESCRIPTION **********************************************************/
_stdio_debug
int _stdio_FILE_free( FILE __far *f)
{
	// make sure f is a valid FILE far * in the _stdio_files array
	// subtract address of file 0, make sure it's not > address of last file,
	// and maybe even take modulo of sizeof(FILE) to make sure it's one of
	// the actual entries

	// if not valid, return -EINVAL

	// DEVNOTE: What if stream is stderr, stdin or stdout?  We can't really
	// mark those entries as free, can we?

	f->flags &= ~_FILE_FLAG_USED;
	if (f->buffer_alloc)
	{
		free( f->buffer_alloc);
		f->buffer_alloc = 0;
	}

	return 0;
}


/*** BeginHeader fflush */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fflush                                                           <stdio.h>

SYNTAX:	int fflush( FILE far *stream)

DESCRIPTION:	If <stream> is an output stream or an update stream that was
					most recently written to, the fflush() function writes any
					buffered data for that stream out to the filesystem.

PARAMETER 1:	Stream to flush or NULL to flush all streams with buffered
					(unwritten) data.

RETURN VALUE:	0 on success, EOF if a write error occurs.

END DESCRIPTION **********************************************************/
_stdio_debug
int fflush( FILE __far *stream)
{
	size_t send, sent;
	int error;
	char __far *p, *end;

	// if <stream> is NULL, loop through all _stdio_files looking for open
	// files with unwritten data
	if (! stream)
	{
		stream = _stdio_files;
		while (stream < &_stdio_files[FOPEN_MAX])
		{
			if ((stream->flags & (_FILE_FLAG_USED | _FILE_FLAG_CAN_WRITE))
									==	(_FILE_FLAG_USED | _FILE_FLAG_CAN_WRITE))
			{
				fflush( stream);
			}
			++stream;
		}

		return 0;
	}

	if (stream->write_cur)
	{
		// see if there is data to write
		p = stream->buffer_base;
		end = stream->write_cur;
		send = (size_t) (end - p);
		if (send)
		{
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: %s flushing %u bytes\n",
					__FUNCTION__, _stdio_stream_name( stream), send);
			#endif
			sent = _stream_wrapped_write( stream, p, send);

			// what do you do with a partial write?!
			if (sent < send)
			{
				// if partial write, move unwritten data to start of buffer
				_f_memcpy( p, p + sent, send - sent);
				stream->write_cur = p + send - sent;

				return EOF;
			}

			// stream buffer is now empty
			stream->write_cur = stream->buffer_base;
		}
	}

	_STDIO_FILE_UPDATE_CAN_RW( stream);

	return 0;
}

/*** BeginHeader fclose */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fclose                                                           <stdio.h>

SYNTAX:	int fclose( FILE far *stream)

DESCRIPTION:	Flushes <stream> and closes the associated file.  This function
					will block while writing buffered data to the stream.  Any
					unread buffered data is discarded.  The stream is disassociated
					with the file.

PARAMETER 1:	Stream to close.

RETURN VALUE:	0 if the stream was successfully closed or EOF if any errors
					were detected.

END DESCRIPTION **********************************************************/
_stdio_debug
int fclose( FILE __far *stream)
{
	// need local copy of temp name in root, since remove can't handle far
	char tmpnam_local[L_tmpnam];

	int retval = 0, error;

	// DEVNOTE: What if stream is stderr, stdin or stdout?  We can't really
	// mark those entries as free, can we?

	// Flush the stream, ignoring any errors.
	if (fflush( stream))
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: fflush(%s) returned %d\n",
				__FUNCTION__, _stdio_stream_name( stream), error);
		#endif
		retval = EOF;
	}

	// Call the close function for this stream, keeping track of any errors
	if (stream->close)
	{
		if ( (error = stream->close( stream->cookie)) )
		{
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: %s->close() returned %d\n",
					__FUNCTION__, _stdio_stream_name( stream), error);
			#endif
			retval = EOF;
		}
	}

	// If tmpnam member of FILE structure is not empty, delete that temp file.
	if (*(stream->tmpnam))
	{
		_f_memcpy( tmpnam_local, stream->tmpnam, sizeof tmpnam_local);
		if ( (error = remove( tmpnam_local)) )
		{
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: error %d deleting temp file '%s' for %s\n", __FUNCTION__,
					tmpnam_local, _stdio_stream_name( stream), error);
			#endif
			retval = EOF;
		}
	}

	// call _stdio_FILE_free(), which will release any allocated buffers
	_stdio_FILE_free( stream);

	// memset the structure to all-zeros for next call to fopen
	_f_memset( stream, 0, sizeof *stream);

	return retval;
}


/*** BeginHeader setbuf */
// define masking macro
// masking macro triggers a warning, don't use for now
//#define setbuf( stream, buf)	\
//							setvbuf( stream, buf, (buf) ? _IOFBF : _IONBF, BUFSIZ)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
setbuf                                                           <stdio.h>

SYNTAX:	void setbuf( FILE far *stream, char far *buf)

DESCRIPTION:	Sets buffering for <stream> to fully-buffered, optionally
					using an external buffer.

					Except that it returns no value, the setbuf function is
					equivalent to the setvbuf function invoked as:

						setvbuf( stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ)

					The macro BUFSIZ is set in stdio.h and should not be modified.

					Since setvbuf() returns errors, it should be used instead of
					setbuf().

PARAMETER 1:	Stream to change buffering for.

PARAMETER 2:	If not set to NULL, <stream> will use this buffer instead of
					an internally-allocated one.  <buf> must be large enough to
					hold at least BUFSIZ bytes.

RETURN VALUE:	None

SEE ALSO:	setvbuf

END DESCRIPTION **********************************************************/
// parens around function name are necessary due to masking macro
_stdio_debug
void (setbuf)( FILE __far *stream, char __far *buf)
{
	setvbuf( stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}


/*** BeginHeader _stream_buf_alloc */
void _stream_buf_alloc( FILE __far *stream);
/*** EndHeader */
// allocate a buffer for use by stream if we don't already have one
_stdio_debug
void _stream_buf_alloc( FILE __far *stream)
{
	if (! stream->buffer_base)
	{
		if (! stream->buffer_alloc)
		{
			stream->buffer_alloc = malloc( BUFSIZ);
			stream->bufsize = BUFSIZ;
		}
		stream->buffer_base = stream->buffer_alloc;
	}
}


/*** BeginHeader setvbuf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
setvbuf                                                          <stdio.h>

SYNTAX:	int setvbuf( FILE far *stream, char far *buf, int mode, size_t bufsize)

DESCRIPTION:	This function can be used after <stream> has been opened, but
					before any other operation has been performed on the stream.
					It changes the buffering mode and, optionally, the buffer
					location for a given stream.

PARAMETER 1:	Stream to change buffering for.

PARAMETER 2:	If not set to NULL, <stream> will use this buffer instead of
					an internally-allocated one.

PARAMETER 3:	Determines how the stream will be buffered.  Set to one of the
					following modes:
							_IOFBF - fully buffered
							_IOLBF - line buffered
							_IONBF - unbuffered

					Line buffering only affects when output is flushed, it does not
					affect buffered reading.

PARAMETER 4:	The size of the buffer specified in parameter 2.  Ignored if
					<buf> is set to NULL.  Must be at least BUFSIZ bytes.

RETURN VALUE:	0 on success, non-zero on failure.
					-EBADF if <stream> is NULL or invalid
					-EINVAL if <mode> isn't valid
									or <buf> is not NULL and <bufsize> less than BUFSIZ
					-EPERM if unable to change buffering for this stream

SEE ALSO:	setbuf

END DESCRIPTION **********************************************************/
_stdio_debug
int setvbuf( FILE __far *stream, char __far *buf, int mode, size_t bufsize)
{
	int mode_bits;
	long offset;

	if (_STDIO_FILE_INVALID( stream))
	{
		return -EBADF;
	}

	if (stream->read_cur || stream->write_cur)
	{
		// can only change buffering if nothing read from or written to stream
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: too late to change buffering for %s\n",
				__FUNCTION__, _stdio_stream_name( stream));
		#endif
		return -EPERM;
	}

	if (stream->flags & _FILE_FLAG_BUF_NEVER)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s does not allow buffering\n", __FUNCTION__,
				_stdio_stream_name( stream));
		#endif
		return -EPERM;
	}

	// set the _FILE_FLAG_MASK bits depending on mode parameter
	switch (mode)
	{
		case _IOFBF:	mode_bits = _FILE_FLAG_BUF_FULL;		break;
		case _IOLBF:	mode_bits = _FILE_FLAG_BUF_LINE;		break;
		case _IONBF:	mode_bits = _FILE_FLAG_BUF_NONE;		break;
		default:
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: invalid mode (%d) selected for %s\n",
					__FUNCTION__, mode, _stdio_stream_name( stream));
			#endif
			return -EINVAL;
	}

	#ifdef STDIO_VERBOSE
		if (stream != stderr) fprintf( stderr,
			"%s: %s now %s\n", __FUNCTION__, _stdio_stream_name( stream),
			mode == _IOFBF ? "fully buffered" :
			mode == _IOLBF ? "line buffered" : "unbuffered");
	#endif

	if (buf)								// user is changing buffer assigned to stream
	{
		if (bufsize < BUFSIZ)
		{
			return -EINVAL;
		}
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s using %u-byte buffer @ 0x%06lX\n",
				__FUNCTION__, _stdio_stream_name( stream), bufsize, buf);
		#endif
		stream->bufsize = bufsize;
	}
	else if (mode == _IONBF)
	{
		stream->bufsize = 0;
	}
	else
	{
		stream->bufsize = BUFSIZ;
		buf = stream->buffer_alloc;
	}

	stream->buffer_base = buf;

	// Clear the old buffered bits (and autoflush) from stream->flags and set
	// to mode_bits.
	stream->flags =
		(stream->flags & ~(_FILE_FLAG_BUFFERED | _FILE_FLAG_AUTOFLUSH))
		| mode_bits;

	return 0;
}


/*** BeginHeader fseek */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fseek                                                            <stdio.h>

SYNTAX:	int fseek( FILE far *stream, long int offset, int whence)

DESCRIPTION:	Sets the file position indicator for a stream.

					A successful call to fseek() clears the end-of-file indicator
					for the stream and undoes any effects of ungetc() on the stream.

					Examples:
						// seek to start of file
						fseek( stream, 0, SEEK_SET);

						// seek to end of file
						fseek( stream, 0, SEEK_END);

						// seek to last 10 bytes of file
						fseek( stream, -10, SEEK_END);

						// skip over 512 bytes in file
						fseek( stream, 512, SEEK_CUR);

PARAMETER 1:	Stream to seek.

PARAMETER 2:	Number of bytes to move.  Positive values move toward the end
					of the file, negative values move toward the beginning of the
					file.  <offset> is relative to position indicated by <whence>.

PARAMETER 3:	One of the following macros:
							SEEK_SET - seek from beginning of file
							SEEK_CUR - seek from the current offset
							SEEK_END - seek from end of file

RETURN VALUE:	0 on success, non-zero on failure
					-EBADF if the stream is not valid
					-EPERM if the stream is not seekable
					-EINVAL if <whence> is not a valid macro

SEE ALSO:	ftell, rewind, fgetpos, fsetpos

END DESCRIPTION **********************************************************/
_stdio_debug
int fseek( FILE __far *stream, long int offset, int whence)
{
	int retval;

	if (_STDIO_FILE_INVALID( stream))
	{
		return -EBADF;
	}

	if (stream->seek == NULL)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s is not seekable (return -EPERM)\n", __FUNCTION__,
				_stdio_stream_name( stream));
		#endif
		return -EPERM;
	}

	if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END)
	{
		return -EINVAL;
	}

	if (stream->write_cur)
	{
		// make sure unwritten data is flushed before seeking
		fflush( stream);
	}

	#ifdef STDIO_VERBOSE
		if (stream != stderr) fprintf( stderr,
			"%s: %s seeking from %ld to %ld (%s)\n", __FUNCTION__,
			_stdio_stream_name( stream), stream->offset, offset,
			whence == SEEK_SET ? "SEEK_SET" :
			whence == SEEK_CUR ? "SEEK_CUR" :
			whence == SEEK_END ? "SEEK_END" : "bad whence");
	#endif
	retval = stream->seek( stream->cookie, &offset, whence);
	if (retval == 0)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: success; %s offset is now %ld\n", __FUNCTION__,
				_stdio_stream_name( stream), offset);
		#endif
		// on success, seek() sets offset to the absolute stream offset
		stream->offset = offset;

		// undoes any effects of ungetc()
		stream->unget_idx = 0;

		// nothing is buffered
		stream->write_cur = stream->write_end =
			stream->read_cur = stream->read_end = NULL;

		// Reset the _CAN_WRITE and _CAN_READ flags, based on _OPEN_READ
		// and _OPEN_WRITE.
		_STDIO_FILE_UPDATE_CAN_RW( stream);

		// Clear the end-of-file flag
		stream->flags &= ~_FILE_FLAG_EOF;
	}
	#ifdef STDIO_VERBOSE
	else
	{
		if (stream != stderr) fprintf( stderr,
			"%s: failure; seek(%s) returned %d\n", __FUNCTION__,
			_stdio_stream_name( stream), retval);
	}
	#endif

	return retval;
}


/*** BeginHeader ftell */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
ftell                                                            <stdio.h>

SYNTAX:	long int ftell( FILE far *stream)

DESCRIPTION:	Report the current file offset.

PARAMETER:	Stream to report position of.

RETURN VALUE:	Current file offset (>= 0) or -1 on failure.
					On failure, errno is set to:
						EBADF -- stream was invalid
						EOVERFLOW -- position overflowed (> LONG_MAX)

SEE ALSO:	fseek, rewind, fgetpos, fsetpos

END DESCRIPTION **********************************************************/
_stdio_debug
long int ftell( FILE __far *stream)
{
	long int retval;

	// DEVNOTE: What if I open a file, ungetc() a single char and then try to
	//				get the current offset?  Undefined?

	// check stream parameter and set errno to EBADF/EINVAL and return -1
	if (_STDIO_FILE_INVALID( stream))
	{
		errno = EBADF;
		return -1;
	}

	if (stream->offset < 0)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s offset overflowed (return -1)\n", __FUNCTION__,
				_stdio_stream_name( stream));
		#endif
		errno = EOVERFLOW;
		return -1;
	}

	if (stream->write_cur)
	{
		// Note that it's only safe to do .write_cur - .buffer_base when
		// .write_cur is not NULL (which also means that we're definitely
		// in "write" mode).
		retval = stream->offset										// data written
			+ (stream->write_cur - stream->buffer_base);		// + bytes buffered
	}
	else
	{
		// This calculation covers when we're in read or "idle" mode (stream
		// doesn't have any buffered read or write data).
		retval = stream->offset										// data read
			- (stream->read_end - stream->read_cur)			// - bytes in buffer
			- stream->unget_idx;										// - bytes pushed back
	}

	#ifdef STDIO_VERBOSE
		if (stream != stderr) fprintf( stderr,
			"%s: %s offset is %ld\n", __FUNCTION__,
			_stdio_stream_name( stream), retval);
	#endif

	return retval;
}


/*** BeginHeader rewind */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
rewind                                                           <stdio.h>

SYNTAX:	void rewind( FILE far *stream)

DESCRIPTION:	Sets the file position indicator for <stream> to the beginning
					of the file and clears the error indicator for the stream.

PARAMETER 1:	Stream to rewind.

RETURN VALUE:	None

SEE ALSO:	fseek, ftell, fgetpos, fsetpos

END DESCRIPTION **********************************************************/
_stdio_debug
void rewind( FILE __far *stream)
{
	#ifdef STDIO_VERBOSE
		if (stream != stderr) fprintf( stderr,
			"%s: rewinding %s\n", __FUNCTION__, _stdio_stream_name( stream));
		// verbose output from fseek() will indicate success or failure
	#endif
	if (! fseek( stream, 0, SEEK_SET))
	{
		// only clear error flag if there wasn't an error
		stream->flags &= ~_FILE_FLAG_ERROR;
	}
}


/*** BeginHeader fgetpos */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fgetpos                                                          <stdio.h>

SYNTAX:	int fgetpos( FILE far *stream, fpos_t *pos)

DESCRIPTION:	Store the current file position in a buffer passed by the caller.
					Since the contents of an fpos_t object are only used by
					fsetpos(), fgetpos() will return an error on unseekable streams.

PARAMETER 1:	Stream to get the position of.

PARAMETER 2:	Buffer for position storage.  This buffer contains unspecified
					information used by fsetpos() to restore the position to the
					current location.

RETURN VALUE:	0 on success, non-zero on failure.
					On failure, errno is set to one of the following:
						EPERM -- stream is not seekable
						EBADF -- stream is invalid
						EOVERFLOW -- position overflowed (> LONG_MAX)
					And -errno is returned.

SEE ALSO:	fseek, ftell, rewind, fsetpos

END DESCRIPTION **********************************************************/
// DEVNOTE: since fpos_t is only used by fsetpos(), and fsetpos can only seek
// on seekable streams, we'll return failure on a non-seekable stream.
_stdio_debug
int fgetpos( FILE __far *stream, fpos_t *pos)
{
	long offset;

	// ftell will set errno to EBADF or EOVERFLOW
	offset = ftell( stream);

	if (offset >= 0)
	{
		if (stream->seek)
		{
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: reporting %ld for %s (return 0)\n", __FUNCTION__,
					offset, _stdio_stream_name( stream));
			#endif
			pos->offset = offset;
			return 0;
		}
		else
		{
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: returning -EPERM for %s (not seekable)\n",
					__FUNCTION__, _stdio_stream_name( stream));
			#endif
			errno = EPERM;
		}
	}
	#ifdef STDIO_VERBOSE
	else
	{
		if (stream != stderr) fprintf( stderr,
			"%s: ftell(%s) failed (errno=%d)\n",
			__FUNCTION__, _stdio_stream_name( stream), errno);
	}
	#endif

	return -errno;
}


/*** BeginHeader fsetpos */
#define fsetpos( stream, pos)		fseek( stream, (pos)->offset, SEEK_SET)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fsetpos                                                          <stdio.h>

SYNTAX:	int fsetpos( FILE far *stream, const fpos_t *pos)

DESCRIPTION:	Sets the file position indicator for <stream> to <pos>, a value
					obtained from an earlier call to fgetpos().

					A successful call to fsetpos() clears the end-of-file indicator
					for <stream> and undoes any effects of the ungetc function on
					<stream>.

					After an fsetpos call, the next operation on an update stream
					may be either input or output.

PARAMETER 1:	Stream to set position on.

PARAMETER 2:	Position to set.  Must point to an fpos_t object set by fgetpos.

RETURN VALUE:	0 on success or non-zero on failure.
					-EBADF if the stream is not valid
					-EPERM if the stream is not seekable

SEE ALSO:	fseek, ftell, rewind, fgetpos

END DESCRIPTION **********************************************************/
_stdio_debug
int (fsetpos)( FILE __far *stream, const fpos_t *pos)
{
	return fseek( stream, pos->offset, SEEK_SET);
}


/*** BeginHeader _stream_wrapped_write */
size_t _stream_wrapped_write( FILE __far *stream, const void __far *base,
													size_t bytes);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_stream_wrapped_write                                            <stdio.c>

SYNTAX:	size_t _stream_wrapped_write( FILE far *stream, const void far *base,
																size_t bytes)

DESCRIPTION:	Helper function to wrap a stream's .write function pointer.
					Functions should NOT call stream->write directly!

					Verifies stream has a .write function pointer.

					Calls stream->write multiple times, if necessary, to write
					all bytes.

					Advances stream's offset by number of bytes written.

PARAMETER 1:	Stream to write to.

PARAMETER 2:	Buffer to write from.

PARAMETER 3:	Number of bytes to write.

RETURN VALUE:	Number of bytes written or 0 for EOF/error condition.

END DESCRIPTION **********************************************************/
_stdio_debug
size_t _stream_wrapped_write( FILE __far *stream, const void __far *base,
										         size_t bytes)
{
	size_t total = 0;
	int wrote;
	const char __far *p = base;

	if (! stream->write)
	{
		return 0;
	}

	while (bytes)
	{
		wrote = stream->write( stream->cookie, p, bytes);
		if (wrote < 0)
		{
			// write failed, abort with write incomplete
			// DEVNOTE: should this set the error flag?
			break;
		}
		p += wrote;
		bytes -= wrote;
		total += wrote;
	}

	stream->offset += total;
	return total;
}


/*** BeginHeader _stream_wrapped_read */
size_t _stream_wrapped_read( FILE __far *stream, void __far *buffer, size_t bytes);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_stream_wrapped_read                                             <stdio.c>

SYNTAX:	size_t _stream_wrapped_read( FILE far *stream,
																void far *buffer, size_t bytes)

DESCRIPTION:	Helper function to wrap a stream's .read function pointer.
					Functions should NOT call stream->read directly!

					Verifies stream has a .read function pointer.

					Calls stream->read multiple times, if necessary, to read
					all bytes.

					Sets EOF and error flags appropriately.

					Advances stream's offset by number of bytes read.

PARAMETER 1:	Stream to read from.

PARAMETER 2:	Buffer to read into.

PARAMETER 3:	Number of bytes to read.

RETURN VALUE:	Number of bytes read or 0 for EOF/error condition.

END DESCRIPTION **********************************************************/
_stdio_debug
size_t _stream_wrapped_read( FILE __far *stream, void __far *buffer, size_t bytes)
{
	int retval;

	// assumes caller has already verified that stream is valid/readable
	if (! stream->read)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: can't read from %s; no .read()\n", __FUNCTION__,
				_stdio_stream_name( stream));
		#endif
		retval = 0;		// no read function is the same as always EOF
	}
	else
	{
		retval = stream->read( stream->cookie, buffer, bytes);

		if (retval > 0)
		{
			// successful read, no longer at EOF
			stream->flags &= ~_FILE_FLAG_EOF;

			// advance offset by number of bytes read
			stream->offset += retval;
			return retval;
		}
	}

	if (retval == 0)					// just EOF
	{
		stream->flags |= _FILE_FLAG_EOF;
	}
	else									// actual error
	{
		stream->flags |= _FILE_FLAG_ERROR;
	}

	return 0;
}

/*** BeginHeader _stream_ready */
#define _stream_can_read( stream)		_stream_ready( stream, 0)
#define _stream_can_write( stream)		_stream_ready( stream, 1)
int _stream_ready( FILE __far *stream, int write);
/*** EndHeader */
// Is it OK to read/write stream?  Also handles allocating buffer if not
// already allocated.
_stdio_debug
int _stream_ready( FILE __far *stream, int write)
{
	int flags = write ? (_FILE_FLAG_USED | _FILE_FLAG_CAN_WRITE)
							: (_FILE_FLAG_USED | _FILE_FLAG_CAN_READ);

	if ((stream->flags & flags) != flags)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s missing flags (0x%04x != 0x%04x)\n", __FUNCTION__,
				_stdio_stream_name( stream), stream->flags & flags, flags);
		#endif
		return 0;
	}

	// if in append mode, seek to end of file before prepping write
	if (write && stream->flags & _FILE_FLAG_OPEN_APPEND)
	{
		if (! stream->write_cur)			// not already writing
		{
			fseek( stream, 0, SEEK_END);
		}
	}

	// If stream is buffered, make sure a buffer has been assigned or allocated
	// and then set the .write_cur/.write_end or .read_cur/.read_end members.
	if (stream->flags & _FILE_FLAG_BUFFERED)
	{
		if (stream->write_cur)				// already writing
		{
			assert( write == 1);
		}
		else if (stream->read_cur)			// already reading
		{
			assert( write == 0);
		}
		else
		{
			if (! stream->buffer_base)
			{
				#ifdef STDIO_VERBOSE
					if (stream != stderr) fprintf( stderr,
						"%s: %s allocating stream buffer\n", __FUNCTION__,
						_stdio_stream_name( stream));
				#endif
				_stream_buf_alloc( stream);
			}

			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: %s entering %s mode\n", __FUNCTION__,
					_stdio_stream_name( stream), write ? "write" : "read");
			#endif
			if (write)
			{
				// entering write mode, disallow reading
				stream->flags &= ~_FILE_FLAG_CAN_READ;

				// set up buffer for writing
				stream->write_cur = stream->buffer_base;
				stream->write_end = stream->buffer_base + stream->bufsize;
			}
			else
			{
				// entering read mode, disallow writing
				stream->flags &= ~_FILE_FLAG_CAN_WRITE;

				// set up buffer for reading
				stream->read_end = stream->read_cur = stream->buffer_base;
			}
		}
	}

	return -1;
}

/*** BeginHeader fgetc */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fgetc                                                            <stdio.h>

SYNTAX:	int fgetc( FILE far *stream)
			int getc( FILE far *stream)
			int getchar( void)

DESCRIPTION:	These functions are used to read a character from a stream
					and advance the associated file position indicator.

						fgetc - read a character from a stream.
						getc - a faster, macro version of fgetc().
						getchar - equivalent to passing stdin to getc().

					Note that getc() may evaluate <stream> more than once, so the
					argument should never be an expression with side effects.

PARAMETER <stream>:	Stream to read from.

RETURN VALUE:	The next character from <stream> (if present) as an unsigned
					char, converted to an int.

					If the stream is at end-of-file, the end-of-file indicator is
					set and fgetc() returns EOF.  If a read error occurs, the error
					indicator for the stream is set and fgetc() returns EOF.

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
_stdio_debug
int fgetc( FILE __far *stream)
{
	char ch;

	if (_STDIO_FILE_INVALID( stream))
	{
		return EOF;
	}

	// something in unget buffer
	if (stream->unget_idx)
	{
		return stream->unget_buf[--stream->unget_idx];
	}

	// something in stream buffer
	if (stream->read_cur < stream->read_end)
	{
		return *stream->read_cur++;
	}

	// fall back on fread() for the uncommon cases
	if (fread( &ch, 1, 1, stream))
	{
		return ch;
	}
	return EOF;
}


/*** BeginHeader getc */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
getc                                                             <stdio.h>

SYNTAX:	int getc( FILE far *stream)

		See function help for fgetc() for a description of this function.

END DESCRIPTION **********************************************************/
/*
	Code generated by the macro was horribly inefficient (238 bytes),
	so this is an assembly version of fgetc() (77 bytes).

	DEVNOTE: If we can improve the backend optimizer, it would be better
				to switch back to the macro.

				If the assembly is overkill, getc() can be defined as a macro
				to fgetc().

	Assumes stream is not NULL.

	Macro version of getc():

// Note that since the unget buffer is unused, we can borrow it for the fread
#define getc(s)	\
	(s && (s)->flags & _FILE_FLAG_CAN_READ ?								\
		((s)->unget_idx ? (s)->unget_buf[--(s)->unget_idx] :				\
		 (s)->read_cur < (s)->read_end ? *(s)->read_cur++ :				\
		 fread( (s)->unget_buf, 1, 1, s) ? (s)->unget_buf[0] : EOF)		\
	: EOF)

*/
#asm __xmem
getc::
		; px already contains address of stream

		; make sure _FILE_FLAG_USED and _FILE_FLAG_CAN_READ are set
		ld		hl, (px+[FILE]+flags)
		ld		de, _FILE_FLAG_USED | _FILE_FLAG_CAN_READ
		and	hl, de
		cp		hl, de
		jr		neq, .exitEOF

		; check for bytes in unget buffer
		ld		py, px+[FILE]+unget_buf
		ld		hl, (px+[FILE]+unget_idx)
		test	hl
		jr		z, .no_unget						; no pushed-back character available

		jp		m, .no_unget						; don't use a negative unget index!

		dec	hl
		ld		(px+[FILE]+unget_idx), hl
		ld		a, (py+hl)							; char = unget_buf[--unget_idx]
		jr		.exitOK

	.no_unget:
		ld		bcde, (px+[FILE]+read_cur)
		ld		jkhl, (px+[FILE]+read_end)
		cp		jkhl, bcde
		jp		eq, .use_fread
		ld		pz, bcde
		ld		a, (pz)								; char = *read_cur
		ld		pz, pz + 1
		ld		(px+[FILE]+read_cur), pz		; read_cur++
		jr		.exitOK

	.use_fread:
		push	px				; push stream
		push	0x0001		; push nmemb
		push	0x0001		; push membsize
		ld		px, py		; push ptr (unget_buf)
		push	px
		lcall	fread
		pop	py				; restore address of character read by fread
		add	sp, 8			; pop remaining parameters from stack

		ld		a, (py)								; char = unget_buf[0]
		test	hl
		jr		z, .exitEOF							; if 0 bytes read, return EOF

		; no bytes read, fall through to OK with return value in A
	.exitOK:
		clr	hl
		ld		l, a
		lret

	.exitEOF:
		ld		hl, -1
		lret
#endasm


/*** BeginHeader getchar */
#define getchar()		getc( stdin)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
getchar                                                          <stdio.h>

SYNTAX:	int getchar( void)

		See function help for fgetc() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_debug
int (getchar)( void)
{
	return getc( stdin);
}


/*** BeginHeader ungetc */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
ungetc                                                           <stdio.h>

SYNTAX:	int ungetc( int c, FILE far *stream)

DESCRIPTION:	Pushes <c> (converted to an unsigned char) back onto the
					input stream <stream>.  The pushed-back characters are
					returned by subsquent reads on that stream, in the reverse
					order of their pushing.

					Calling fseek(), fsetpos() or rewind() on <stream> discards any
					characters pushed with ungetc().

					One character of pushback is guaranteed.  If ungetc() is called
					too many times on a stream without an intervening read or file
					positioning operation (which clears the pushback buffer), the
					operation may fail.

					A successful call to ungetc() clears the end-of-file indicator
					for the stream,  The value of the file position indicator is
					decremented for each successful call to ungetc().  After reading
					or discarding pushed characters, the position indicator will
					be the same as it was before the characters were pushed.

PARAMETER 1:	Character to push back onto the input stream.  If <c> is equal
					to the macro EOF, the operation fails and the input stream
					is unchanged.

PARAMETER 2:	Stream to push the character into.

RETURN VALUE:	The character pushed back on success, EOF on failure.

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
_stdio_debug
int ungetc( int c, FILE __far *stream)
{
	// check for c == EOF (or any invalid value) and error out
	if (c & 0xFF00)
	{
		return EOF;
	}

	// Should fail if stream is in write mode
	if (! _stream_can_read( stream))
	{
		return EOF;
	}

	// if there is room in the unget buffer, stuff the byte
	if (stream->unget_idx == _STDIO_UNGET_BYTES)
	{
		// unget buffer is full
		return EOF;
	}

	stream->unget_buf[stream->unget_idx++] = c;

	// on success, clear the EOF flag
	stream->flags &= ~_FILE_FLAG_EOF;

	return c;
}


/*** BeginHeader fgets */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fgets                                                            <stdio.h>

SYNTAX:	char far *fgets( char far *s, int n, FILE far *stream)

DESCRIPTION:	Reads no more than (<n>-1) characters from <stream> into the
					character buffer <s>.  No additional characters are read after
					a newline character (which is retained) or end-of-file.

					A null character is written immediately after the last character
					read into the array.

				>>	Note that fgets() includes the newline but gets() does not.

PARAMETER 1:	Buffer to store characters read from <stream>.  Must be able to
					hold <n> characters (including null terminator).

PARAMETER 2:	Maximum number of characters to write to <s>.

PARAMETER 3:	Stream to read from.

RETURN VALUE:	Returns <s> if successful, NULL on failure.
					If end-of-file is encountered before any characters have been
					read, the contents of <s> remain unchanged.

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
_stdio_debug
char __far *fgets( char __far *s, int n, FILE __far *stream)
{
	char __far *p;		// position of next byte to write
	int c;				// last character read

	// Save room for the trailing newline and null terminator.
	if (_STDIO_FILE_INVALID( stream) || n < 2)
	{
		return NULL;
	}

	// make sure we're allowed to read
	if (! _stream_can_read( stream))
	{
		stream->flags |= _FILE_FLAG_EOF;
		return NULL;
	}

	p = s;
	while (--n)					// predecrement to leave room for null terminator
	{
		c = getc( stream);
		if (c == EOF)
		{
			if (p == s)
			{
				return NULL;
			}
			break;
		}
		*p++ = c;
		if (c == '\n')
		{
			break;
		}
	}
	*p = '\0';
	return s;
}


/*** BeginHeader gets */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
gets                                                             <stdio.h>

SYNTAX:	char *gets( char *s)

DESCRIPTION:	Reads characters from stdin (the STDIO Window in Dynamic C,
					or a serial port if STDIO was redirected) and stores them in
					the character buffer <s>, until a newline character is read.

					The newline character is discarded and a null terminator is
					written to the buffer before returning.

					Echos characters read to stdout and processes backspace
					characters by deleting the last character entered.

					Use fgets() instead of gets() to avoid overflowing the buffer.
				>>	Note that fgets() includes the newline but gets() does not.

				>>	Echos input to stdout.  If you don't want input echoed, use
					fgets() instead.

					For backward compatibility, gets() only works with near pointers.
					Use fgets() instead of gets() to read into a far buffer.

PARAMETER 1:	Buffer to hold characters read from stdin.

RETURN VALUE:	Returns <s>, the buffer passed as parameter 1.  Blocks until
					a newline is received.

					Returns NULL on error (for example, if stdin has been closed or
					redirected to a file that reaches EOF).

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
_stdio_debug
char *gets( char *s)
{
	char *p;
	int c;

	p = s;
	while (1)
	{
		c = getchar();
		switch (c)
		{
			case '\b':
				if (s != p)
				{
					fputs( "\b \b", stdout);
					p--;
				}
				break;

			case '\r':
				fputc( '\n', stdout);
				fflush( stdout);
			case EOF:
				*p = 0;
				return s;

			default:
				*p++= c;
				fputc( c, stdout);
				break;
		}
		fflush( stdout);
	}
}

// DEVNOTE: bring in getsn as well?

/*** BeginHeader fread */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fread                                                            <stdio.h>

SYNTAX:	size_t fread( void far *ptr, size_t membsize, size_t nmemb,
																				FILE far *stream)

DESCRIPTION:	Reads up to <nmemb> elements of <membsize> bytes from <stream>
					and stores them in the buffer <ptr>.  Advances the file position
					indicator for the number of bytes read.

					If an error occurs, the file position indicator is indeterminate.
					If a partial element is read, its value is indeterminate.

PARAMETER 1:	Buffer to store data from <stream>.  Must be at least
					(<membsize>*<nmemb>) bytes large.

PARAMETER 2:	Size of each member (record) to read from the stream.

PARAMETER 3:	Number of members (records) to read.

PARAMETER 4:	Stream to read from.

RETURN VALUE:	Returns the number of elements successfully read, which may be
					less than <nmemb> if a read error or end-of-file is encountered.

					If <nmemb> or <membsize> are zero, the contents of <ptr> and
					the stream remain unchanged and fread() returns zero.

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
// tag as xmem since getc() lcalls it
_stdio_debug __xmem
size_t fread( void __far *ptr, size_t membsize, size_t nmemb, FILE __far *stream)
{
	size_t	bytesleft, buffered, copy, read;
	char __far *dest = ptr;

	if (_STDIO_FILE_INVALID( stream))
	{
		return 0;
	}

	bytesleft = membsize * nmemb;
	if (! bytesleft)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s trying to read %u recs of %u bytes (return 0)\n",
				__FUNCTION__, _stdio_stream_name( stream), nmemb, membsize);
		#endif
		return 0;
	}

	if (! _stream_can_read( stream))
	{
		stream->flags |= _FILE_FLAG_EOF;
		return 0;
	}

	/*
		fread() methodology:

		First, take characters from the unget buffer.
		Repeat the following until the request is satisfied:
			Take any characters still in the stream's buffer.
			If the request size is larger than the stream's buffer size,
				bypass the stream buffer and read directly to destination.
			Fill the stream's buffer.

	*/

	// check for characters in the unget buffer
	if (stream->unget_idx)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s pulling %u chars from unget buffer\n", __FUNCTION__,
				_stdio_stream_name( stream), stream->unget_idx);
		#endif
		do
		{
			*dest = stream->unget_buf[--stream->unget_idx];
			++dest;
			--bytesleft;
		} while (stream->unget_idx && bytesleft);
	}

	read = 1;
	while (bytesleft && read)
	{
		// copy buffered bytes first
		buffered = (size_t) (stream->read_end - stream->read_cur);
		if (buffered)
		{
			if (buffered > bytesleft)
			{
				buffered = bytesleft;
			}
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: %s pulling %u chars from stream buffer\n", __FUNCTION__,
					_stdio_stream_name( stream), buffered);
			#endif
			_f_memcpy( dest, stream->read_cur, buffered);
			stream->read_cur += buffered;
			bytesleft -= buffered;
			dest += buffered;
			if (! bytesleft)
			{
				break;
			}
		}

		// shouldn't be any buffered bytes at this point
		assert( stream->read_end == stream->read_cur);
		stream->read_cur = stream->read_end = stream->buffer_base;

		// copy directly to destination instead of through buffer
		while (bytesleft && (bytesleft >= stream->bufsize) && read)
		{
			copy = bytesleft;
			if (copy > BUFSIZ)
			{
				copy = BUFSIZ;
			}
			read = _stream_wrapped_read( stream, dest, copy);
			if (read)
			{
				bytesleft -= read;
				dest += read;
				#ifdef STDIO_VERBOSE
					if (stream != stderr) fprintf( stderr,
						"%s: %s read %u chars (of %u) unbuffered\n", __FUNCTION__,
						_stdio_stream_name( stream), read, copy);
				#endif
			}
		}

		// fill stream buffer to satisfy remaining bytes
		if (bytesleft && read)
		{
			// Can only reach this point if we're buffered (bufsize > 0).
			assert (stream->bufsize > 0);

			// read into buffer, and allow top of loop to copy portion to dest
			read = _stream_wrapped_read( stream,
												stream->buffer_base, stream->bufsize);
			if (read)
			{
				stream->read_end += read;
				#ifdef STDIO_VERBOSE
					if (stream != stderr) fprintf( stderr,
						"%s: %s buffered %u chars\n", __FUNCTION__,
						_stdio_stream_name( stream), read);
				#endif
			}
		}
	}

	if (dest == ptr)			// nothing read
	{
		return 0;
	}

	if (bytesleft)
	{
		// partial read, calculate number of members read
		return (membsize * nmemb - bytesleft) / membsize;
	}

	// successfully read all members requested
	return nmemb;
}


/*** BeginHeader fputc */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fputc                                                            <stdio.h>

SYNTAX:	int fputc( int c, FILE far *stream)
			int putc( int c, FILE far *stream)
			int putchar( int c)

DESCRIPTION:	Writes character <c> (converted to an unsigned char) to
					<stream>, and advances the file position indicator.  If the
					stream doesn't support positioning requests, or the stream was
					opened in append mode, the character is appended to the output
					stream.

						fputc - write <c> to <stream>.
						putc - a faster, macro version of fputc().
						putchar - equivalent to passing stdout to putc().

					Note that putc() may evaluate <stream> more than once, so the
					argument should never be an expression with side effects.

PARAMETER <c>:			Character to write.

PARAMETER <stream>:	Stream to write <c> to.

RETURN VALUE:	Returns the character written.  Returns EOF and sets the error
					indicator for <stream> if a write error occurs.

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
_stdio_debug
int fputc( int c, FILE __far *stream)
{
	int mode;

	if (_STDIO_FILE_INVALID( stream))
	{
		return EOF;
	}

	// If auto-flush flag is set, go straight to fwrite.
	if (!(stream->flags & _FILE_FLAG_AUTOFLUSH)
		&& stream->write_cur < stream->write_end)
	{
		*stream->write_cur++ = c;
		if ((c == '\n') && (stream->flags & _FILE_FLAG_BUF_LINE))
		{
			if (fflush( stream))
			{
				return EOF;
			}
		}
	}
	else if (! fwrite( &c, 1, 1, stream))
	{
		stream->flags |= _FILE_FLAG_ERROR;
		return EOF;
	}

	return c;
}


/*** BeginHeader putc */
// define masking macro -- !!! currently conflicts with putc vars in stdio.lib
//#define putc( c, stream)	fputc( c, stream)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
putc                                                             <stdio.h>

SYNTAX:	int putc( int c, FILE far *stream)

		See function help for fputc() for a description of this function.

END DESCRIPTION **********************************************************/
// parens around function name are necessary due to masking macro
_stdio_debug
int (putc)( int c, FILE __far *stream)
{
	return fputc( c, stream);
}


/*** BeginHeader putchar */
// define masking macro
#define putchar( c)		fputc( c, stdout)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
putchar                                                          <stdio.h>

SYNTAX:	int putchar( int c)

		See function help for fputc() for a description of this function.

END DESCRIPTION **********************************************************/
// parens around function name are necessary due to masking macro
_stdio_debug
int (putchar)( int c)
{
	return putc( c, stdout);
}


/*** BeginHeader fputs */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fputs                                                            <stdio.h>

SYNTAX:	int fputs( const char far *s, FILE far *stream)
			int puts( const char far *s)

DESCRIPTION:	Writes a string to a stream.  Does not write the null terminator.

						fputs - writes <s> to <stream>
						puts - writes <s> and a newline to stdout

					If the macros __ANSI_STRICT__ or __ANSI_PUTS__ are defined,
					puts() will append a newline to the string.  If not defined,
					puts() follows legacy Dynamic C behavior of not appending a
					newline.

PARAMETER <s>:			Null-terminated string to write.

PARAMETER <stream>:	Stream to write to.

RETURN VALUE:	EOF if a write error occurs, otherwise a non-negative value.

NOTE: For backward compatability with earlier versions of Dynamic C, puts()
		returns 1 on success.

END DESCRIPTION **********************************************************/
_stdio_debug
int fputs( const char __far *s, FILE __far *stream)
{
	int wrote;
	int len;

	len = strlen( s);
	wrote = fwrite( s, 1, len, stream);

	return (wrote == len) ? 0 : EOF;
}


/*** BeginHeader puts */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
puts                                                             <stdio.h>

SYNTAX:	int puts( const char far *s)

		See function help for fputs() for a description of this function.

END DESCRIPTION **********************************************************/
// DEVNOTE: Strict ANSI includes a newline at the end of the string.
//				Legacy Dyanmic C mode just sends the string.
_stdio_debug
int puts( const char __far *s)
{
	int retval;

#if (defined __ANSI_STRICT__ || defined __ANSI_PUTS__)
	if (fputs( s, stdout) < 0 || fputc( '\n', stdout) < 0)
#else
	if (fputs( s, stdout) < 0)
#endif
	{
		retval = EOF;
	}
	else
	{
		retval = 1;
	}

	if (stdout->flags & _FILE_FLAG_AUTOFLUSH)
	{
		fflush( stdout);
	}

	return retval;
}


/*** BeginHeader fwrite */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fwrite                                                           <stdio.h>

SYNTAX:	size_t fwrite( const void far *ptr, size_t membsize, size_t nmemb,
																					FILE far *stream)

DESCRIPTION:	Writes up to <nmemb> elements of <membsize> bytes to <stream>
					from the buffer <ptr>.  The file position indicator is advanced
					by the number of characters successfully written.

					If an error occurs, the file position indicator is indeterminate.

					To know for certain how much data was written, set <membsize>
					to 1 or use fseek() and ftell() on errors to determine how
					many bytes have been written to the stream.

PARAMETER 1:	Source of data to write to <stream>.

PARAMETER 2:	Size of each member (record) to write to the stream.

PARAMETER 3:	Number of members (records) to write.

PARAMETER 4:	Stream to write to.

RETURN VALUE:	The number of elements successfully written, which will be
					less than <nmemb> only if a write error is encountered.

SEE ALSO:	fgetc, getc, getchar, ungetc, fgets, gets, fread,
				fputc, putc, putchar, fputs, puts, fwrite

END DESCRIPTION **********************************************************/
_stdio_debug
size_t fwrite( const void __far *ptr, size_t membsize, size_t nmemb,
																				FILE __far *stream)
{
	// if _FILE_FLAG_APPEND is set, seek to end of file before writing

	// make sure _CAN_WRITE is set, or file is at EOF

	size_t	bytesleft, send, sent, copy, bufspace;
	const char __far *src = ptr;
	char __far *newline;
	int bufmode, error;

	if (_STDIO_FILE_INVALID( stream))
	{
		return 0;
	}

	bytesleft = membsize * nmemb;
	if (! bytesleft)
	{
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: %s trying to write %u recs of %u bytes (return 0)\n",
				__FUNCTION__, _stdio_stream_name( stream), nmemb, membsize);
		#endif
		return 0;
	}

	if (! _stream_can_write( stream))
	{
		// not allowed to write to the stream
		stream->flags |= _FILE_FLAG_ERROR;
		return 0;
	}

	bufmode = (stream->flags & _FILE_FLAG_BUFFERED);

	// If newline is set, we're in line buffered mode and there is a newline
	// character in the data to send.
	newline = (bufmode == _FILE_FLAG_BUF_LINE) ?
													_f_memchr(src, '\n', bytesleft) : NULL;


	// shortcut -- if the buffer is empty, and we have:
	//		a full line (in line buffered mode)
	//		more data than can fit (always the case for nonbuffered)
	// just send it directly
	//	otherwise, add the data to the current buffer, and if full flush it

	error = 0;
	if (bufmode)
	{
		// note that _stream_can_write() set up .write_cur and .write_end for us
		assert( stream->write_end == stream->buffer_base + stream->bufsize);
		assert( stream->write_cur != NULL);

		if (stream->write_cur != stream->buffer_base)
		{
			// we already have buffered data -- add to it before sending

			// see if there is room in the buffer for our data
			bufspace = (size_t) (stream->write_end - stream->write_cur);
			if (bufspace)
			{
				copy = newline ? (size_t)(newline - src + 1) : bytesleft;
				if (bufspace < copy)
				{
					copy = bufspace;
				}
				#ifdef STDIO_VERBOSE
					if (stream != stderr) fprintf( stderr,
						"%s: %s buffering %u bytes\n",
						__FUNCTION__, _stdio_stream_name( stream), copy);
				#endif
				_f_memcpy( stream->write_cur, src, copy);
				src += copy;
				stream->write_cur += copy;
				bufspace -= copy;
				bytesleft -= copy;
			}
			// if we have a complete line to send, or buffer is full, send it
			if (newline || ! bufspace)
			{
				#ifdef STDIO_VERBOSE
					if (stream != stderr) fprintf( stderr,
						"%s: %s flushing (%s)\n",
						__FUNCTION__, _stdio_stream_name( stream),
						newline ? "buffered line" : "full buffer");
				#endif
				error = fflush( stream);
				// if error trying to flush stream, unsent bytes still buffered
			}
		}
	}

	// Still have to send <bytesleft> bytes from <src>, but now there isn't
	// anything in the stream's buffer to deal with.
	while (bytesleft && !error)
	{
		assert( stream->write_cur == stream->buffer_base);

		if (newline)
		{
			// find next line break if line buffered
			newline = _f_memchr(src, '\n', bytesleft);
		}
		send = newline ? (size_t)(newline - src + 1) : bytesleft;

		if (!bufmode || newline || (send >= stream->bufsize))
		{
			// if unbuffered || sending a line || more data than fits in buffer
			// send this data directly, bypassing the stream's buffer
			sent = _stream_wrapped_write( stream, src, send);
			src += sent;
			bytesleft -= sent;
			error = (sent < send);
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: %s wrote %u (of %u) bytes (unbuffered)\n",
					__FUNCTION__, _stdio_stream_name( stream), sent, send);
			#endif
		}
		else
		{
			assert( bytesleft == send);

			// buffer the remaining bytes, to be sent later
			#ifdef STDIO_VERBOSE
				if (stream != stderr) fprintf( stderr,
					"%s: %s buffering remaining %u bytes\n",
					__FUNCTION__, _stdio_stream_name( stream), send);
			#endif
			_f_memcpy( stream->write_cur, src, send);
			stream->write_cur += send;
			src += send;
			bytesleft = 0;
		}
	}


	if (src == ptr)			// nothing written
	{
		return 0;
	}

	if (bytesleft)
	{
		// partial write, calculate number of members written
		return (membsize * nmemb - bytesleft) / membsize;
	}

	// successfully wrote all members requested
	return nmemb;
}


/*** BeginHeader clearerr */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
clearerr                                                         <stdio.h>

SYNTAX:	void clearerr( FILE far *stream)

DESCRIPTION:	Clears the end-of-file and error indicators for <stream>.

PARAMETER 1:	Stream to clear errors on.

RETURN VALUE:	None.

SEE ALSO:	feof, ferror, perror

END DESCRIPTION **********************************************************/
_stdio_debug
void clearerr( FILE __far *stream)
{
	if (! _STDIO_FILE_INVALID( stream))
	{
		stream->flags &= ~(_FILE_FLAG_ERROR | _FILE_FLAG_EOF);
		#ifdef STDIO_VERBOSE
			if (stream != stderr) fprintf( stderr,
				"%s: cleared errors for %s; flags now 0x%04x\n",
				__FUNCTION__, _stdio_stream_name( stream), stream->flags);
		#endif
	}
}


/*** BeginHeader feof */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
feof                                                             <stdio.h>

SYNTAX:	int feof( FILE far *stream)

DESCRIPTION:	Tests the end-of-file indicator for <stream>.

PARAMETER 1:	Stream to test.

RETURN VALUE:	0 if end-of-file indicator is not set, non-zero if it is.

SEE ALSO:	ferror, clearerr, perror

END DESCRIPTION **********************************************************/
_stdio_debug
int feof( FILE __far *stream)
{
	if (_STDIO_FILE_INVALID( stream))
	{
		return -EBADF;
	}

	#ifdef STDIO_VERBOSE
		if (stream != stderr) fprintf( stderr,
			 "%s: %s %s %s set\n", __FUNCTION__, _stdio_stream_name( stream),
			"EOF", stream->flags & _FILE_FLAG_EOF ? "is" : "is not", "EOF");
	#endif

	// only EOF if flag is set AND there isn't any buffered or ungetc data
	return (stream->flags & _FILE_FLAG_EOF) && (! stream->unget_idx)
		&& (stream->read_cur == stream->read_end);
}


/*** BeginHeader ferror */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
ferror                                                           <stdio.h>

SYNTAX:	int ferror( FILE far *stream)

DESCRIPTION:	Tests the error indicator for <stream>.

PARAMETER 1:	Stream to test.

RETURN VALUE:	0 if error indicator is not set, non-zero if it is.

SEE ALSO:	feof, clearerr, perror

END DESCRIPTION **********************************************************/
_stdio_debug
int ferror( FILE __far *stream)
{
	if (_STDIO_FILE_INVALID( stream))
	{
		return -EBADF;
	}

	#ifdef STDIO_VERBOSE
		if (stream != stderr) fprintf( stderr,
			"%s: %s %s %s set\n", __FUNCTION__, _stdio_stream_name( stream),
			"error flag", stream->flags & _FILE_FLAG_ERROR ? "is" : "is not");
	#endif

	// Use "!= 0" to force return to either 0 (not set) or 1 (set).
	return (stream->flags & _FILE_FLAG_ERROR) != 0;
}


/*** BeginHeader perror */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
perror                                                           <stdio.h>

SYNTAX:	void perror( const char far *s)

DESCRIPTION:	Uses the variable errno (defined in errno.h) and parameter
					<s> to send an error message, followed by a newline character,
					to stderr.  The error messages are the same as those returned
					by calling strerror( errno).

PARAMETER 1:	String to use as a prefix (followed by a colon and a space) to
					the error message.  Ignored if NULL or empty.

RETURN VALUE:	None.

SEE ALSO:	feof, ferror, clearerr, strerror

END DESCRIPTION **********************************************************/
_stdio_debug
void perror( const char __far *s)
{
	char buffer[40];

	if (s && *s)
	{
		fputs( s, stderr);
		fputs( ": ", stderr);
	}
	fputs( _error_message( errno, buffer), stderr);
	fputc( '\n', stderr);
}




