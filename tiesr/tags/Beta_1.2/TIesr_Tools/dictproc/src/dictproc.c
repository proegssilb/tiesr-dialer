/*=======================================================================
 dictproc.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


  This program is used to compress a dictionary file into a binary
  file that is compatible with the TIesrDict API.  The compressed
  binary dictionary is delta encoded based upon the default
  pronunciation supplied by an API that can develop a default
  pronunciation from a word spelling.

  The program originally used a rule-based utility to generate
  the default pronunciation, which was specific to English.  The
  utility included some text-normalization capabilities. This
  rule-based system was derived from the public domain source code
  ENGLISH TO PHONEME TRANSLATION written by John A. Wasser, 4/15/85.

  Later, the capability was added to optionally use the TIesrDT
  API, which is the preferred method. TIesrDT is based on
  decision tree generation of phonetic pronunciations, where
  there is a decision tree for each letter of a spelled word.
  This method requires more space to store the decision trees,
  but provides more accurate pronunciations.

======================================================================*/
// System includes
#include <sys/types.h>
#include <dirent.h>

/* Now use TIesrDT rather than dtmakeup */
/* #include "dtmakeup_user.h" */
#include <TIesrDT_User.h>

#include   "dictproc.h"


/* Now done via input argument */
/* #define     MAP        1 */       /* 0: no map, 1: map */

extern int  cur_entry;
extern char *entry[];
extern char *entry_pron[];

extern char *phone[];
extern int n_phone;                /* 46 + "bbb" = 47 */

extern char *pron_dict;
extern char *pron_def;
extern int  len_dict;
extern int  len_def;
extern int  gerrc, gsubc, gdelc, ginsc;
extern char g_match_pattern[];
extern int  g_match_pattern_cnt;

static char prev_word[ MAX_STR ];
static char save_word[ MAX_STR ];

static int little_endian;

/* ---------------------------------------------------------------------------
   output a 32-bit, big endian or little endian
   ---------------------------------------------------------------------- */

void
output_int32(FILE *fp, int i32)
{
  if ( little_endian ) {

    fputc( 0xff & i32, fp);
    i32 = i32 >> 8;
    fputc( 0xff & i32, fp);
    i32 = i32 >> 8;
    fputc( 0xff & i32, fp);
    i32 = i32 >> 8;
    fputc( 0xff & i32, fp);

  } else {

    fputc( ( i32 >> 24 ) & 0xff, fp );
    fputc( ( i32 >> 16 ) & 0xff, fp );
    fputc( ( i32 >> 8 ) & 0xff, fp );
    fputc( i32 & 0xff, fp );
  }
}

/* ---------------------------------------------------------------------------
   word: entry word
   ptr1: correct phone sequence (from the input dictionary)
   ptr2: phone sequence from rule (default)
   ---------------------------------------------------------------------- */

void process(char *word, char *ptr1, char *ptr2)
{
  char  pron1[ MAX_PRON ], pron2[ MAX_PRON ], pron_err[ MAX_PRON ];
  char  match_pattern[ MAX_PRON ];

  /* convert from ASCII string to byte phone index.
   pron1 is dictionary pronunciation, pron2 is default. */
  pron2bin(ptr1, pron1);
  pron2bin(ptr2, pron2);

  /* Prepare for align. Initialize global substitution, deletion, and
     insertion counts to a large value.  Alignment matching finds the
     pattern of insertions, substitutions and deletions that minimizes
     the sum of the three types of misalignment errors. */

  gsubc = gdelc = ginsc = LARGE;

  gerrc = gsubc + gdelc + ginsc;

  g_match_pattern_cnt = 0;

  /* Length of phone string for each pronunciation.  len_def may be zero. */
  len_dict = (int) pron1[0];
  len_def = (int) pron2[0];

  /* Pointers to pronunciations.  */
  pron_dict = &pron1[1];
  pron_def = &pron2[1];

  /* Align pronunciations. */
  match(0, 0, 0, 0, 0, match_pattern, 0);

  /* error encode, convert to pron error */
  pron2err( pron_err );

  /* delta encode and store */
  if ( pron_err[0] == '\0' ) {           /* same as default pron */

    if ( equal_str(word, prev_word) ) {  /* prev nondef exist, need to save */

      encode_entry( word );
      encode_entry_pron( pron_err );
      cur_entry++;

    } else {

      copy_str( save_word, word );       /* tag def, may need to save later */
    }

  } else {                               /* different from default pron */

    if ( equal_str(word, save_word) ) {  /* prev def need to save */

      encode_entry( save_word );
      pron2[0] = '\0';                   /* reset to default */
      encode_entry_pron( pron2 );
      cur_entry++;

      save_word[0] = '\0';               /* def saved, clear tag */

      encode_entry( word );              /* save nondef */
      encode_entry_pron( pron_err );
      cur_entry++;

    } else {

      encode_entry( word );              /* save nondef */
      encode_entry_pron( pron_err );
      cur_entry++;
    }

    copy_str( prev_word, word );
  }
}

/* ---------------------------------------------------------------------------
   debug
   ---------------------------------------------------------------------- */

void decode_all_entry()
{
  int   i, j;
  char  line[ MAX_STR ], pron_ascii[ MAX_STR ], pron_bin[ MAX_PRON ];
  char  pron[ MAX_PRON ];
  char  pron_err[ MAX_PRON ];
  int   entry_size = 0, entry_pron_size = 0;

  for (i = 0; i < cur_entry; i++) {

    entry_size += get_strlen( entry[i] );

    line[0] = '\0';
    expand_str(line, i);
    printf("%s\t", line);
    
    entry_pron_size += entry_pron[i][0];

    pron_err[0] = '\0';
    expand_pron(pron_err, i);
    epd_make_string(line, pron_ascii, MAX_STR);
    pron2bin(pron_ascii, pron_bin);
    len_def = pron_bin[0];
    pron_def = &pron_bin[1];
    err2pron( pron_err, pron );
    for (j = 0; j < pron[0]; j++)
      printf("%s ", phone[ pron[ 1 + j ] ]);
    printf("\n");
  }

  fprintf(stderr, "word size = %d pron size = %d\n", 
	  entry_size, entry_pron_size);
}
/* ---------------------------------------------------------------------------
   output dictionary
   ---------------------------------------------------------------------- */

int output_entry( FILE *fp, int idx )
{
  int   len, byte_cnt;
  signed char  len_char;
  int   i;

  /* output alignemnt byte, it is unique, no other byte can be ALIGN,
     this alignment byte allows us to not use an address array */
  len_char = ALIGN;    /* tmp */
  fwrite( &len_char, sizeof( char ), 1, fp);

  /* output the delta-encoded entry name */
  i = 0;
  while ( entry[ idx ][ i ] )
    fwrite( &entry[ idx ][ i++ ], sizeof( char ), 1, fp);

  byte_cnt = i + 1;

  /* output the pronunciation size and error-delta encoded pronunciation,
     the pronunciation size is converted into negative to act as
     delimiter for the entry name */
  len = entry_pron[ idx ][ 0 ];
  len_char = (signed char) entry_pron[ idx ][ 0 ] | (signed char) 0x80;
  fwrite( &len_char, sizeof( char ), 1, fp);

  for (i = 0; i < len; i++)
    fwrite( &entry_pron[ idx ][ 1 + i ], sizeof( char ), 1, fp);

  byte_cnt += len + 1;

  return byte_cnt;
}

/* ---------------------------------------------------------------------------
   main
   ---------------------------------------------------------------------- */

int main(int argc, char **argv)
{
  FILE  *fpi, *fpo;
  char  line[ MAX_STR ], buf[ MAX_STR ], buf1[ MAX_STR ], buf2[ MAX_STR ];
  char  *ptr1, *ptr2;
  int   entry_offset;
  int   i;
  int dodt = 0;
  char* dt_dir;
  DIR *dirp;
  int domap = 1;


  /* TIesrDT API instance and error tracking variable */
  TIesrDT_t dtInstance;
  TIesrDT_Error_t dtError;


  if ( argc <  5 ) {
    fprintf(stderr, "\nusage: %s dict_in phone dict_out (little_endian or big_endian) [map] [dt_dir]\n\n", argv[0]);
    exit(1);
  }

  if ( strcmp(argv[4], "little_endian") == 0 ) {
    little_endian = 1;
  } else if ( strcmp(argv[4], "big_endian") == 0 ) {
    little_endian = 0;
  } else {
    fprintf(stderr, "Error: specify little_endian or big_endian!\n");
    exit(1);
  }


  /* Determine if mapping is to be performed */
  if( argc >= 6 )
  {
      domap = atoi( argv[5] );
  }


  /* 
     If a decision tree directory is specified, check for valid
     directory, and if so, use decision tree. 
  */
  if( argc == 7 )
  {
     dt_dir = argv[6];
     dirp = opendir( dt_dir );
     if( dirp == NULL )
     {
         fprintf( stderr, "Error: failed to open %s\n", dt_dir );
	exit(1);
     }
     closedir( dirp );
     dodt = 1;

     /* Create a TIesrDT instance */
     dtError = TIesrDT_CreatePreload( &dtInstance, dt_dir );
     if( dtError != TIesrDTErrorNone )
     {
	fprintf(stderr, "Could not create TIesrDT instance\n" );
	exit(1);
     }
  }

  /* Initialize dictionary processing globals */
  cur_entry = 0;
  prev_word[0] = '\0';
  save_word[0] = '\0';

  /* Open input dictionary */
  fpi = fopen(argv[1], "r");
  if( fpi == NULL )
  {
      fprintf( stderr, "Could not open %s\n", argv[1] );
      exit(1);
  }

  /* Read list of all phones used in the dictionary */
  read_phone( argv[2] );


  while ( fgets(line, MAX_STR, fpi) ) 
  {
     /* Obtain pointer to the word */
     ptr1 = (char *) strtok(line, DELIMIT);

     /* Obtain pointer to the word pronunciation phone string */
     ptr2 = (char *) strtok(NULL, "\n");


     /* Output word being compressed */
     printf( "%s\n", ptr1 );
     fflush( stdout );


    /* get default pronunciation in buf */
    if( dodt )
    {
       /* Now use TIesrDT API to get the default pronunciation */
       /* epd_make_case( ptr1, buf, MAX_STR, dt_dir, true ); */
       
       dtError = TIesrDT_Pron( dtInstance, ptr1, buf, MAX_STR );
       if( dtError != TIesrDTErrorNone )
       {
	  TIesrDT_Destroy( dtInstance );
	  fprintf( stderr, "TIesrDT failure for %s\n", ptr1 );
	  exit(1);
       }
    }
    else
    {
       epd_make_string(ptr1, buf, MAX_STR);
    }
    
    
if( ! domap )
{

    /* Add the word in ptr1 to the compressed dictionary
       based on delta coding between ptr2 (dictionary) and buf (default) 
       pronunciations. */
    process( ptr1, ptr2, buf );
}

else if( domap )
{
    
    /* Add stop closures to dictionary pronunciation held in ptr2 */
    map_pron(ptr2, buf1);

    /* Add stop closures to default word pronunciation held in buf. */
    map_pron(buf, buf2);

    /* Add the word in ptr1 to the compressed dictionary
       based on delta coding between buf1 (dictionary)  and buf2 (default)
       pronunciations. */
    process( ptr1, buf1, buf2 );
}

    if ( cur_entry >= MAX_ENTRY ) {
      fprintf(stderr, "Error: Exceed max entry %d\n", MAX_ENTRY);
      exit(1);
    }
  }

  fclose( fpi );

  dtError = TIesrDT_Destroy( dtInstance );
  if( dtError != TIesrDTErrorNone )
  {
     fprintf( stderr, "Fail during TIesrDT destroy\n" );
  }


  /* output the binary dictionary,
     the binary dictionary starts with the size of the file - 4 */

  fpo = fopen(argv[3], "wb");
  if( fpo == NULL )
  {
      fprintf( stderr, "Could not open output %s\n", argv[3]);
      exit(1);
  }

  entry_offset = 0;

  /* dummy size of the dictionary, don't know yet */
  output_int32( fpo, entry_offset );

  for (i = 0; i < cur_entry; i++)
    entry_offset += output_entry( fpo, i );

  /* real size of the dictionary, not including this 4 bytes */
  rewind( fpo );
  output_int32( fpo, entry_offset );

  fclose( fpo );

  /* decode_all_entry(); */

  exit(0);

}
