#include "myMalloc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

/** Representation of a free memory hole. */
typedef struct HoleTag {
  // Size of this hole (not counting the size field itself).
  size_t size;

  // This takes zero bytes in the struct, but it lets us treat the
  // memory right after size as an array of bytes, sometimes useful.
  char mem[ 0 ];

  // Pointer to the next block if this one is free.
  struct HoleTag *next;
} Hole;

/** Pointer to the first free block. */
Hole *freeList = NULL;

// A debugging function to report all the blocks of memory.
void reportFreeList() {
  for ( Hole *h = freeList; h; h = h->next )
    printf( "%14p : %zd\n", h, h->size );
}

// Get the first block containing at least size bytes.  Unlink it
// from the free list and return it.  Return NULL if there's no
// block big enough.
static Hole *getBlock( size_t size ) {
  
  // You get to write this whole function.
  // ...
  
}

// Insert the given block back into the free list, in the right
// location to keep the free list ordered by address.  Merge the block
// with its predecessor and/or successor blocks if it's consecutive
// with them.
static void returnBlock( Hole *h ) {
  // Find the predecessor of this block in the free list.
  Hole *pred = NULL;
  for ( Hole *node = freeList; node && node < h; node = node->next )
    pred = node;

  // Link this node back into the linked list.
  if ( pred ) {
    h->next = pred->next;
    pred->next = h;
  } else {
    // No predecessor, must be the first node on
    // the list.
    h->next = freeList;
    freeList = h;
  }

  // Should this node absorb its successor?
  if ( h->next && h->mem + h->size == (char *) h->next ) {
    // Yes.  Absorb the successor block into this one.  Block h
    // gets larger, by the size of its successor block and memory used
    // for its successor's size field.
    h->size += h->next->size + sizeof( size_t );

    // And the successor node no longer needs to show up as a separate
    // node in the linked list, skip over it.
    h->next = h->next->next;
  }

  // Does block h have a predecessor?
  if ( pred ) {
    // Absorb h into its predecessor if there's no gap
    // between them.

    // You get to write the is part.
    // ...

  }
}


void *malloc( size_t size ) {
  // We can't give out a block smaller than a pointer, otherwise,
  // we won't have enough room to link the block into the free list when
  // it's returned.
  if ( size < sizeof( Hole * ) )
    size = sizeof( Hole * );

  // Round up to the nearest multiple of 4.  This is an effort to
  // satisfy any alignment constraints the host may have.  I'm not sure
  // if this is necessary (or sufficient) on the common platform.
  size = ( size + 3 ) / 4 * 4;

  // Get a block from the free memory list.
  Hole *h = getBlock( size );

  // If we didn't find a large enough block, ask the OS for some more.
  if ( h == NULL ) {
    // How much do we need along with the size field itself.
    size_t req = size + sizeof( size_t );

    // Round up to a multiple of the page size, since that's what
    // we're going to get anyway.
    size_t psize = sysconf(_SC_PAGESIZE);
    req = ( req + psize - 1 ) / psize * psize;

    // Get the block and fill in its size field.
    h = (Hole *)mmap( NULL, req, PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 );
    if ( h == MAP_FAILED )
      return NULL;
    h->size = req - sizeof( size_t );
  }
  
  // This block may be larger than what's needed.  If it's large
  // enough, split off anything extra and give it back to the free list.
  if ( size + sizeof( Hole ) <= h->size ) {
    // Where would the next block start.
    Hole *h2 = (Hole *)( h->mem + size );

    // Split the available memory between the blocks.
    h2->size = h->size - size - sizeof( size_t );
    h->size = size;

    // Put the left-over block back on the free list.
    returnBlock( h2 );
  }

  // Return the part of this block right after the size field.
  return &( h->mem );
}

void free( void *ptr ) {
  // The documentation says it does nothing if you free a null pointer,
  // so we had better do the same.
  if ( ptr == NULL )
    return;
 
  // From the pointer the caller gave us, back up a few bytes to get a
  // pointer to the start of the hole it's part of.
  Hole *h = (Hole *)( (char *) ptr - sizeof( size_t ) );

  // Put this block back on the free list.
  returnBlock( h );
}

// Dumb implementations of calloc and realloc, just based on
// malloc/free.

void *calloc( size_t count, size_t size ) {
  // How much memory do we need?
  size_t n = count * size;

  // Try to get it from malloc.
  char *p = malloc( n );

  // Zero the block before returning it.
  if ( p )
    for ( size_t i = 0; i < n; i++ )
      p[ i ] = 0;

  return p;
}

// Realloc has a lot of special behaviors.  I hope I got them all.
void *realloc( void *ptr, size_t size ) {
  if ( ptr == NULL )
    return malloc( size );

  if ( size == 0 ) {
    free( ptr );
    return NULL;
  }

  // Try to get the new block.
  char *p2 = malloc( size );
  if ( p2 == NULL )
    return NULL;

  // Figure out the size of the previous block.
  Hole *h = (Hole *)( (char *) ptr - sizeof( size_t ) );

  // Copy everything over.
  size_t sz = h->size < size ? h->size : size;
  char *p1 = ptr;
  for ( int i = 0; i < sz; i++ )
    p2[ i ] = p1[ i ];

  // Free the old block and return the new one.
  free( ptr );
  return p2;
}

