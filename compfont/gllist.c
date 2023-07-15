/*****************************************************************************
 * gllist.c                                                                  *
 *                                                                           *
 *  Simple generic linked list implementation.                               *
 *  Copyright (C) 2023 Alexander Taylor                                      *
 *                                                                           *
 *  This code is placed in the public domain.                                *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "gllist.h"


PGLINKEDLIST gl_list_new( void )
{
    PGLINKEDLIST pList = (PGLINKEDLIST) malloc( sizeof(GLINKEDLIST) );
    if ( !pList ) return NULL;

    pList->pHead = NULL;
    pList->size = 0;

    return pList;
}


unsigned long gl_list_push( PGLINKEDLIST pList, void* pData )
{
    PGLISTNODE pNode;

    pNode = (PGLISTNODE) malloc( sizeof(GLISTNODE) );
    if ( !pNode ) return 0;

    pNode->pData = pData;
    pNode->pNext = pList->pHead;
    pList->pHead = pNode;
    pList->size++;

    return pList->size;
}


void * gl_list_pop( PGLINKEDLIST pList )
{
    PGLISTNODE pNode;
    void *pData;

    if ( !pList->size ) return NULL;

    pNode = pList->pHead;
    pData = pNode->pData;
    pList->pHead = pNode->pNext;
    free( pNode );
    pList->size--;

    return pData;
}


unsigned long gl_list_insert( PGLINKEDLIST pList, void* pData, unsigned long pos )
{
    PGLISTNODE pNode, pNew;
    unsigned long i;

    if ( pos > pList->size )
        return 0;
    if ( pos == 0 )
        return gl_list_push( pList, pData );

    pNode = pList->pHead;
    for ( i = 1; i < pos; i++ )
        pNode = pNode->pNext;
    pNew = (PGLISTNODE) malloc( sizeof(GLISTNODE) );
    if ( !pNew ) return 0;

    pNew->pData = pData;
    pNew->pNext = pNode->pNext;
    pNode->pNext = pNew;
    pList->size++;

    return pList->size;
}


unsigned long gl_list_append( PGLINKEDLIST pList, void *pData )
{
    return gl_list_insert( pList, pData, pList->size );
}


void * gl_list_at( PGLINKEDLIST pList, unsigned long pos )
{
    PGLISTNODE pNode;
    unsigned long i;

    if (( !pList->size ) || ( pos >= pList->size ))
        return NULL;

    pNode = pList->pHead;
    for ( i = 0; i < pos; i++ )
        pNode = pNode->pNext;

    return pNode->pData;
}


unsigned long gl_list_delete( PGLINKEDLIST pList, unsigned long pos )
{
    PGLISTNODE pNode,
               pPrev;
    unsigned long i;

    if (( !pList->size ) || ( pos >= pList->size ))
        return 0;
    if ( pos == 0 ) {
        return ( gl_list_pop( pList ) == NULL? 0: 1 );
    }

    pPrev = pList->pHead;
    for ( i = 0; (i + 1) < pos; i++ )
        pPrev = pPrev->pNext;
    pNode = pPrev->pNext;

    pPrev->pNext = pNode->pNext;
    free( pNode );
    pList->size--;

    return 1;
}


void gl_list_free( PGLINKEDLIST pList )
{
    PGLISTNODE pNode = pList->pHead;

    while ( pNode ) {
        PGLISTNODE pNext = pNode->pNext;
        free( pNode );
        pNode = pNext;
    }

    free( pList );
}


