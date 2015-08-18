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
/******
	Samples\SlavePort\slave_demo.c

	A sample Rabbit slave, runs a serial port

******/
#class auto

#use "slave_port.lib"
#use "sp_stream.lib"

#define STREAM_BUFFER_SIZE 31

void main()
{
	char buffer[10];
	int bytes_read;
	SPStream stream;
	char stream_inbuf[STREAM_BUFFER_SIZE + 9];
	char stream_outbuf[STREAM_BUFFER_SIZE + 9];
	SPStream *stream_ptr;
	
	//setup buffers
	cbuf_init(stream_inbuf, STREAM_BUFFER_SIZE);
	stream.inbuf = stream_inbuf;
	cbuf_init(stream_outbuf, STREAM_BUFFER_SIZE);
	stream.outbuf = stream_outbuf;
	stream_ptr = &stream;
	SPinit(1);
	SPsetHandler(0x42, SPShandler, stream_ptr);
	while(1)
	{
		//SPtick();
		//printf("Going %d\n", slave_test_char);
		bytes_read = SPSread(stream_ptr, buffer, 10, 10);
		if(bytes_read)
		{
			SPSwrite(stream_ptr, buffer, bytes_read);
		}						
	}
}	
