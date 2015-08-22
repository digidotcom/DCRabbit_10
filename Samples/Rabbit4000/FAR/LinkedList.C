/******************************************************************************\
 * LinkedList.C                                                               *
 * Z-World 2006                                                               *
 *                                                                            *
 * This sample is meant for Rabbit 4000 series processors with FAR support.   *
 *                                                                            *
 * This is a demonstration of FAR and the use of FAR pointers and FAR data.   *
 * The library "LinkedList.lib" contains a group of functions for creating    *
 * and manipulating a linked list of data.  The data can be anything from a   *
 * string or an integer to a complex struct of user data.  All the user has   *
 * to do is supply a comparison function for sorting.                         *
 *                                                                            *
 * The example below uses a simple data structure with a string and a long.   *
 *                                                                            *
 * The linked list library functions as a simple memory manager. When the     *
 * list is created, the memory for MAX_LIST_SIZE elements is allocated. When  *
 * data is inserted, it simply uses this allocated space. The space used by   *
 * deleted items is reused with the insertion of each new item.               *
\******************************************************************************/

// Uncomment the following line to debug the linked list functions.
//#define LL_DEBUG
// The maximum size of the list (allocated upon creation)
#define MAX_LIST_SIZE 20
#use LINKEDLIST.LIB

// List handle.  This is necessary because the linked list library supports
// multiple lists and doesn't remember which one is which.  The handle also
// keeps track of the size of the user data and the function for comparison.
ListHandle list;

/******************************************************************************\
	Begin User Data/Functions.

	This section is where the data structure and compare/print functions are
	defined for the custom data structure.
\******************************************************************************/

// A simple data structure with user names and status
struct _mydata {
	char name[50];
	int status;
};
typedef struct _mydata custom_data;

void custom_print(const far custom_data * data) {
	printf("  Status: %d Name: %ls\n", data->status, data->name);
}

int custom_compare(const far custom_data * lhs, const far custom_data * rhs)
{
	// return 0 if name is the same, so we can delete users by just their name.
	if(strcmp(lhs->name, rhs->name) == 0)
		return 0;

	// sort by status first
	if(lhs->status < rhs->status)
		return -1;
	if(lhs->status > rhs->status)
		return 1;

	// if the status is the same, sort by name.
	return strcmp(lhs->name, rhs->name);
}

// This function just asks for a user name.  We only find by name, not status.
custom_data * custom_finder()
{
	static custom_data d;
	printf("Name: ");
	gets(d.name);
	return &d;
}

// This is for creation of new users.
custom_data * custom_input()
{
	static custom_data d;

	do {
		printf("Status (0-3): ");
		d.status = atoi(gets(d.name));
	} while(d.status < 0 || d.status > 3);

	printf("Name: ");
	gets(d.name);
	return &d;
}

/******************************************************************************\
	End User Data/Functions.
\******************************************************************************/

// UI Function prototypes and variables
void menu();
int input();
enum { PUSH_FRONT, PUSH_BACK, POP_FRONT, POP_BACK, REVERSE,
		 REMOVE, EMPTY, CLEAR, PRINT, SIZE, SORT, HELP };

int main()
{
	// List iterator.  The iterator is used to traverse the list. The example of
	// this can be seen in the print section below.
	Iterator it;

	Create(&list, sizeof(custom_data), custom_compare);
   menu();

	while(1)
	{
		printf("\n> ");
		switch(input())
		{
			case PUSH_FRONT:
				Push_front(&list, custom_input());
				break;
			case PUSH_BACK:
				Push_back(&list, custom_input());
				break;
			case POP_FRONT:
				Pop_front(&list);
				printf("First element deleted.\n");
				break;
			case POP_BACK:
				Pop_back(&list);
				printf("Last element deleted.\n");
				break;
			case REVERSE:
				Reverse(&list);
				printf("The list has been reversed.\n");
				break;
			case REMOVE:
				Remove(&list, custom_finder());
				printf("All matching elements have been removed.\n");
				break;
			case SORT:
				Sort(&list);
				printf("The list has been sorted.\n");
				break;
			case EMPTY:
				if(Empty(&list))
					printf("List is Empty.\n");
				else
					printf("List is NOT Empty.\n");
				break;
			case SIZE:
				printf("List contains %d elements.\n", Size(&list));
				break;
			case CLEAR:
				Clear(&list);
				printf("The list has been cleared.\n");
				break;

			case PRINT:
				printf("Printing list...\n");
	         for(Begin(&list, &it); Get(&it) != (far void *)NULL; Next(&it)) {
					custom_print(Get(&it));
	         }
				break;

			case HELP:
				menu();
				break;

			default:
				printf("Unknown command.\n");
		}
	}
}

void menu()
{
	printf("\n");
	printf("|-----------------------------|\n");
	printf("| Instructions:               |\n");
	printf("| 'F' Insert at Front of list |\n");
	printf("| 'B' Insert at Back of list  |\n");
	printf("| 'T' Remove at Front of list |\n");
	printf("| 'K' Remove at Back of list  |\n");
	printf("| 'R' Remove elements in list |\n");
	printf("| 'E' Check if list is empty  |\n");
	printf("| 'C' Clear entire list       |\n");
	printf("| 'P' Print elements in list  |\n");
	printf("| 'Z' Size of list            |\n");
	printf("| 'S' Sort list               |\n");
	printf("| 'V' Reverse the list        |\n");
	printf("| 'H' Print this help menu    |\n");
	printf("| 'Q' Quit                    |\n");
	printf("|-----------------------------|\n");
	printf("\n");
}

int input()
{
	switch(toupper(getchar())) {
		case 'F': return PUSH_FRONT;
		case 'B': return PUSH_BACK;
		case 'T': return POP_FRONT;
		case 'K': return POP_BACK;
		case 'V': return REVERSE;
		case 'R': return REMOVE;
		case 'E': return EMPTY;
		case 'C': return CLEAR;
		case 'P': return PRINT;
		case 'Z': return SIZE;
		case 'S': return SORT;
		case 'H': return HELP;
		case 'Q': exit(0);
	}

	return -1;
}

