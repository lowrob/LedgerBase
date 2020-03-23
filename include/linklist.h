/* Node add positioning.	*/
#define	BEFORE	-1
#define	AFTER	1

/* Node retrieval positioning.	*/
#define	CURRENT_NODE	0
#define	PREVIOUS_NODE	-1
#define	NEXT_NODE	1
#define	LAST_NODE	-2
#define	FIRST_NODE	2

/* Return codes.	*/
#define	NOERROR		0
#define	NOTLIST		-1
#define	END_OF_LIST	-1

typedef	char	*Object;

typedef	struct	list_node	*Node;

typedef struct list_node
{
	Node	forward;	/* Pointer to next node. */
	Node	backward;	/* Pointer to previous node. */
	Object	object;		/* Pointer to the object in the linked list. */
} List_node;

typedef	struct	linked_list	*List;

typedef struct linked_list
{
	long	magic;		/* Magic number for linked lists.	*/
	long	size;		/* Size of the object in the linked list. */
	long	direction;	/* Last direction of list_get().	*/
	Node	first;		/* Pointer to first node.	*/
	Node	current;	/* Pointer to current node.	*/
	Node	last;		/* Pointer to last node.	*/
} Linked_list;

/* moved to here so they would no what (List) and (Object) consist of */
#define	NOLIST	(List)0
#define	NOOBJ	(Object)0

List	list_make();
long	list_kill();
Object	list_add();
long	list_remove();
Object	list_get();
long	list_set();
