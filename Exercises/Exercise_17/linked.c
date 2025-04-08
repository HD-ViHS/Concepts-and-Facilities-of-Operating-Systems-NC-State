#include <stdio.h>
#include <stdlib.h>

// Make a short name for a Node.
typedef struct Node Node;

// Node to build our list from.
struct Node {
  // Value in our list
  int val;
  
  // Pointer to the next node.
  Node *next;
};

// Representation for a list, with a head and a tail pointer.
typedef struct {
  // Head pointer for the list.
  Node *head;
  
  // Pointer to the last node in the list, or null if the list
  // is empty.
  Node *tail;
} List;

// Recursive quicksort.
void sort( List *list ) {

  // 0-element list
  if(list->tail == NULL)
    return;

  // Single-element list
  if(list->head->next == NULL)
    return;
  
  int partition = list->head->val;
  Node *pivot = list->head;

  List lesser = {NULL, NULL};
  List greater = {NULL, NULL};
  Node **n = &(list->head->next);

  while ( *n != NULL ) {
    if((*n)->val < partition) {
      if (lesser.head == NULL) {
	lesser.head = *n;
	lesser.tail = *n;
	Node *temp = *n;
	*n = (*n)->next;
	temp->next = NULL;
      } else {
	lesser.tail->next = *n;
	lesser.tail = *n;
	Node *temp = *n;
	*n = (*n)->next;
	temp->next = NULL;
      }
    } else {
      if (greater.head == NULL) {
	greater.head = *n;
	greater.tail = *n;
	Node *temp = *n;
	*n = (*n)->next;
	temp->next = NULL;
      } else {
	greater.tail->next = *n;
	greater.tail = *n;
	Node *temp = *n;
	*n = (*n)->next;
	temp->next = NULL;
      }
    }
  }

  sort(&lesser);
  sort(&greater);

  lesser.tail->next = pivot;
  pivot->next = greater.head;

  list->head = lesser.head;
  list->tail = greater.tail;
  
}

int main( int argc, char *argv[] )
{
  int n = 10;
  if ( argc >= 2 )
    if ( argc > 2 || sscanf( argv[ 1 ], "%d", &n ) != 1 ||
         n < 0 ) {
      fprintf( stderr, "usage: array <n>\n" );
      exit( EXIT_FAILURE );
    }
  
  // Representaton for the list.
  List list = { NULL, NULL };

  // Pointer to the null pointer at the end of our list, so we can grow
  // the list front-to-back;
  for ( int i = 0; i < n; i++ ) {
    // Make a node containing a random value.
    Node *n = (Node *) malloc( sizeof( struct Node ) );
    *n = (Node) { rand(), NULL };

    // Link it in at the tail end of the list.
    if ( list.tail )
      list.tail->next = n;
    else
      list.head = n;
    list.tail = n;
  }

  sort( &list );

  // Print the sorted items.
  for ( Node *n = list.head; n; n = n->next )
    printf( "%d\n", n->val );

  // Free memory for the list.
  while ( list.head ) {
    Node *n = list.head;
    list.head = n->next;
    free( n );
  }

  return 0;
}
