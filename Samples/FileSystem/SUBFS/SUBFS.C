/*****************************************************************************
	Samples\FILESYSTEM\SUBFS\SUBFS.C
	Digi International Inc., 2010

	Show how to use the Simple User Block File System (SUBFS).

	The purpose of SUBFS is to make it easier to manage data in the
	userID block, which is a non-volatile area of storage available on
	all Rabbit boards.

	SUBFS is used to save network configuration if using iDigi, thus
	it is very handy for saving other application data when using
	iDigi for network management.

	Without SUBFS, there are some basic routines in IDBLOCK.LIB which
	allow direct read/write to specified offsets in the userID block.
	This can sometimes cause conflict with other software and libraries,
	and is dificult to manage efficiently.

	SUBFS imposes a simple filesystem over the userID block area.
	The filesystem has the following attributes and limitations:

	1. It is intended for limited amounts of data which are not
	frequently updated.  For example, configuration and calibration data.

	2. Up to 15 files may exist.  Each file has a 1-12 character name.

	3. A file must be completely written from a buffer in memory.  It is
	not possible to append or update part of a file.  This restriction
	is imposed by the following attribute.

	4. Each update operation succeeds or fails completely.  This
	ensures the integrity of the filesystem.  The userID block itself
	provides protection against corruption caused by e.g. power failure
	during write.


	Note that all SUBFS API functions may return -EAGAIN, in which case
	you need to call again with the same parameters to complete the
	process.  This requirement is to deal with boards which have a serial
	boot flash, which may need to wait for another task to finish an
	unrelated operation on the flash.
******************************************************************************/
#use "subfs.lib"

// Stub functions to perform "blocking" write and read.
int save_config(char * name, void * data, int data_len)
{
	int rc;

	while ((rc = subfs_create(name, data, data_len)) == -EAGAIN);
	return rc;
}

int get_config(char * name, void * data, int * data_len)
{
	int rc;
	unsigned long len = *data_len;	// Initialize to expected buffer length

	while ((rc = subfs_read(name, data, 0, &len)) == -EAGAIN);
	*data_len = (int)len;
	return rc;
}




int main(void)
{
	static char data[100];
	int rc;
	int len;

	strcpy(data, "Hello world!");
	len = strlen(data);

	rc = save_config("hello.txt", data, len);
	if (rc)
		printf("save_config rc = %d\n", rc);
	else
		printf("saved %d bytes\n", len);

	memset(data, 0, sizeof(data));

	rc = get_config("hello.txt", data, &len);
	if (rc)
		printf("save_config rc = %d\n", rc);
	else
		printf("File contents (len=%d): %.*s\n", len, len, data);

/*
	// uncomment this to delete file
	subfs_delete("hello.txt");
*/
	return 0;
}