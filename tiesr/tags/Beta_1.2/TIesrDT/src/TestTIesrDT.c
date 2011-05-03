/*=======================================================================
 TestTIesrDT.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This program tests TIesrDT.  The user inputs a file containing words
 on lines, and the program sends these words to TIesrDT.  The result
 is each word on a line, followed by the pronunciation of the word
 returned by TIesrDT.
  
 The command line is:
 
 Test tree_path filename preload_trees(0/1)
  
=====================================================================*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <TIesrDT_User.h>

#define MAX_STRING  256

/* Now allow path to be input
static const char* const TREE_PATH = "../Tools/CDtrees";
*/


int main( int argc, char** argv)
{
  char* fname;
  FILE* fp;
  char word[MAX_STRING];
  char pron[MAX_STRING];


  TIesrDT_t tiesrDT;
  TIesrDT_Error_t dtError;

  if( argc < 2 )
  {
     printf("TestTIesrDT tree_path filename preload_trees(0/1) \n");
     exit(1);
  }

  fname = argv[2];
  fp = fopen( fname, "r" );
  if( ! fp )
  {
     printf( "Can not open %s\n", fname );
     exit(1);
  }
  

  //dtError = TIesrDT_Create( &tiesrDT, TREE_PATH );
  if( argc > 3 && atoi( argv[3] ) != 0 )
  {
     dtError = TIesrDT_CreatePreload( &tiesrDT, argv[1] );
  }
  else
  {
     dtError = TIesrDT_Create( &tiesrDT, argv[1] );
  }

  if( dtError != TIesrDTErrorNone )
  {
     printf( "Can not create TIesrDT instance: %d\n", dtError  );
     exit(1);
  }

  while( fscanf( fp, "%s", word ) != EOF )
  {
     printf( "%s:  ", word );

     dtError = TIesrDT_Pron( tiesrDT, word, pron, MAX_STRING );
     if( dtError != TIesrDTErrorNone )
     {
	printf( "Can not get pronunciation: %d\n", dtError );
	exit(1);
     }

     printf( "%s\n", pron );
  }

  dtError = TIesrDT_Destroy( tiesrDT );

  fclose( fp );

  exit(0);
}

