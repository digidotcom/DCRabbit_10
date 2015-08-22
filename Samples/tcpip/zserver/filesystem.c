/*
  Digi International, Copyright (c) 2007.  All rights reserved.
  Sample program to demonstrate use of ZSERVER.LIB functionality.

  It implements a stdio-based console with some Unix-like commands.

  This is a kinda big sample, because it implements a lot of commands,
  but it is useful if you want to know how something is done.

*/

#memmap xmem

// Define one or both of these to include a filesystem
#define DO_FAT			// FAT filesystem.

// Not in a filesystem, but some static resources.
#define DO_FLASH		// Add some flashspec entries
#define DO_RAM			// Add some ramspec entries

#ifdef DO_FAT

	// Necessary for zserver.
	#define FAT_USE_FORWARDSLASH
	// Set FAT library to blocking mode
	#define FAT_BLOCK

	// This brings in all FAT filesystem and driver libraries
	#use "fat16.lib"
#endif

//#define ZSERVER_DEBUG
//#define ZSERVER_VERBOSE
#ifndef DO_FLASH
  #define HTTP_NO_FLASHSPEC		// Remove unnecessary code if not doing static table
#endif
#ifdef DO_RAM
  #define INPUT_COMPRESSION_BUFFERS 4
  #use "zimport.lib"
#endif


#use "zserver.lib"

#ifdef DO_FLASH
	#ximport "samples/tcpip/http/pages/static.html"    index_html
	#ximport "samples/tcpip/http/pages/rabbit1.gif"    rabbit1_gif
	#zimport "samples/tcpip/http/pages/zimport.shtml"        zimport_shtml
	//#zimport "samples/tcpip/http/pages/alice.html"            alice_html
	#zimport "samples/tcpip/http/pages/alice-rabbit.jpg"     alice_jpg

// This shows the new way of initializing the flashspec...

#define ALL_GROUPS	0xFFFF
#define NO_GROUPS		0

SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_P_XMEMFILE("/index.html", index_html,
	         "flash", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD),
	SSPEC_RESOURCE_P_ZMEMFILE("/index.shtml", zimport_shtml, \
	         "flash", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD),
	//SSPEC_RESOURCE_P_ZMEMFILE("/alice.html", alice_html, \
	//         "flash", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD),
	SSPEC_RESOURCE_P_XMEMFILE("/rabbit1.gif", rabbit1_gif, \
	         "flash", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD)
SSPEC_RESOURCETABLE_END


#endif

#ifdef DO_RAM
	long text_size;
	long image_size;
#endif

// OK whether or not using FAT.
sspec_fatinfo fati;
#ifdef DO_FAT
	fat_part fparts[4];	// Support up to 4 partitions for FAT
#endif

int make_webpage(int handle, char * str)
{
	auto int rc;

	rc = sspec_write(handle, str, strlen(str));
   if (rc < 0)
   	rc = 0;
   return rc;
}

char * serverName(word server) {
	switch (server) {
   case SERVER_HTTP:
   	return "HTTP";
   case SERVER_FTP:
   	return "FTP";
   case SERVER_SMTP:
   	return "Email";
   case SERVER_HTTPS:
   	return "HTTPS";
   case SERVER_SNMP:
   	return "SNMP";
   case SERVER_USER:
   	return "USER";
   case SERVER_USER2:
   	return "USER2";
   case SERVER_ANY:
   	return "Legion";	// Because we are many... (Biblical reference here :-)
   }
   return "Unnamed";
}

int main()
{
   char buf[80];
   char path[256];
   char rootdir[80];
   char dfile[80];
	int rc;						// Return code from filesystem functions
   char * p, * cmd, * arg;
   ServerContext ctx;
   int uid, root_user;
   int handle;
   int wlen;
   word mask;
   word fattype;
#ifdef DO_FAT
	auto fat_part * pt;
   auto int pn;
#endif

   strcpy(dfile, "index.htm");

#ifdef DO_RAM
//	xmem2root(&text_size, alice_html, 4);
//	text_size &= ZIMPORT_MASK;
	text_size = 12345;
	image_size = xgetlong(alice_jpg) & ZIMPORT_MASK;
	sspec_addxmemfile("/alice.jpg", alice_jpg, SERVER_HTTP | SERVER_COMPRESSED);
	sspec_addvariable("text_size", &text_size, INT32, "%ld", SERVER_HTTP);
   sspec_addvariable("image_size", &image_size, INT32, "%ld", SERVER_HTTP);

#endif

	printf("Initializing filesystems...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
#ifdef DO_FAT
   memset(&fati, 0, sizeof(fati));
	for (pn = 0; pn < 4; ++pn)
   	fati.part[pn] = fparts + pn;	// Point to un-init part structs for all possible partitions.
#endif
   rc = sspec_automount(SSPEC_MOUNT_ANY, &fati, NULL, NULL);
   if (rc)
   	printf("Failed to initialize, rc=%d\nProceeding anyway...\n", rc);

   // Create a permissions rule for FAT file system.  If the FAT file system
   // is not used, the rules will be ignored.
   rc = sspec_addrule("/A", "FAT-A-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/B", "FAT-B-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/C", "FAT-C-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/D", "FAT-D-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/E", "FAT-E-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/F", "FAT-F-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/G", "FAT-G-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);
   rc = sspec_addrule("/H", "FAT-H-realm", 0xFFFF, 0x0003, SERVER_ANY, 0, NULL);

   // Add users
   uid = root_user = sauth_adduser("root", "super", SERVER_ANY);
   sauth_setwriteaccess(uid, SERVER_ANY);
   uid = sauth_adduser("admin", "work", SERVER_HTTP | SERVER_FTP);
   sauth_setwriteaccess(uid, SERVER_HTTP | SERVER_FTP);

	// Set up current context.
   ctx.userid = sauth_adduser("foo", "bar", SERVER_HTTP);
	ctx.server = SERVER_HTTP;
   ctx.rootdir = "/";
   ctx.dfltname = dfile;
   strcpy(ctx.cwd, "/");

_help:
	printf("\nEnter:\n");
   printf("  cd <dir>      Change current working directory\n");
   printf("  pwd           Print CWD\n");
   printf("  chroot <dir>  Change 'root' directory\n");
   printf("  ls [<dir>]    List CWD or specified directory\n");
   printf("  touch <file>  Create a file\n");
   printf("  cat <file>    Print a file\n");
   printf("  rm <file>     Delete a file\n");
   printf("  su [<name>]   Set current user [root]\n");
   printf("  pass <passwd> Set password of current user\n");
   printf("  whoami        Print current user\n");
   printf("  adduser <name> Add another user\n");
   printf("  rmuser <name> Remove specified user\n");
   printf("  lsuser        List users\n");
   printf("  grp [<mask>]  Change [print] current user's group bits\n");
   printf("  setsrv [<n>]  Become [print] specified server (0=http, 1=ftp, 2=smtp, 9=all)\n");
   printf("  dfile [<file>] Set [print] default file name\n");
#ifdef DO_FAT
   printf("  mount <dir>   Mount FAT partition (/A, /B etc.)\n");
   printf("  umount <dir>  Unmount FAT partition\n");
   printf("  format <dir>  Format FAT partition\n");
   printf("  mkdir <dir>   Create a directory\n");		// Only available for FAT
   printf("  rmdir <dir>   Delete a directory\n");
#endif
   printf("  help          Show this message again\n");
   printf("  exit          Sync filesystems and exit\n");

	for (;;) {
   	printf("[%s,%s]%c ",
      	sauth_getusername(ctx.userid),
         serverName(ctx.server),	// This is our function
         ctx.userid == root_user ? '#' : '%');
      gets(buf);
		p = buf;
      while (isspace(*p)) ++p;
      cmd = p;
      if (!*cmd)
      	continue;
      while (*p && !isspace(*p)) ++p;
      if (*p) {
      	*p = 0;
         ++p;
         while (isspace(*p)) ++p;
         if (*p)
         	arg = p;
         else
         	arg = NULL;
      }
      else
      	arg = NULL;

		if (!strcmp(cmd, "cd")) {
      	if (!arg) {
         	printf("cd: no directory specified\n");
            continue;
         }
         rc = sspec_cd(arg, &ctx, 1);
         if (rc < 0)
         	printf("cd: error code %d\n", rc);
      }
      else if (!strcmp(cmd, "pwd")) {
      	printf("%s\n", sspec_pwd(&ctx, buf));
      }
      else if (!strcmp(cmd, "chroot")) {
      	if (!arg) {
         	printf("chroot: no directory specified\n");
            continue;
         }
         if (arg[0] != '/') {
         	printf("chroot: must give absolute path\n");
            continue;
         }
         if (arg[strlen(arg)-1] != '/')
         	strcat(arg, "/");	// ensure trailing slash - rootdir must have it!
         strcpy(rootdir, arg);
			ctx.rootdir = rootdir;
         rc = sspec_cd("/", &ctx, 1);
         if (rc < 0) {
         	printf("chroot: no such directory (rc = %d): reverting to '/'.\n", rc);
            ctx.rootdir = "/";
            sspec_cd("/", &ctx, 0);
         }
      }
      else if (!strcmp(cmd, "ls")) {
      	if (arg) {
         	// Remember current w.d.
      		sspec_pwd(&ctx, path);
            if ((rc = sspec_cd(arg, &ctx, 1)) < 0) {
            	printf("ls: no such directory %s (error code %d)\n", arg, rc);
               continue;
            }
         }
			for (handle = 0; handle >= 0; ) {
         	handle = sspec_dirlist(handle, buf, sizeof(buf),
                                          &ctx, SSPEC_LIST_LONG);
            if (handle >= 0)
	               printf(buf);
         }
         if (arg)
         	// Put back to previous w.d.
         	sspec_cd(path, &ctx, 0);
      }
      else if (!strcmp(cmd, "touch")) {
      	if (!arg) {
         	printf("touch: no file name specified\n");
            continue;
         }
      	handle = sspec_open(arg, &ctx, O_WRITE|O_CREATE, 0);
         if (handle < 0) {
         	printf("touch: could not create %s, rc = %d\n", arg, handle);
            continue;
         }
         wlen = 0;
	      wlen += make_webpage(handle, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD W3 HTML//EN\">\r\n");
         sprintf(path, "<HTML><HEAD><TITLE>Resource %s</TITLE></HEAD>\r\n", arg);
	      wlen += make_webpage(handle, path);
	      wlen += make_webpage(handle, "<BODY topmargin=\"0\" leftmargin=\"0\" marginwidth=\"0\" marginheight=\"0\"\r\n");
	      wlen += make_webpage(handle, "bgcolor=\"#FFFFFF\" link=\"#009966\" vlink=\"#FFCC00\" alink=\"#006666\">\r\n");
			sprintf(path, "<CENTER><img SRC=\"/rabbit1.gif\"><BR><BR><BR>" \
         					"This is resource %s</CENTER></BODY></HTML>\r\n", arg);
	      wlen += make_webpage(handle, path);
        	printf("touch: wrote %d chars\n", wlen);
         if ((rc = sspec_close(handle)) < 0)
         	printf("touch: failed to close file, rc = %d\n", rc);
      }
      else if (!strcmp(cmd, "cat")) {
      	if (!arg) {
         	printf("cat: no file name specified\n");
            continue;
         }
      	handle = sspec_open(arg, &ctx, O_READ, 0);
         if (handle < 0) {
         	printf("cat: could not open %s, rc = %d\n", arg, handle);
            continue;
         }
         do {
	         rc = sspec_read(handle, path, 80);
	         if (rc < 0) {
            	if (rc != -EEOF)
	            	printf("cat: failed to read, rc = %d\n", rc);
            }
	         else if (rc) {
	            path[rc] = 0;
	            printf("%s", path);
	         }
         } while (rc > 0);
         if ((rc = sspec_close(handle)) < 0)
         	printf("cat: failed to close file, rc = %d\n", rc);
      }
      else if (!strcmp(cmd, "rm")) {
      	if (!arg) {
         	printf("rm: no file name specified\n");
            continue;
         }
         rc = sspec_delete(arg, &ctx);
         if (rc < 0)
         	printf("rm: could not delete file, rc = %d\n", rc);
      }
      else if (!strcmp(cmd, "su")) {
      	if (!arg)
         	arg = "root";
         uid = sauth_getuserid(arg, SERVER_ANY);
         if (uid >= 0)
         	ctx.userid = uid;
         else
         	printf("su: user %s not defined (hint: foo, admin or root)\n", arg);
      }
      else if (!strcmp(cmd, "pass")) {
      	if (!arg) {
         	printf("password for %s is %s\n", sauth_getusername(ctx.userid), sauth_getpassword(ctx.userid));
            continue;
         }
         rc = sauth_setpassword(ctx.userid, arg);
         if (rc)
         	printf("pass: failed to change password\n");
      }
      else if (!strcmp(cmd, "whoami")) {
      	printf("%s (userid=%d)\n", sauth_getusername(ctx.userid), ctx.userid);
      }
      else if (!strcmp(cmd, "adduser")) {
      	if (!arg) {
         	printf("adduser: no user name\n");
            continue;
         }
   		uid = sauth_adduser(arg, "", SERVER_ANY);
         if (uid < 0)
         	printf("adduser: failed to add user, rc = %d\n", uid);
      }
      else if (!strcmp(cmd, "lsuser")) {
      	printf("Userid Username__ Password__ mask__ write_ srvm__\n");
      	for (uid = 0; uid < SAUTH_MAXUSERS; ++uid) {
         	if (!sauth_getusermask(uid, &mask, NULL))
         		printf("%6d %-10s %-10s 0x%04X 0x%04X 0x%04X\n",
            		uid, sauth_getusername(uid), sauth_getpassword(uid),
                  mask, sauth_getwriteaccess(uid), sauth_getserver(uid));
         }
      }
      else if (!strcmp(cmd, "rmuser")) {
      	if (!arg) {
         	printf("rmuser: no user name\n");
            continue;
         }
   		uid = sauth_removeuser(sauth_getuserid(arg, SERVER_ANY));
         if (uid < 0)
         	printf("rmuser: failed to remove user\n");
      }
      else if (!strcmp(cmd, "grp")) {
      	if (arg) {
#ifdef USE_FAR_STRING_LIB
	         mask = (word)_n_strtol(arg, NULL, 0);
#else
	         mask = (word)strtol(arg, NULL, 0);
#endif
	         sauth_setusermask(ctx.userid, mask, NULL);
         }
         sauth_getusermask(ctx.userid, &mask, NULL);
         printf("userid %d mask = 0x%04X\n", ctx.userid, mask);
      }
      else if (!strcmp(cmd, "setsrv")) {
      	if (arg) {
#ifdef USE_FAR_STRING_LIB
	         mask = (word)_n_strtol(arg, NULL, 0);
#else
	         mask = (word)strtol(arg, NULL, 0);
#endif
            if (mask == 9)
            	ctx.server = SERVER_ANY;
            else
	         	ctx.server = 1<<mask;
         }
         printf("server = %s (0x%04X)\n", serverName(ctx.server), ctx.server);
      }
      else if (!strcmp(cmd, "dfile")) {
      	if (arg) {
	         strcpy(dfile, arg);
         }
         printf("dfile = %s\n", ctx.dfltname);
      }
#ifdef DO_FAT
      else if (!strcmp(cmd, "mount")) {
      	if (!arg) {
         	printf("mount: no mount point specified\n");
            continue;
         }
         if (arg[0] == '/' && !arg[2] && arg[1] >= 'A' && arg[1] < 'E') {
         	pn = arg[1] - 'A';
				if (sspec_fatregistered(pn))
            	printf("mount: %s is already mounted\n", arg);
            else {
            	rc = fat_EnumPartition(fati.dev, pn, fati.part[pn]);
               if (rc < 0)
	               printf("Could not enumerate partition %d: rc=%d\n", pn, rc);
	            rc = fat_MountPartition(fati.part[pn]);
	            if (!rc || rc == -EEOF)
	               sspec_fatregister(pn, fati.part[pn]);
	            else
	               printf("Could not mount partition %d: rc=%d\n", pn, rc);
            }
         }
         else
         	printf("mount: %s is not valid FAT mount point\n", arg);
      }
      else if (!strcmp(cmd, "umount")) {
      	if (!arg) {
         	printf("umount: no unmount point specified\n");
            continue;
         }
         if (arg[0] == '/' && !arg[2] && arg[1] >= 'A' && arg[1] < 'E') {
         	pn = arg[1] - 'A';
				if (!(pt = sspec_fatregistered(pn)))
            	printf("umount: %s is not mounted\n", arg);
            else {
	            sspec_fatregister(pn, NULL);	// unregister to zserver
	            fat_UnmountPartition(fati.part[pn]);
            }
         }
         else
         	printf("umount: %s is not valid FAT mount point\n", arg);
      }
      else if (!strcmp(cmd, "format")) {
      	if (!arg) {
         	printf("format: no FAT mount point specified\n");
            continue;
         }
         if (arg[0] == '/' && !arg[2] && arg[1] >= 'A' && arg[1] < 'E') {
         	pn = arg[1] - 'A';
				if (sspec_fatregistered(pn))
            	printf("format: %s is mounted (unmount it first)\n", arg);
            else {
	            fat_FormatPartition(fati.dev, fati.part[pn], pn, 6, "zserver", NULL);
            }
         }
         else
         	printf("format: %s is not valid FAT mount point\n", arg);
      }
      else if (!strcmp(cmd, "mkdir")) {
      	if (!arg) {
         	printf("mkdir: no directory name specified\n");
            continue;
         }
         rc = sspec_mkdir(arg, &ctx);
         if (rc < 0)
         	printf("mkdir: could not create %s, rc = %d\n", arg, rc);
      }
      else if (!strcmp(cmd, "rmdir")) {
      	if (!arg) {
         	printf("rmdir: no directory name specified\n");
            continue;
         }
         rc = sspec_rmdir(arg, &ctx);
         if (rc < 0)
         	printf("rmdir: could not delete %s, rc = %d\n", arg, rc);
      }
#endif
      else if (!strcmp(cmd, "help"))
      	goto _help;
      else if (!strcmp(cmd, "exit")) {
#ifdef DO_FAT
      	for (pn = 0; pn < 4; ++pn)
         	if (pt = sspec_fatregistered(pn))
            	fat_UnmountPartition(pt);
#endif
      	break;
      }
      else
      	printf("%s: command not found.  Enter 'help'.\n", cmd);
   }
	return 0;
}