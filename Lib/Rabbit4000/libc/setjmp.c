/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Code for setjmp() and longjmp() as defined in setjmp.h.
*/

/*** BeginHeader setjmp, longjmp */
#include <setjmp.h>
/*** EndHeader */

/* START FUNCTION DESCRIPTION ********************************************
setjmp                                                          <setjmp.h>

SYNTAX: int setjmp(jmp_buf env);

KEYWORDS:

DESCRIPTION: Store the PC (program counter), SP (stack pointer) and other
information about the current state into env.  The saved information can
be restored by executing longjmp. NOTE: you cannot use set/longjmp to
move out of slice statements, costatements, or cofunctions.

Typical usage:

switch (setjmp(e)) {
	case 0:
		// first time
		f();     // try to execute f, which may eventually call longjmp
		break;   // if we get here, f() was successful
	case 1:
		// if we get to here f() must have called longjmp
		// do exception handling
		break;
	case 2:
		// similar to above, just a different exception code.
		...
}

f() {
	g()
	...
}

g(){
	...
	longjmp(e,2);     // exception code 2
							// jump back to setjmp statement,
							// but causes setjmp to return 2, and
							// therefore execute case 2 in the switch
							// statement
}

PARAMETER1: information about the current state

RETURN VALUE: Returns zero if it is executed.  After longjmp is executed,
the program counter, stack pointer and etc. are restored to the state
when setjmp was executed the first time.  However, this time setjmp returns
whatever value is specifed by the longjmp statement.
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
longjmp                                                         <setjmp.h>

SYNTAX: void longjmp(jmp_buf env, int val);

KEYWORDS:

DESCRIPTION: Restores the stack environment saved in jump buffer <env>.  See
				the description of setjmp for details of usage.

PARAMETER1: environment previously saved with setjmp()

PARAMETER2: Value "returned" from the saved setjmp call, changed to 1 if 0.

RETURN VALUE: None.
END DESCRIPTION **********************************************************/

#asm __nodebug

;  SETJMP.CC
;
;  Purpose:
;
;    SETJMP saves its stack environment in array env[] for possible
;    later use by LONGJMP. It returns the value 0.
;
;    LONGJMP restores the stack environment saved in array env[].
;    The only confusing issue is how the return is made, and the effect
;    on the registers.
;
;          LONGJMP RETURN:  HL=VAL
;                           AF,BC,DE clobbered
;                           PC=address after the CALL SETJMP
;                              that created the data in env[].
;                           SP=env[0] - same as set by SETJMP
;
;    The use of SETJMP is not dangerous, since all it does is save
;    values. LONGJMP may be a problem if SETJMP is called recursively.
;
;  Reference: S.R.Bourne, "The Unix System", Addison-Wesley, 1983
;

setjmp::
		pop      bc
		pop      de                   ; point to env[] structure
		clr		hl							; Get Location of SP
		add      hl,sp
		push     de
		push     bc
		ex       de,hl                ; HL = env, DE = SP

		push     ix                   ; Save IX
		ld			ix,hl						; IX now points to env[]

		; save stack pointer
		ld			hl, de
		ld			(ix+[jmp_buf]+retsp),hl

		pop      de							; Copy original IX into DE

		; store IX (now in DE) and return address (in BC)
		ld			(ix+[jmp_buf]+retix),bcde

		ld			hl,lxpc
		ld			(ix+[jmp_buf]+retlxpc),hl				; save LXPC

ioi	ld			hl,(STACKSEGL)
		ld			(ix+[jmp_buf]+retstackseg),hl

		; restore IX
		ld			hl, de
		ld			ix, hl

		clr		hl							; return 0
		ret

longjmp::
		pop      bc                   ; Return Addr (unused)
		pop      hl                   ; Jump Buffer
		pop		bc							; value to return to setjmp()

		ld			ix, hl					; IX points to jump buffer

		; restore LXPC
		ld			hl, (ix+[jmp_buf]+retlxpc)
		ld			lxpc, hl

		; restore stack pointer
		ld			hl, (ix+[jmp_buf]+retsp)
		ld			sp, hl

		; restore stack seg
		ld			hl, (ix+[jmp_buf]+retstackseg)
ioi	ld			(STACKSEGL), hl

		; get return address
		ld			hl, (ix+[jmp_buf]+retaddr)
		ld			de, hl

		; restore IX
		ld			hl, (ix+[jmp_buf]+retix)
		ld			ix, hl

		; IX no longer points to jump buffer (restored to correct value)
		; DE has return address
		; BC has value to pass back

		ld			hl, bc					; return value

		test		hl							; if HL == 0, set it to 1 instead
		jr			nz, .retOK
		inc		hl

.retOK:
		push		de							; dummy parameter popped by setjmp
		push		de							; put on return address
		ret

#endasm