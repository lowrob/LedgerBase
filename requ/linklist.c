#include	<stdio.h>
#include	"linklist.h"

/* Constants.	*/
#define	LINKED_MAGIC	12345
#define	NONODE		(Node)0

/* Retrieval positioning.	*/
#define	FORWARD		1
#define	NONE		0
#define	BACKWARD	-1

/* Macros.	*/
#define	EMPTY_LIST	(list->first == Endnode)

/* Global Variables.	*/
static	Node	Endnode;

/*----------------------------------------------------------------------*/
/* 	The following routine creates a linked list data structure.  It */
/* takes the size of the object to be placed in the linked list as an	*/
/* argument.  It returns a linked list identifier to be used as an	*/
/* argument for the other linked list processing routines.  If the list	*/
/* can't be created the routine returns the identifier NOLIST.		*/
/*----------------------------------------------------------------------*/
List
list_make(object_size)

long	object_size;	/* Size of the objects in the linked list.	*/

{
	List	list;	/* Identifier of list that is created.	*/
static	List_node	endnode; /* Node at the end of the list.	*/

	/* Set up the self referencing Endnode structure.	*/
	Endnode = &endnode;
	Endnode->forward = Endnode;
	Endnode->backward = Endnode;

	/* Allocate the memory for the list.	*/
	list = (List)malloc(sizeof(Linked_list));

	/* If the allocation was not a failure,	initialize the contents
		of the linked list.	*/
	if (list != NOLIST)
	{
		list->magic = LINKED_MAGIC;
		list->size = object_size;
		list->direction = NONE;
		list->first = Endnode;
		list->current = Endnode;
		list->last = Endnode;
	}

	/* Return the linked list identifier.  (Value may be NOLIST).	*/
	return(list);
}

/*----------------------------------------------------------------------*/
/* 	The following routine frees a linked list data structure.  It 	*/
/* takes a linked list identifier as an argument and frees all data	*/
/* structures associated with that linked list.	 If the data structure	*/
/* is not a linked list, then NOTLIST is returned.  Otherwise NOERROR	*/
/* is returned.								*/
/*----------------------------------------------------------------------*/
long
list_kill(list)

List	list;	/* List idendifier of list to be destroyed.	*/

{
	/* Check if this is a linked list.	*/
	if (list->magic != LINKED_MAGIC)
	{
		return(NOTLIST);
	}

	/* While the linked list still contains nodes,	*/
	while ( ! EMPTY_LIST )
	{
		/* Free the last node in the list;	*/
		list_set(list,LAST_NODE);
		list_remove(list);
	}

	/* Free the list data structure.	*/
	free((char *)list);

	return(NOERROR);
}

/*----------------------------------------------------------------------*/
/* 	The following routine adds a new node to a linked list.  It 	*/
/* takes as arguments: the identifier of the linked list; a pointer to	*/
/* the object to be added to the linked list; and a flag indicating if	*/
/* the object is to be added before or after the last node retrieved	*/
/* with a list_get() routine call.  The object passed is copied into	*/
/* linked list, so its contents need not be preserved in the calling	*/
/* routine.  The routine returns a pointer to the object just placed in	*/
/* the linked list.  If an allocation error occurs, the routine returns	*/
/* a NULL pointer.							*/
/*----------------------------------------------------------------------*/
Object
list_add(list,object,position)

List	list;		/* List the object is to be added to.	*/
Object	object;		/* Pointer to object to be added to list.	*/
long	position;	/* BEFORE or AFTER switch for positioning.	*/

{
	long	i;
	Node	newnode;	/* New node to be added to the list.	*/

	/* Check if this is a linked list.	*/
	if (list->magic != LINKED_MAGIC)
	{
		return(NOOBJ);
	}

	/* Allocate the memory for the linked list node.	*/
	newnode = (Node)malloc(sizeof(List_node));
	if (newnode == NONODE)
	{
		return(NOOBJ);
	}

	/* Allocate the memory for the object.	*/
	newnode->object = (Object)malloc(list->size);
	if (newnode->object == NOOBJ)
	{
		return(NOOBJ);
	}

	/* Copy the object passed to the linked list.	*/
	for (i = 0; i < list->size; i++)
	{
		newnode->object[i] = object[i];
	}

	/* Add the node to the list in the correct position.	*/
	if (position == BEFORE)
	{
			
		newnode->forward = list->current;
		newnode->backward = list->current->backward;
		if (newnode->backward != Endnode)
		{
			newnode->backward->forward = newnode;
		}
		if (list->current != Endnode)
		{
			list->current->backward = newnode;
		}
		if (list->current == list->first)
		{
			list->first = newnode;
		}
	}
	if (position == AFTER)
	{
		newnode->backward = list->current;
		newnode->forward = list->current->forward;
		if (newnode->forward != Endnode)
		{
			newnode->forward->backward = newnode;
		}
		if (list->current != Endnode)
		{
			list->current->forward = newnode;
		}
		if (list->current == list->last)
		{
			list->last = newnode;
		}
	}

	/* This code handles the first node entered in a linked list.	*/
	if (list->first == Endnode)
	{
		list->first = newnode;
	}
	if (list->last == Endnode)
	{
		list->last = newnode;
	}
	if (list->current == Endnode)
	{
		list->current = newnode;
	}

	/* Return the object in the linked list.	*/
	return(newnode->object);
}
	
/*----------------------------------------------------------------------*/
/* 	The following routine removes a node from a linked list.  It	*/
/* takes a linked list identifier as an argument and removes the last	*/
/* node retrieved with a list_get() routine call.  If the data structure*/
/* is not a linked list, then NOTLIST is returned.  Otherwise NOERROR	*/
/* is returned.								*/
/*----------------------------------------------------------------------*/
long
list_remove(list)

List	list;	/* List that the node is to be removed from.	*/

{
	Node	oldnode;	/* Node that is to be removed.	*/

	/* Check if this is a linked list.	*/
	if (list->magic != LINKED_MAGIC)
	{
		return(NOTLIST);
	}

	/* Check for an empty list.	*/
	if (EMPTY_LIST)
	{
		return(END_OF_LIST);
	}

	/* Set the node to be deleted.	*/
	oldnode = list->current;
	
	/* Adjust the links to remove the node from the list.	*/
	if (oldnode->forward != Endnode)
	{
		oldnode->forward->backward = oldnode->backward;
	}
	if (oldnode->backward != Endnode)
	{
		oldnode->backward->forward = oldnode->forward;
	}

	/* Fix the linked list pointers.	*/
	if (list->first == oldnode)
	{
		list->first = oldnode->forward;
		list->current = list->first;
	}
	if (list->last == oldnode)
	{
		list->last = oldnode->backward;
		list->current = list->last;
	}
	if (list->current == oldnode)
	{
		if (list->direction == BACKWARD)
		{
			list->current == oldnode->forward;
		}
		else
		{
			list->current == oldnode->backward;
		}
	}

	/* Free the object.	*/
	free(oldnode->object);

	/* Free the node.	*/
	free(oldnode);

	return(NOERROR);
}

/*----------------------------------------------------------------------*/
/* 	The following routine gets an object from the linked list. It 	*/
/* takes as arguments: the identifier of the linked list; a flag	*/
/* indicating which object is to be retrieved.  Valid flag values are:	*/
/*	CURRENT_NODE:	same object that was retrieved last call.	*/
/*	NEXT_NODE:	next object in linked list.			*/
/*	PREVIOUS_NODE:	previous object in linked list.			*/
/*	FIRST_NODE:	first object in linked list.			*/
/*	LAST_NODE:	last object in linked list.			*/
/* The routine returns the requested object.				*/
/*----------------------------------------------------------------------*/
Object
list_get(list,position)

List	list;		/* List the object is to be retrieved from.	*/
long	position;	/* Switch for positioning.	*/

{
	long	retcode;

	/* Set the position in the list.	*/
	retcode = list_set(list,position);

	/* Check for end of linked list.	*/
	if (retcode == END_OF_LIST)
	{
		return(NOOBJ);
	}

	/* Return the object.	*/
	return(list->current->object);
}

/*----------------------------------------------------------------------*/
/* 	The following routine sets the current position in the linked	*/
/* It takes as arguments: the identifier of the linked list; a flag	*/
/* indicating which position is to be set.  Valid flag values are:	*/
/*	CURRENT_NODE:	same object that was retrieved last call.	*/
/*	NEXT_NODE:	next object in linked list.			*/
/*	PREVIOUS_NODE:	previous object in linked list.			*/
/*	FIRST_NODE:	first object in linked list.			*/
/*	LAST_NODE:	last object in linked list.			*/
/*----------------------------------------------------------------------*/
long
list_set(list,position)

List	list;		/* List to change current position in.	*/
long	position;	/* Switch for positioning.	*/

{
	/* Set the position in the list.	*/
	switch (position)
	{
	case CURRENT_NODE:
		list->direction = NONE;
		break;
	case NEXT_NODE:
		list->direction = FORWARD;
		if (list->current->forward ==  Endnode)
		{
			return(END_OF_LIST);
		}
		list->current = list->current->forward;
		break;
	case PREVIOUS_NODE:
		list->direction = BACKWARD;
		if (list->current->backward ==  Endnode)
		{
			return(END_OF_LIST);
		}
		list->current = list->current->backward;
		break;
	case FIRST_NODE:
		list->direction = NONE;
		if (EMPTY_LIST)
		{
			return(END_OF_LIST);
		}
		list->current = list->first;
		break;
	case LAST_NODE:
		list->direction = NONE;
		if (EMPTY_LIST)
		{
			return(END_OF_LIST);
		}
		list->current = list->last;
		break;
	}

	return(NOERROR);
}
