#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>

/**
 * Given the input: 23 d2 b8 19, read in little-endian format:
 *
 * 00100011 11010010 10111000 00011001
 *
 * 87654321 65432109 43210987 21098765
 *          1111111  22222111 33322222
 *
 * So the unpacked output is:
 * (0 0010011) (00 1101001) (001 101110) (xxxx 00011)
 */
static int next_bit( int in )
{
  static unsigned char byte;
  static unsigned int mask = 0x100;
  int bit;

  if ( mask == 0x100 )
  {
    mask = 0x01;
    if ( !read( in, &byte, 1 ) )
    {
      return -1;
    }
  }

  bit = ( byte & mask ) ? 1 : 0;
  mask <<= 1;

  return bit;
}

int main( int argc, char *argv[] )
{
  int input;
  int maxbits;
  int code_length = 9;
  int i;
  unsigned char magic[ 2 ];
  unsigned char header;
  int code, prev = -1;
  unsigned char **dictionary;
  int dictionary_ind;

  if ( argc < 2 )
  {
    fprintf( stderr, "Usage: %s <input file>\n", argv[ 0 ] );
    exit( 0 );
  }

  input = open( argv[ 1 ], O_RDONLY );

  if ( input == -1 )
  {
    fprintf( stderr, "Error opening file '%s'", argv[ 1 ] );
    perror( ":" );
  }

  // LZW starts with two bytes of "magic"
  read( input, &magic, 2 );

  if ( ( magic[ 0 ] != 0x1f ) ||
       ( magic[ 1 ] != 0x9d ) )
  {
    fprintf( stderr, "This is not a compressed file." );
    exit( 0 );
  }

  // followed by a single byte of header info
  read( input, &header, 1 );
  maxbits = header & 0x1f;

  // Create a dictionary large enough to hold "code_length" entries.
  // Once the dictionary overflows, code_length increases
  dictionary = ( unsigned char ** ) malloc( sizeof( unsigned char * ) *
    ( 1 << code_length ) );
  memset( dictionary, sizeof( unsigned char * ) * ( 1 << code_length ), 0x0 );

  // Initialize the first 256 entries of the dictionary with their
  // indices.  The rest of the entries will be built up dynamically.
  for ( dictionary_ind = 0; dictionary_ind < 256; dictionary_ind++ )
  {
    dictionary[ dictionary_ind ] = ( unsigned char * ) malloc( 2 );
    sprintf((char *)dictionary[ dictionary_ind ], "%c", dictionary_ind );
  }

  // 256 is the special "clear" code; don't give it an entry here
  dictionary_ind++;

  // followed by the compressed data itself.  Read the data 9 bits
  // at a time.  .Z formatted data always starts with 9 bit codes.
  {
    int done = 0;
    while ( !done )
    {
      code = 0x0;
      for ( i = 0; i < code_length; i++ )
      {
        int bit = next_bit( input );
        if ( bit == -1 )
        {
          done = 1;
          break;
        }
        code = code | ( bit << i );
      }

      if ( code == 256 )
      {
        code_length = 9;
        dictionary = ( unsigned char ** ) realloc( dictionary,
          sizeof( unsigned char * ) * ( 1 << code_length ) );
      }

      // Update the dictionary with this character plus the _entry_
      // (character or string) that came before it
      if ( prev > -1 )
      {
        if ( code == dictionary_ind )
        {
          dictionary[ dictionary_ind ] = ( unsigned char * ) malloc(
            strlen((const char *)dictionary[ prev ] ) + 2 );
          sprintf((char *)dictionary[ dictionary_ind ], "%s%c", dictionary[ prev ],
            dictionary[ prev ][ 0 ] );
        }
        else
        {
          dictionary[ dictionary_ind ] = ( unsigned char * ) malloc(
            strlen((const char *)dictionary[ prev ] ) + 2 );
          sprintf((char *)dictionary[ dictionary_ind ], "%s%c", dictionary[ prev ],
            dictionary[ code ][ 0 ] );
        }

        dictionary_ind++;

        if ( dictionary_ind == ( 1 << code_length ) )
        {
          unsigned char **new_dictionary;

          //printf( "End of dictionary.\n" );
          code_length++;

          dictionary = ( unsigned char ** ) realloc( dictionary,
            sizeof( unsigned char * ) * ( 1 << code_length ) );
        }
      }

      if ( dictionary[ code ] == NULL )
      {
        printf( "Error in dictionary (no entry for code %d).\n", code );
        exit( 0 );
      }
      printf( "%s", dictionary[ code ] );

      prev = code;
    }
  }

  printf( "\n" );

  close( input );
}