/*****************************************************************************
 * gllist.h                                                                  *
 *                                                                           *
 *  Simple generic linked list implementation.                               *
 *  Copyright (C) 2023 Alexander Taylor                                      *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

// Basic list type
typedef struct _gl_list_node {
    void                 *pData;
    struct _gl_list_node *pNext;
} GLISTNODE, *PGLISTNODE;


// List node type
typedef struct _gl_list_type {
    PGLISTNODE    pHead;
    unsigned long size;
} GLINKEDLIST, *PGLINKEDLIST;


// FUNCTIONS

// Create a new, empty linked list.
// Returns a pointer to the list structure.
PGLINKEDLIST gl_list_new( void );

// Push a new item onto the front of the list.
// Returns the new list size, or 0 on error.
unsigned long gl_list_push( PGLINKEDLIST pList, void* pData );

// Pop one item off the front of the list.
// Returns the item data, or NULL on error.
void * gl_list_pop( PGLINKEDLIST pList );

// Insert a new item at a specific (0-indexed) position in the list.
// Returns the new list size, or 0 on error.
unsigned long gl_list_insert( PGLINKEDLIST pList, void* pData, unsigned long pos );

// Append a new item at the end of the list.
// 1 on success or 0 on error.
unsigned long gl_list_append( PGLINKEDLIST pList, void* pData );

// Retrieve the item data at a specific (0-indexed) position in the list.
// Returns the item (as pointer), or NULL on error.
void * gl_list_at( PGLINKEDLIST pList, unsigned long pos );

// Delete the item at a specific (0-indexed) position in the list.
// Returns the new list size, or 0 on error.
unsigned long gl_list_delete( PGLINKEDLIST pList, unsigned long pos );

// Delete the list and all its nodes.  The individual data objects pointed
// to by each node are not freed, so the calling program must take care not
// to leave them orphaned.
void gl_list_free( PGLINKEDLIST pList );

