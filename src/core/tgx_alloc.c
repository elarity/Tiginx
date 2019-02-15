#include "tgx_alloc.h"

void * tgx_alloc( size_t length ) {
  return malloc( length );  
}

void tgx_free( void * pointer ) {
  free( pointer );
}
