/*****************************************************

     far_demo1.c
     Z-World, 2006

		This is a sample program for demonstration of
		the FAR keyword.
******************************************************/

// This variable is stored in FAR but can be accessed just like other
// variables.
far int i;

// const data in far is a useful way to store large amounts of static data such
// as strings.
const far int k = 20000;

main() {
	// Local variables in FAR must be declared static.
	static far int j;

	i = 0;

	while (1) {
		i++;

		for (j = 0; j < k; j++);

		printf("i = %d\n", i);
	}
}