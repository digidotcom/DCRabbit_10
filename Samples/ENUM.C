/**********************************************************************
    Samples\enum.c
    Z-World, 2002

    This program demonstrates the use of the enum keyword in Dynamic
    C.  It also demonstrates ANSI escape sequences with the STDIO
    window so that various effects can be achieved.

    The enum construct allows the simple construction of identifiers
    that correspond to specific numbers.  These identifiers share the
    scope of the enum definition.  Note that enums are always of type
    int.  The initial identifier's value is 0 by default.  Each
    successive identifier's value is 1 more than the previous
    identifier by default.  At any time, an explicit constant value
    can used for an identifier.  There is no need for each value to
    be unique.

    Note also that enum identifiers are not lvalues--hence, you can
    not change them once they are defined.
    
**********************************************************************/
#class auto


// This macro will output an ANSI escape sequence for the given
// attribute.
#define stdio_set(attrib) printf("\x1b[%dm", attrib)

// This enum definition specifies the attributes that we can
// set on the STDIO window.
enum StdioAttrib {
	STDIO_RESET			= 0,	// 0 is the default starting point, so this isn't
									// strictly necessary
	STDIO_BOLD			= 1,	// enums increment by default, so again, this
									// isn't strictly necessary
	STDIO_REVERSE		= 7,	// We need a jump in the sequence, so we can
									// explicitly set it
	STDIO_CONCEALED	= 8,

	STDIO_FG_BLACK		= 30,
	STDIO_FG_RED,				// Here, we let the natural enum incrementing
									// take over
	STDIO_FG_GREEN,
	STDIO_FG_YELLOW,
	STDIO_FG_BLUE,
	STDIO_FG_MAGENTA,
	STDIO_FG_CYAN,
	STDIO_FG_WHITE,

	STDIO_BG_BLACK		= 40,	// Another jump in the sequence
	STDIO_BG_RED,				// And more enum incrementing
	STDIO_BG_GREEN,
	STDIO_BG_YELLOW,
	STDIO_BG_BLUE,
	STDIO_BG_MAGENTA,
	STDIO_BG_CYAN,
	STDIO_BG_WHITE
};

void main(void)
{
	// We want to cycle the colors, but we don't want all of them.
	// Here we select the colors, being careful to follow the general
	// Roy G. Biv progression so that the transitions are smooth.
	static const enum StdioAttrib colors[] = {
		STDIO_FG_RED,
		STDIO_FG_YELLOW,
		STDIO_FG_GREEN,
		STDIO_FG_CYAN,
		STDIO_FG_BLUE,
		STDIO_FG_MAGENTA,
	};
	int i;		// Generic counter
	int offset;	// We want to animate the color cycling, so we need
					// to offset into the colors array as we output line-
					// by-line
	int bgmode;	// We can flash the background too, so here we remember
					// what background mode we are in.
	static char* const message = "Hello, world!\n";	// Our message

	// Print it out once at the beginning...
	printf("%s", message);	// Boring!
	// Complain about how boring the message was...
	printf("\nWell, that wasn't very interesting...\n");
	// Suggest an alternative...
	printf("How about this:\n\n");
	
	// This is a trick...We're printing out the conclusion of the
	// message first, so that we can go back and animate the message
	// for the rest of the program.  The following escape sequences
	// move the cursor around on the STDIO window (the first is cursor
	// down 2 lines, the second is cursor up 3 lines).
	printf("\x1b[2BAh, much better!\n\x1b[3A");

	// Initialize our offset and background mode
	offset = 0;
	bgmode = 0;

	// Loop forever
	for (;;) {
		// Go back to the beginning of the message
		i = 0;
		// Loop over the characters in the message
		while (i < strlen(message)) {
			// Since the '\n' isn't really visible, don't cycle the
			// color on it
			if (message[i] != '\n') {
				// Change to the next color (wrap around at the end)
				stdio_set(colors[(i + offset) % 6]);
			}
			// Print the character
			printf("%c", message[i]);
			// Go to the next character
			i += 1;
		}
		// This escape sequence moves the cursor back up to the
		// "Hello, world" line
		printf("\x1b[1A");
		// We've reached the end of the message, so let's shift the
		// colors
		offset = (offset + 1) % 6;
		// If we have cycled the colors all the way around, change
		// the background
		if (offset == 0) {
			bgmode = !bgmode;
			if (bgmode) {
				// Switch the background to black
				stdio_set(STDIO_BG_BLACK);
				// Bold colors look better on the black background
				stdio_set(STDIO_BOLD);
			} else {
				// Reset the STDIO attributes (in particular, this
				// resets the background to white)
				stdio_set(STDIO_RESET);
			}
		}
	}
}
