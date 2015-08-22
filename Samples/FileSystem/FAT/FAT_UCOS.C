/**************************************************************
  FAT_UCOS.C
  Digi International, Copyright (c) 2007.  All rights reserved.

  Demonstrate using FAT with uC/OS-II.

  Also requires that you run this on a board with a compatible
  storage medium (serial flash, NAND flash or SD card).

  Versions of the Dynamic C FAT prior to 2.10 did not support
  FAT unless all FAT API calls are done in one task.

  FAT_USE_UCOS_MUTEX must be #defined before #use'ing FAT16.LIB

  fat_InitUCOSMutex(priority)  MUST be called after calling
  OSInit() and before calling FAT APIs or begining multitasking.
  'priority' MUST be higher than all tasks using FAT APIs.

  Only high level fat APIs with names that begin with 'fat_'
  should be called from uC/OS-II tasks.

**************************************************************/
#memmap xmem

#define OS_MUTEX_EN		       1	 	// Enable mutexes
#define OS_TIME_DLY_HMSM_EN	 1    // Enable hour, min, sec, ms delays
#define OS_TASK_SUSPEND_EN     1    // Enable task ssuspension

// Must use mutexes to multitask with FAT,
//  and must use blocking mode
#define FAT_USE_UCOS_MUTEX
#define FAT_BLOCK

// Create 1024 task stacks for FAT tasks just to be safe
#define STACK_CNT_1K 2

// Bring in libraries
#use "ucos2.lib"
#use "fat16.lib"
#ifndef FAT_VERSION
  #error "This program requires FAT16.lib"
#endif

int taskcount;

#define  FILESIZE 4096
char fbuff1[FILESIZE];
char fbuff2[FILESIZE];

void File1Task(void* pdata);
void File2Task(void* pdata);

void main()
{
   int rc;

   OSInit();
   taskcount = 0;

   // MUST Initialize FAT mutex at higher priority than other tasks
   fat_InitUCOSMutex(4);

   rc = fat_AutoMount(FDDF_MOUNT_PART_0    | FDDF_MOUNT_DEV_0     |
                      FDDF_COND_DEV_FORMAT | FDDF_COND_PART_FORMAT );
   if(rc)
   {
      printf("ERROR: fat_AutoMount() reports %ls.\n", strerror(rc));
      exit(rc);
   }

   OSTaskCreate(File1Task, NULL, 1024, 5);
   OSTaskCreate(File2Task, NULL, 1024, 6);
   OSStart();  // Start multi-tasking
}

void File1Task(void* pdata)
{
   auto FATfile my_file;
   auto char ch;
   auto int i, rc, success;

   while (1)
   {
      OSTimeDlyHMSM(0, 0, 0, 100);  // Allow lower priority tasks to run

      success = 0;
      // PARTITION_A = partition 0  on device 0
      rc = fat_Open(PARTITION_A, "file1.txt", FAT_FILE, FAT_CREATE, &my_file,
                    NULL);
      if (0 !=  rc)
      {
         printf("Task1 ERROR: fat_Open() reports %ls.\n", strerror(rc));
      }
      else
      {
         // Fill file with whatever is in the low byte of MS_TIMER
         ch = (char) MS_TIMER;
         memset(fbuff1, ch, FILESIZE);
         rc = fat_Write(&my_file, fbuff1, FILESIZE);
         if (FILESIZE != rc)
         {
            printf("Task1 ERROR: fat_Write() reports %ls.\n", strerror(rc));
         }
         else
         {
            // Make sure it reads back correctly
            memset(fbuff1, 0, FILESIZE);             // clear buffer
            rc = fat_Seek(&my_file, 0L, SEEK_SET);
            if (0 != rc)
            {
               printf("Task1 ERROR: fat_Seek() reports %ls.\n", strerror(rc));
            }
            else
            {
               rc = fat_Read(&my_file, fbuff1, FILESIZE);
               if (FILESIZE != rc)
               {
                  printf("Task1 ERROR: fat_Read() reports %ls.\n",
                         strerror(rc));
               }
               else
               {
                  for (i = 0; i < FILESIZE; i++)
                  {
                     if (fbuff1[i] != ch)
                     {
                        printf("Task1 ERROR: fat_Read() bad value.\n");
                        break;
                     }
                  }
                  if (i == FILESIZE)
                  {
                     success = 1;
                  }
               }
            }
         }
         rc = fat_Close(&my_file);
         if (0 != rc)
         {
            success = 0;
            printf("Task1 ERROR: fat_Close() reports %ls.\n", strerror(rc));
         }
         rc = fat_Delete(PARTITION_A, FAT_FILE, "file1.txt");
         if (0 != rc)
         {
            success = 0;
            printf("Task1 ERROR: fat_Delete() reports '%ls'.\n", strerror(rc));
         }
         if (success)
         {
            printf("Task1 successful\n");
         }
      }

      // quit after 4 passes so we don't wear out flash if left running
      if (taskcount++ > 4)
      {
         rc = fat_UnmountDevice(PARTITION_A->dev);
         if (0 != rc)
         {
            printf("Task1 ERROR: fat_UnmountDevice() reports %ls.\n",
                   strerror(rc));
         }
         printf("Test done: Hit any key to end program");
         OSTaskSuspend(6);
         while (!kbhit());
         exit(0);
      }
   }
}

void File2Task(void* pdata)
{
   auto FATfile my_file;
   auto char ch;
   auto int i, rc, success;

   while (1)
   {
      success = 0;
      rc = fat_Open(PARTITION_A, "file2.txt", FAT_FILE, FAT_CREATE, &my_file,
                    NULL);
      if (0 !=  rc)
      {
         printf("Task2 ERROR: fat_Open() reports %ls.\n", strerror(rc));
      }
      else
      {
         // Fill file with whatever is in the low byte of MS_TIMER
         ch = (char) MS_TIMER;
         memset(fbuff2, ch, FILESIZE);
         rc = fat_Write(&my_file, fbuff2, FILESIZE);
         if (FILESIZE != rc)
         {
            printf("Task2 ERROR: fat_Write() reports %ls.\n", strerror(rc));
         }
         else
         {
            // Make sure it reads back correctly
            memset(fbuff2, 0, FILESIZE);         // clear buffer
            rc = fat_Seek(&my_file, 0L, SEEK_SET);
            if (0 != rc)
            {
               printf("Task2 ERROR: fat_Seek() reports %ls.\n", strerror(rc));
            }
            else
            {
               rc = fat_Read(&my_file, fbuff2, FILESIZE);
               if (FILESIZE != rc)
               {
                  printf("Task2 ERROR: fat_Read() reports %ls.\n",
                         strerror(rc));
               }
               else
               {
                  for (i = 0; i < FILESIZE; i++)
                  {
                     if (fbuff2[i] != ch)
                     {
                        printf("Task2 ERROR: fat_Read() bad value.\n");
                        break;
                     }
                  }
                  if (i == FILESIZE)
                  {
                     success = 1;
                  }
               }
            }
         }
         rc = fat_Close(&my_file);
         if (0 != rc)
         {
            success = 0;
            printf("Task2 ERROR: fat_Close() reports %ls.\n", strerror(rc));
         }
         rc = fat_Delete(PARTITION_A, FAT_FILE, "file2.txt");
         if (0 != rc)
         {
            success = 0;
            printf("Task2 ERROR: fat_Delete() reports '%ls'.\n", strerror(rc));
         }
      }
      if (success)
      {
         printf("Task2 successful\n");
      }
   }
}