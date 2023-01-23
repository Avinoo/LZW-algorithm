#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>

#define BUFSZ 256

// Записывает биты кодов в прямом ( little-endian) порядке

static void write_bits( int code, int out, int code_length ) {
    static unsigned char buf = 0x0;
    static int bufpos = 0;

    int mask_pos = 0;

    while ( code_length-- ) {
        buf |= ( ( code & ( 1 << mask_pos ) ) ? 1: 0 ) << bufpos; // побитовый сдвиг вправо
        mask_pos++;
        if ( bufpos++ == 7 ) {
            write( out, &buf, 1 );
            buf = 0x0;
            bufpos = 0;
        }
    }
}

int main( int argc, char *argv[ ] ) {

    char buf[BUFSZ * 2];
    int buflen;
    int in, out;
    char *out_file_name;
    char **dictionary;
    int dictionary_ind;
    int code_length = 9;
    int out_code;
    char *inp = buf;
    int prev = -1;

    if (argc < 2) {
        fprintf( stderr, "Usage: %s <file>\n", argv[ 0 ]);
        exit(0);
    }

    if (( in = open( argv[ 1 ], O_RDONLY ) ) == -1 ) {
        fprintf( stderr, "Не получилось открыть входной файл '%s'", argv[ 1 ]);
        perror(": ");
        exit(0);
    }

    out_file_name = ( char * ) malloc( strlen( argv[ 1 ] ) + 3 );
    sprintf( out_file_name, "%s.Z", argv[ 1 ] );

    if ( ( out = open( out_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644 ) ) == -1 ) {
        fprintf( stderr, "Не получилось открыть выходной файл '%s'", out_file_name );
        perror( ": " );
        exit( 0 );
    }

    //заголовок
    out_code = 0x1f;                //"магические" числа 0x1F9D- сигнатура LZW файлов
    write( out, &out_code, 1 ); 
    out_code = 0x9d;
    write( out, &out_code, 1 );
    out_code = 0x90;                // сообщает длину самого больного кода
    write( out, &out_code, 1 );

    // начало сжатия
    //инициализация словаря с 2^9=512 строчками
    dictionary = ( char ** ) malloc( sizeof( char * ) * ( 1 << code_length ) ); 
    memset( dictionary, 0x0, sizeof( char * ) * ( 1 << code_length ) );

    // инициализация первых 256 (0- 255) односимвольных строк словаря
    for ( dictionary_ind = 0; dictionary_ind < 256; dictionary_ind++ ) { 
        dictionary[ dictionary_ind ] = ( char * ) malloc( 2 );           
        sprintf( dictionary[ dictionary_ind ], "%c", dictionary_ind );
    }

    dictionary_ind++; // 256 строчка остается пустой

    buflen = read( in, buf, BUFSZ * 2 );

    while ( inp <= ( buf + buflen ) ) {
        // предварительный буффер
        if ( ( inp - buf ) > BUFSZ ) {
            memcpy( buf, inp, buflen - ( inp - buf ) );
            buflen -= ( inp - buf );
            buflen += read( in, buf + buflen, BUFSZ + ( BUFSZ - buflen ) );
            inp = buf;
            }
        //поиск самого длинного совпадения в словаре
        for (int i = dictionary_ind - 1; i; i-- ) {    
            if ( dictionary[ i ] != NULL ) {
                if ( !strncmp( dictionary[ i ], inp,
                strlen( dictionary[ i ] ) ) ) {
                    out_code = i;
                    break;
                    }
                }
            }

        write_bits( out_code, out, code_length );
        inp += strlen( dictionary[ out_code ] );

        // это совпадение + следующий символ добавляются в словарь
        dictionary[ dictionary_ind ] = (char *)malloc( strlen( dictionary[ out_code ] ) + 2 );
        sprintf( dictionary[ dictionary_ind ], "%s%c", dictionary[ out_code ], *inp );
        dictionary_ind++;

        // расширить словарь, если необходимо
        if ( dictionary_ind == ( 1 << code_length ) ) {
            code_length++;
            dictionary = (char **)realloc( dictionary, 
            sizeof( unsigned char ** ) * ( 1 << code_length ) );
            }
        }
}