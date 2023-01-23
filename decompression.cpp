#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>

// считывает биты, записанные в прямом порядке (little-endian)
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
    fprintf( stderr, "Ошибка при открытии файла '%s'", argv[ 1 ] );
    perror( ":" );
  }

  //LZW файл начинает с двух "магических" байтов 0x1F9D
  read( input, &magic, 2 );

  if ( ( magic[ 0 ] != 0x1f ) ||
       ( magic[ 1 ] != 0x9d ) )
  {
    fprintf( stderr, "Это не сжатый файл." );
    exit( 0 );
  }

  // за которым следует один байт информации заголовка
  read( input, &header, 1 );
  maxbits = header & 0x1f;

  // Создается словарь, достаточно большой, чтобы содержать записи длины code_length
  // Когда словарь переполняется, длина кода code_length увеличивается
  dictionary = ( unsigned char ** ) malloc( sizeof( unsigned char * ) *
    ( 1 << code_length ) );
  memset( dictionary, sizeof( unsigned char * ) * ( 1 << code_length ), 0x0 );

  // Инициализируются первые 256 записей словаря с их индексами
  for ( dictionary_ind = 0; dictionary_ind < 256; dictionary_ind++ )
  {
    dictionary[ dictionary_ind ] = ( unsigned char * ) malloc( 2 );
    sprintf((char *)dictionary[ dictionary_ind ], "%c", dictionary_ind );
  }

  // 256 - специальный пустой код
  dictionary_ind++;

  // за которым следуют сами сжатые данные. Чтение данных происходит по 9 бит за раз
  // Данные в формате .Z всегда начинаются с 9-битных кодов
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

      // В словарь добавляется этот символ плюс _entry_ (символ или строка), который был до него.
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

          code_length++;

          dictionary = ( unsigned char ** ) realloc( dictionary,
            sizeof( unsigned char * ) * ( 1 << code_length ) );
        }
      }

      if ( dictionary[ code ] == NULL )
      {
        printf( "Ошибка словаря (нет строки для кода %d).\n", code );
        exit( 0 );
      }
      printf( "%s", dictionary[ code ] );

      prev = code;
    }
  }

  printf( "\n" );

  close( input );
}