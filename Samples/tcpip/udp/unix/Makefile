##########################
#
#	Build UNIX UDP send executable
#

CC = gcc
CFLAGS = -Wall 
.PHONY : all clean tarball

all :	unix_udp_send

clean :
	rm -f *.o unix_udp_send *~ core*

tarball :
	tar cvf ../unix-udp.tar Makefile *.c

# -----------------------------------------------

unix_udp_send :	unix_udp_send.c


