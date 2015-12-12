/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
 * errno.lib
 *
 * Error code definitions, returned from e.g. filesystem.
 *
 * Change History:
 *  2001 Apr 11 - SJH - Created
 *  2004 Aug 12 - SJH - Incorporated NETERRNO.LIB, moved to base lib dir.
 *  2008 Aug 26 - SJH - Moved SSL/TLS error codes here
 *  2010 Feb 24 - TBC - moved to include/errno.h and libc/errno.c
 */


/*
	C90 - 7.1.4 Errors
*/

#ifndef __ERRNO_H
#define __ERRNO_H

// Codes derived from Unix header file; meaningless (to Rabbit) values deleted
// but still reserved.  Codes specific to FILESYSTEM.LIB start at code 201.
// Note that the existence of a code definition below does not imply that the
// code is actually generated in any library.  Some codes are reserved for
// future use.  A function which generates error codes will document the
// possible returned codes.

// The setting of the 'errno' global variable is discouraged, since it makes
// routines thereby non-reentrant.  The preferred convention, used by most of
// the libraries, is to use an integer return code, with the code set to the
// negative of one of the following E*** codes.

extern int errno;

// Note: Be sure to update the errmsg_xstrings and errmsg_indices arrays
//  in errors.lib so that the error_message function will be able to report
//  the new error code. Also update errmsg.ini so that if the new error code
//  is unhandled, Dynamic C will be able to report the error.

// No error; generally the return code should be zero
#define	ENONE		 		 0

// Unix/Posix-compatible codes...
#define	EPERM				 1	/* FFFF Operation not permitted (this is not for user
											  access control, see EACCES for that) */
#define	ENOENT			 2	/* FFFE No such file or directory */
#define	EIO			 	 5	/* FFFB I/O error */
#define	ENXIO				 6	/* FFFA No such device or address */
#define	E2BIG				 7	/* FFF9 Arg list too long, or something else is too big
											  like a string parameter */
#define	EBADF				 9	/* FFF7 Bad file number */
#define	EAGAIN			11	/* FFF5 Try again */
#define	EWOULDBLOCK	EAGAIN	     /* Operation would block */
#define	ENOMEM			12	/* FFF4 Out of memory */
#define	EACCES			13	/* FFF3 Permission denied */
#define	EFAULT			14	/* FFF2 Bad address */
#define	ENOTBLK			15	/* FFF1 Block device required */
#define	EBUSY				16	/* FFF0 Device or resource busy */
#define	EEXIST			17	/* FFEF File exists */
#define	ENODEV			19	/* FFED No such device */
#define	ENOTDIR			20	/* FFEC Not a directory */
#define	EISDIR			21	/* FFEB Is a directory */
#define	EINVAL			22	/* FFEA Invalid argument */
#define	ENFILE			23	/* FFE9 File table overflow */
#define	EMFILE			24	/* FFE8 Too many open files */
#define	EFBIG				27	/* FFE5 File too large */
#define	ENOSPC			28	/* FFE4 No space left on device */

//  Added for I2C interface errors
#define  ELOSTARB        25    /* FFE7 Lost arbitration dispute */
#define  EBUSNAK         26    /* FFE6 Bus or NACK error */
#define  ESLVADDR			 29    /* FFE3 Invalid slave address */

#define	EROFS				30	/* FFE2 Read-only file system */
#define	EDOM				ERR_DOMAIN	/* Math argument out of domain of func */
#define	ERANGE			ERR_RANGE	/* Math result not representable */
#define	ENAMETOOLONG	36	/* FFDC File name too long */
#define	ENOSYS			38	/* FFDA Function not implemented */
#define	ENOTEMPTY		39	/* FFD9 Directory not empty */
#define	EEOF				41	/* FFD7 End of file or directory */
#define	ENODATA			61	/* FFC3 No data available */
#define	ETIME				62	/* FFC2 Timer expired */
#define	ENONET			64	/* FFC0 Machine is not on the network */
#define	ENOLINK			67	/* FFBD Link has been severed */
#define	ECOMM				70	/* FFBA Communication error on send */
#define	EPROTO			71	/* FFB9 Protocol error */
#define	EMULTIHOP		72	/* FFB8 Multihop attempted */
#define	EBADMSG			74	/* FFB6 Not a data message */
#define	EOVERFLOW		75	/* FFB5 Value too large for defined data type */
#define	ENOTUNIQ			76	/* FFB4 Name not unique on network */
#define	EBADFD			77	/* FFB3 File descriptor in bad state */
#define	EILSEQ			84	/* FFAC Illegal byte sequence */
#define	ERESTART			85	/* FFAB Interrupted system call should be restarted */
#define	EUSERS			87	/* FFA9 Too many users */
#define	ENOTSOCK			88	/* FFA8 Socket operation on non-socket */
#define	EDESTADDRREQ	89	/* FFA7 Destination address required */
#define	EMSGSIZE			90	/* FFA6 Message too long */
#define	EPROTOTYPE		91	/* FFA5 Protocol wrong type for socket */
#define	ENOPROTOOPT		92	/* FFA4 Protocol not available */
#define	EPROTONOSUPPORT	93	/* FFA3 Protocol not supported */
#define	ESOCKTNOSUPPORT	94	/* FFA2 Socket type not supported */
#define	EOPNOTSUPP		95	/* FFA1 Operation not supported on transport endpoint */
#define	EPFNOSUPPORT	96	/* FFA0 Protocol family not supported */
#define	EAFNOSUPPORT	97	/* FF9F Address family not supported by protocol */
#define	EADDRINUSE		98	/* FF9E Address already in use */
#define	EADDRNOTAVAIL	99	/* FF9D Cannot assign requested address */
#define	ENETDOWN			100	/* FF9C Network is down */
#define	ENETUNREACH		101	/* FF9B Network is unreachable */
#define	ENETRESET		102	/* FF9A Network dropped connection because of reset */
#define	ECONNABORTED	103	/* FF99 Software caused connection abort */
#define	ECONNRESET		104	/* FF98 Connection reset by peer */
#define	ENOBUFS			105	/* FF97 No buffer space available */
#define	EISCONN			106	/* FF96 Transport endpoint is already connected */
#define	ENOTCONN			107	/* FF95 Transport endpoint is not connected */
#define	ESHUTDOWN		108	/* FF94 Cannot send after transport endpoint shutdown */
#define	ETOOMANYREFS	109	/* FF93 Too many references: cannot splice */
#define	ETIMEDOUT		110	/* FF92 Connection timed out */
#define	ECONNREFUSED	111	/* FF91 Connection refused */
#define	EHOSTDOWN		112	/* FF90 Host is down */
#define	EHOSTUNREACH	113	/* FF8F No route to host */
#define	EALREADY			114	/* FF8E Operation already in progress */
#define	EINPROGRESS		115	/* FF8D Operation now in progress */
#define	EDQUOT			122	/* FF86 Quota exceeded */
#define	ENOMEDIUM		123	/* FF85 No medium found */
#define	EMEDIUMTYPE		124	/* FF84 Wrong medium type */
// Values up to and including 200 reserved

// Codes specific to Dynamic C Filesystem Mk II...
#define	EBADSEQ			201	/* FF37 Bad sequence number in file */
#define	EUNEXEOC			202	/* FF36 Unexpected end-of-chain in file */
#define	ENOTB				203	/* FF35 Not a data block (B-block) */
#define	EBADFNUM			204	/* FF34 Bad file number in file */

// Codes specific to Dynamic C driver level libraries
#define  ESHAREDBUSY    240   /* FF10 Shared port is not available */

// Codes specific to FAT filesystem...
#define  EFATMUTEX      300   /* FED4 FAT Mutex error (uC/OS) */
#define	EROOTFULL		301	/* FED3 Root directory full */
#define	ENOPART			302	/* FED2 Not partitioned */
#define	EBADPART			303	/* FED1 Partition bad or unrecognized */
#define	EUNFORMAT		304	/* FED0 Partition or volume not formatted */
#define	ETYPE				305	/* FECF Bad type */
#define  EPATHSTR			306 	/* FECE Bad file/dir path string */
#define  EBADBLOCK		307   /* FECD Block marked bad on the device */
#define  EBADDATA       308   /* FECC Error detected in read data */
#define  EDRVBUSY       309   // FECB Driver level is busy, new write not started
#define  EUNFLUSHABLE	310	// FECA Cannot flush enough entries from cache to
										//      perform next read.  There are pending dirty
                              // 	  cache entries from a previous boot.  Register
                              //      all devices and this may go away.  If not,
                              //      there are dirty entries for a removable
                              //      medium, which is not mounted.  In this case,
                              //      call fatwtc_flushdev with the unregister flag.
#define  EMISMATCH		311	// FEC9 Parameter mismatch when registering a device.
										//      The device had outstanding cache entries from
                              //      previous boot, but the caller is attempting to
                              //      change the cusize or removable status.
#define  EDEVNOTREG		312	// FEC8 Internal error: device not registered when
										//      _fatwtc_devwrite called.
#define  EPARTIALWRITE	313	// FEC7 Internal error: not writing full physical
										//      sector in _fatwtc_devwrite.
#define  EJOVERFLOW		314	// FEC6 Rollback journal overflow.  Transaction
										//      requires too much data to be stored.  Either
                              //      increase FAT_MAXRJ in the BIOS, or review
                              //      calling code to make sure transactions are
                              //      terminated at the right time and do not
                              //      journal unnecessary data.
#define  ETRANSOPEN		315	// FEC5 fatrj_transtart() called with transaction
										//      already open.
#define  EBROKENTIE		316	// FEC4 Internal error: a tied cache group is in an
										//      inconsistent state.
#define  ETRANSNOTOPEN	317	// FEC3 fatrj_setchk() called without transaction
										// 	  being open.
#define  ECMCONFLICT 	318	// FEC2 Transaction cannot contain both checkpoint and
										//      marker data.
#define  EFSTATE		 	319	// FEC1 File is in an invalid state.  Probably because
										// 	  the FATfile structure was not zero when opened
                              // 	  for the first time.
#define  EPSTATE		 	320	// FEC0 Partition is in an invalid state.  This occurs
										// 	  if you are trying to delete a file when another
                              //      file is being allocated, or vice versa
#define  ECORRUPT			321	// FEBF FAT filesystem appears to be corrupted
#define  EWRITEFAIL     322   // FEBE FAT Write operation failed
#define  EERASEFAIL     323   // FEBD FAT Erase operation failed
#define  EBADMAX			324   // FEBC FTL based device has too many bad blocks

// Following network-specific codes moved from old NETERRNO.LIB.  The original
// code has 400 added so that the codes are globally unique.  Note that when
// stored in the socket error message queue, 400 is subtracted so that codes
// 1..37 correspond to these error codes.  If unix-style error codes are stored,
// then after subtraction of 400 and modulo 256, the resulting codes are in the
// range 113 to 236, which are still distinct.  Error codes in the 2xx or 3xx
// range are never stored in network sockets.
#define NETERR_NONE		 					400	// FE70 No network error
#define NETERR_NOHOST_ARP					401	// FE6F Local host not reachable
															//		  (ARP could not resolve)
#define NETERR_NOHOST_RTE					402	// FE6E Host not reachable (Router not
															//		  resolved)
#define NETERR_HOST_REFUSED				403	// FE6D Host refused connection
#define NETERR_OPEN_TIMEOUT				404	// FE6C Timeout on open or close
#define NETERR_CONN_TIMEOUT				405	// FE6B Connection timed out (keepalive etc.)
#define NETERR_ABORT							406	// FE6A Active abort sent from this peer
#define NETERR_INACTIVE_TIMEOUT			407	// FE69 Timed out due to inactivity
#define NETERR_LEASE_EXPIRED				408	// FE68 DHCP lease expired
#define NETERR_ICMP							409	// FE67 ICMP reported trouble
#define NETERR_PROTOCOL						410	// FE66 Protocol error e.g. bad ack
															//		  number for TCP
#define NETERR_REMOTE_RESET				411	// FE65 Remote peer reset connection
#define NETERR_NET_UNREACH					412	// FE64 ICMP errors...
#define NETERR_HOST_UNREACH				413	// FE63
#define NETERR_PROTO_UNREACH				414	// FE62
#define NETERR_PORT_UNREACH				415	// FE61
#define NETERR_FRAG_NEEDED					416	// FE60
#define NETERR_SOURCE_ROUTE_FAIL			417	// FE5F
#define NETERR_DEST_NET_UNKNOWN			418	// FE5E
#define NETERR_DEST_HOST_UNKNOWN			419	// FE5D
#define NETERR_ISOLATED						420	// FE5C
#define NETERR_DEST_NET_PRHB				421	// FE5B
#define NETERR_DEST_HOST_PRHB				422	// FE5A
#define NETERR_NET_TOS_UNREACH			423	// FE59
#define NETERR_HOST_TOS_UNREACH			424	// FE58
#define NETERR_TTL_EXC						425	// FE57
#define NETERR_FRAG_REASM_EXC				426	// FE56
#define NETERR_REDIRECT_NET				427	// FE55
#define NETERR_REDIRECT_HOST				428	// FE54
#define NETERR_REDIRECT_TOS_NET			429	// FE53
#define NETERR_REDIRECT_TOS_HOST			430	// FE52
#define NETERR_PARAM_PROB					431	// FE51
#define NETERR_PARAM_PROB_REQOPT			432	// FE50
#define NETERR_SOURCE_QUENCH				433	// FE4F ...last ICMP error
#define NETERR_IPADDR_CHANGE				434	// FE4E IP address changed
#define NETERR_OUT_OF_MEMORY				435   // FE4D No memory for buffer etc.
#define NETERR_IPADDR_CONFLICT			436	// FE4C IP address conflict detected
#define NETERR_IFDOWN						437	// FE4B Interface down or deactivated
// errors 438-499 need to be copied to errmsg.ini and two places in errors.lib
// and to neterrno.lib.  Try to fill in the 440's gap.
#define NETERR_SMTP_TIME					438	// FE4A email sending timeout
#define NETERR_SMTP_UNEXPECTED			439	// FE49 invalid response from SMTP server
#define NETERR_HTTPC_REDIRECT				440	// FE48 too many redirects
//													441	// FE47
//													442	// FE46
//													443	// FE45
//													444	// FE44
//													445	// FE43
//													446	// FE42
//													447	// FE41
//													448	// FE40
//													449	// FE3F
#define NETERR_SMTP_NOSOCK 				450   // FE3E could not open network socket
#define NETERR_DNSERROR					   451	// FE3D cannot resolve hostname
#define NETERR_SMTP_DNSERROR			   NETERR_DNSERROR	// alias for old macro
#define NETERR_SMTP_ABORTED				452	// FE3C transaction aborted data handler
#define NETERR_SMTP_AUTH_UNAVAILABLE   453	// FE3B unable to attempt authentication
#define NETERR_SMTP_AUTH_FAILED		   454	// FE3A attempts to authenticate failed

// DNS lookup errors
#define NETERR_RESOLVE_FAILED				460	// Nameserver resolve failed
#define NETERR_RESOLVE_TIMEDOUT			461	// Nameserver resolve timed out
#define NETERR_RESOLVE_HANDLENOTVALID	462	// Nameserver resolve handle invalid
#define NETERR_RESOLVE_NOENTRIES			463	// Nameserver resolve no handles available
#define NETERR_RESOLVE_LONGHOSTNAME		464	// Nameserver resolve hostname too long
#define NETERR_RESOLVE_NONAMESERVER		465	// No nameserver available



// These codes returned by aton2()...  (No neterror_strings entry!)
#define NETERR_NOTDIGIT						501	// FE0B aton2() encountered non-digit
															//	     in IP address
#define NETERR_TOOBIG						502	// FE0A aton2() encountered decimal
															//		  field outside 0-255
#define NETERR_BADDELIM						503	// FE09 aton2() encountered delimiter
															//		  not '.' or ','.
#define NETERR_UNBALANCED					504	// FE08 aton2() did not find ']' for '['.
#define NETERR_BADPORT						505	// FE07 aton2() port number out of range.

// These codes are specific to RABBITSYS
#define _SYS_ERR_WATCHMEM					601	// FDA7 watch log full
#define _SYS_ERR_INVALID_STATE			602   // FDA6 state machine has invalid state
#define _SYS_PROGRAM_ENDED					603	// FDA5 user program ended.
#define _SYS_NO_HANDLES						604   // FDA4 no handles available. Close
															//		  something.
#define _SYS_STACK_LIMIT_VIOLATION		605   // FDA3 Stack limit violation occurred
															//		  in user mode.
#define _SYS_SYSTEM_MODE_VIOLATION		606   // FDA2 Processor attempted to execute
															//		  system code while running in
															//		  user mode.
#define _SYS_WRITE_PROTECT_VIOLATION	607	// FDA1 Write protection violation
															//		  occurred in user mode.
#define _SYS_UNDEFINED_USER_SYSCALL		608	// FDA0 user syscall undefined
#define _SYS_NO_SHADOW						609   // FD9F No shadow associated with I/O register
#define _SYS_NO_MORE_SOCKETS				610	// FD9E Out of udp or tcp sockets.

// Run-time Dynamic C errors
#define  ERR_SECONDARYWDTO             701   // FD43
#define  ERR_ASSERTFAILURE             702   // FD42
#define  ERR_LZINPUTBUFFERS            703   // FD41
#define  ERR_LZOUTPUTBUFFERS           704   // FD40
#define	ERR_BADPOINTER                705   // FD3F
#define	ERR_BADARRAYINDEX             706   // FD3E
#define	ERR_STACKCORRUPTED            707   // FD3D (not currently used)
#define	ERR_STACKOVERFLOW             708   // FD3C (used by RabbitSys)
#define	ERR_AUXSTACKOVERFLOW          709   // FD3B (not currently used)
#define	ERR_DOMAIN                    710   // FD3A
#define	ERR_RANGE                     711   // FD39
#define	ERR_FLOATOVERFLOW             712   // FD38
#define	ERR_LONGDIVBYZERO             713   // FD37
#define	ERR_LONGZEROMODULUS           714   // FD36
#define	ERR_BADPARAMETER              715   // FD35
#define	ERR_INTDIVBYZERO              716   // FD34
#define	ERR_UNEXPECTEDINTRPT          717   // FD33
#define	ERR_CORRUPTEDCODATA           718   // FD32
#define	ERR_VIRTWDOGTIMEOUT           719   // FD31
#define	ERR_BADXALLOC                 720   // FD30
#define	ERR_BADSTACKALLOC             721   // FD2F
#define	ERR_BADSTACKDEALLOC           722   // FD2E
#define	ERR_BADXALLOCINIT             723   // FD2D
#define	ERR_NOVIRTWDOGAVAIL           724   // FD2C
#define	ERR_INVALIDMACADDR            725   // FD2B
#define	ERR_INVALIDCOFUNC             726   // FD2A
#define  ERR_TCPSOCKETISAUTO           727   // FD29
#define  ERR_STACKINVALID					728   // FD28
#define	ERR_UNEXPECTEDRST10			   729   // FD27
#define	ERR_UNEXPECTEDSYSCALL         730   // FD26
#define	ERR_UNEXPECTEDRST38				731   // FD25
#define	ERR_UNEXPECTEDSLAVE           732   // FD24
#define  ERR_UNEXPECTEDWPV             733   // FD23
#define	ERR_UNEXPECTEDTIMERA          734   // FD22
#define	ERR_UNEXPECTEDTIMERB          735   // FD21
#define	ERR_UNEXPECTEDSERIALB         736   // FD20
#define	ERR_UNEXPECTEDSERIALC         737   // FD1F
#define	ERR_UNEXPECTEDSERIALD         738   // FD1E
#define	ERR_UNEXPECTEDEXT0            739   // FD1D
#define	ERR_UNEXPECTEDEXT1            740   // FD1C
#define	ERR_UNEXPECTEDPWM             741   // FD1B
#define  ERR_UNEXPECTEDSMV             742   // FD1A
#define  ERR_UNEXPECTEDQUAD				743   // FD19
#define	ERR_UNEXPECTEDINPUTCAP        744   // FD18
#define	ERR_UNEXPECTEDSSLV				745   // FD17
#define	ERR_UNEXPECTEDSERIALE         746   // FD16
#define	ERR_UNEXPECTEDSERIALF         747   // FD15
#define	ERR_UNKNOWNPLDPFRESOURCEREQ	748   // FD14
#define	ERR_INITNOTCALLED					749	// FD13
#define	ERR_LIBCLOCKSPEED					750	// FD12
#define	ERR_UNEXPECTEDRETURN				751	// FD11 Unexpected return from coprocess
#define	ERR_SPI_MUTEX_ERROR				752	// FD10

// Errors from malloc library
#define  ERR_HEAP_CORRUPT					780	// FCF4 Heap corruption detected
#define  ERR_HEAP_USAGE						781	// FCF3 Heap usage error detected (e.g. double free)
#define  ERR_SYS_HEAP_NULL					782	// FCF2 System heap could not be allocated
															//  - no xalloc memory available for fist _sys_malloc().
#define  ERR_APP_HEAP_NULL					783	// FCF1 Application heap could not be allocated
															//  - no xalloc memory available for fist _app_malloc().

// Zigbee errors
#define  ZBERR_FRAMEID_MISMATCH			801	// FCDF frame id of received message incorrect
#define  ZBERR_INCORRECT_FW_VERSION		802	// FCDE incorrect Radio firmware version
#define  ZBERR_TX_LOCKED               803	// FCDD Radio transmissions are currently blocked
#define  ZBERR_AT_CMD_RESP_STATUS		804	// FCDC Radio returned failure on AT command
#define  ZBERR_CALLBACK_LOCK           805	// FCDB Serial link temporarily locked
#define  ZBERR_BOOTLOAD_FAILED			806	// FCDA
#define  ZBERR_INCORRECT_FW_TYPE 		807	// FCD9 incorrect Radio firmware type


/* 900-999 range reserved for SSL/TLS errors */

#define SSL_ALLOC_FAIL              901 // Memory allocation failure
#define SSL_INIT_ALLOC_FAIL         903 // Initialization failed to allocate new
                                        // resources (increase SSL_MAX_CONNECTIONS)
#define SSL_SEQ_NUM_OVERFLOW        908 // Overflowed a sequence
#define SSL_READ_USER_CANCEL        911 // Remote user cancelled connection
#define SSL_READ_PROTOCOL_VER       912 // Incoming message had wrong version
#define SSL_READ_REC_OVERFLOW       913 // Incoming record message overflow
#define SSL_READ_UNEXPECTED_MSG     916 // Unexpected message during read
#define SSL_READ_RECEIVED_ALERT     917 // Receieved a fatal alert
#define SSL_READ_WRONG_LENGTH			919 // Internal error, the length of a record
                                        // received did not match header length
#define SSL_HS_UNEXPECTED_MSG       921 // Unexpected message during handshake
#define SSL_HS_SESSION_ID_TOO_LONG  922 // Session ID in message too long
#define SSL_HS_NO_RENEGOTIATION     923 // Peer attempted renegotiation when
                                        // not allowed.
#define SSL_HS_CLI_PROTOCOL_ERROR   924 // Incorrect protocol version in client
                                        // hello message
#define SSL_CIPHER_CHOICE_ERROR     926 // could not find compatible ciphersuite
                                        // in client offer
#define SSL_EXCH_KEYS_PROTOCOL_VER  927 // Exchange keys wrong protocol version
#define SSL_PUB_KEY_DECRYPTION_FAIL 928 // Public-key signature verification failure
#define SSL_PUB_KEY_INT_ERROR       929 // Internal public-key operation error
#define SSL_FINISH_VERIFY_FAIL      930 // The finish message verification failed
#define SSL_BAD_RECORD_MAC          933 // Received a message from the client
                                        // that failed the record MAC check
#define SSL_COULD_NOT_SEND_ALERT    936 // The write of an alert failed (internal
                                        // write buffer overflow most likely)
#define SSL_READ_BLOCK_CIPHER_ERROR	938 // Received record did not have length
													 // which was multiple of cipher block size
#define SSL_READ_PROTOCOL_ERROR		939 // Protocol error in received record e.g.
													 // the record contents were shorter than
													 // expected for the given record type, or
													 // app data received when handshake
													 // incomplete
#define SSL_WRITE_REC_TOO_BIG			940 // Record too large to fit in write buffer
#define SSL_WRITE_BLOCK_CIPHER_ERROR 941 // Attempted to write record which did
													 // not have length
													 // which was multiple of cipher block size
#define SSL_PUB_KEY_ENCRYPTION_FAIL 942 // Public-key encryption failure
#define SSL_PRIV_KEY_DECRYPTION_FAIL 944// Private-key decryption failure
#define SSL_PRIV_KEY_ENCRYPTION_FAIL 945// Private-key signature failure
//----------
#define SSL_CERT_CODE_BASE				950 // Not an error, just start of following
													 // block of codes, which are based on
													 // error codes from X509 library...
#define SSL_BAD_CERT                         951 // Bad certificate format
#define SSL_VALIDATE_UNSUPPORTED_CERTIFICATE 952
#define SSL_VALIDATE_CERTIFICATE_REVOKED     953
#define SSL_VALIDATE_CERTIFICATE_EXPIRED     954
#define SSL_VALIDATE_CERTIFICATE_UNKNOWN     955
#define SSL_VALIDATE_UNKNOWN_CA              956
//----------

#define SSL_BAD_STATE_CLI_HELLO     960 // Inappropriate state to be attempting
													 // to send client hello.  State must be
													 // initial (SSL_STATE_LISTEN) or done
													 // (SSL_STATE_DONE).
#define SSL_UNKNOWN_IDENTITY        961 // Unknown PSK identity.
#define SSL_HELLO_EXT_DECODE_ERROR  962 // Error decoding extensions in client
                                        // hello.

// Run Time Analog Errors (Made near max. negative to keep outside ADC ranges)
#define  ERR_ANA_INVAL               32000   // 8300 invalid parameter value
#define  ERR_ANA_CALIB               32001   // 82FF error reading/writing calibration factor

#endif