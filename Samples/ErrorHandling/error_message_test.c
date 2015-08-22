/********************************************************************

ERROR_MESSAGE_TEST.C

Rabbit Semiconductor, 2006

This program exercises the error_message function for all error codes
between 0 and 799 inclusive in the positive and negative range. If
the error_message function fails to find the error code, the main
routine will print "Unknown error," otherwise it will print the error
message returned using the handy %ls format specifier.

********************************************************************/


void main()
{
	int i, j;
   unsigned long err_string;

   for (i = 0; i < 800; ++i) {
   	j = i % 2 ? i : -i;
      err_string = error_message(j);
      printf("%5d: ", j);
      if (!err_string) {
      	printf("Unknown error\n");
      }
      else {
	   	printf("%ls\n", err_string);
      }
   }

   for (i = 0; i < 800; ++i) {
   	j = i % 2 ? -i : i;
      err_string = error_message(j);
      printf("%5d: ", j);
      if (!err_string) {
      	printf("Unknown error\n");
      }
      else {
	   	printf("%ls\n", err_string);
      }
   }
}