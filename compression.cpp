#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>

#define BUFSZ 256

static void write_bits( int code, int out, int code_length ) {
    static unsigned char buf = 0x0;
    static int bufpos = 0;

    int mask_pos = 0;

    while ( code_length-- ) {
        // right shift by bytes
        buf |= ( ( code & ( 1 << mask_pos ) ) ? 1: 0 ) << bufpos;
        mask_pos++;
        if ( bufpos++ == 7 ) {
            write( out, &buf, 1 );
            buf = 0x0;
            bufpos = 0;
        }
    }
}

int main( int argc, char *argv[ ] ) {
    char buf[ BUFSZ * 2 ];
    int buflen;
    int in, out;
    char *outfilename;
    char **dictionary;
    int dictionary_ind;
    int code_length = 9;
    int i;
    int outc;
    char *inp = buf;
    int prev = -1;

    if ( argc < 2 ) {
        fprintf( stderr, "Usage: %s <file>\n", argv[ 0 ] );
        exit( 0 );
    }

    if ( ( in = open( argv[ 1 ], O_RDONLY ) ) == -1 ) {
        fprintf( stderr, "Unable to open input file '%s'", argv[ 1 ] );
        perror( ": " );
        exit( 0 );
    }

    outfilename = ( char * ) malloc( strlen( argv[ 1 ] ) + 3 );
    sprintf( outfilename, "%s.Z", argv[ 1 ] );

    if ( ( out = open( outfilename, O_WRONLY | O_CREAT | O_TRUNC, 0644 ) ) == -1 ) {
        fprintf( stderr, "Unable to open output file '%s'", outfilename );
        perror( ": " );
        exit( 0 );
    }

    // Write out magic header 1f 9d
    outc = 0x1f;
    write( out, &outc, 1 );
    outc = 0x9d;
    write( out, &outc, 1 );
    outc = 0x90; // bit 7 must be set for compatibility
    write( out, &outc, 1 );

    // Header done; start compressing
    // Create an initial dictionary with 2**9=512 entries. When this
    // dictionary fills up, it will be expanded and the code size will
    // increase.
    dictionary = ( char ** ) malloc( sizeof( char * ) * ( 1 << code_length ) );
    memset( dictionary, 0x0, sizeof( char * ) * ( 1 << code_length ) );

    // pre-initialize the first 255 entries with their own values
    for ( dictionary_ind = 0; dictionary_ind < 256; dictionary_ind++ ) {
        dictionary[ dictionary_ind ] = ( char * ) malloc( 2 );
        sprintf( dictionary[ dictionary_ind ], "%c", dictionary_ind );
    }

    // Skip 256 (this is the clear code)
    dictionary_ind++;

    buflen = read( in, buf, BUFSZ * 2 );

    while ( inp <= ( buf + buflen ) ) {
        // Advance buffer
        if ( ( inp - buf ) > BUFSZ ) {
            memcpy( buf, inp, buflen - ( inp - buf ) );
            buflen -= ( inp - buf );
            buflen += read( in, buf + buflen, BUFSZ + ( BUFSZ - buflen ) );
            inp = buf;
            }

        // Seach the dictionary for the longest match that matches
        // inp
        for ( i = dictionary_ind - 1; i; i-- ) {
            if ( dictionary[ i ] != NULL ) {
                if ( !strncmp( dictionary[ i ], inp,
                strlen( dictionary[ i ] ) ) ) {
                    outc = i;
                    break;
                    }
                }
            }

        write_bits( outc, out, code_length );
        inp += strlen( dictionary[ outc ] );

        // Add this match, along with the next character, to the dictionary
        dictionary[ dictionary_ind ] = (char *)malloc( strlen( dictionary[ outc ] ) + 2 );
        sprintf( dictionary[ dictionary_ind ], "%s%c", dictionary[ outc ], *inp );
        dictionary_ind++;

        // Expand the dictionary if necessary
        if ( dictionary_ind == ( 1 << code_length ) ) {
            code_length++;
            dictionary = (char **)realloc( dictionary, 
            sizeof( unsigned char ** ) * ( 1 << code_length ) );
            }
        }
}