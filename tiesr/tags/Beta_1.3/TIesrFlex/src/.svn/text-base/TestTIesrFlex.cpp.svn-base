/*=======================================================================

 *
 * TestTIesrFlex.cpp
 *
 * Program to test TIesrFlex API.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 Code to test TIesrFlex.

 TestTIesrFlex grammar_string output_dir modelset_dir language [ max_pron include_rule auto_silence little_endian byte_mean byte_var addClosure]

======================================================================*/

#include <TIesrFlex.h>
#include <stdlib.h>
#include <stdio.h>

// Prototype for getGrammar
char* getGrammar( char* aString );


// Main program
int main( int argc, char** argv )
{
   CTIesrFlex *flex = new CTIesrFlex();
   CTIesrFlex::Errors error;
   
   int littleEndian = 0;
   int addClosure = 1;

   if (argc != 5 && argc != 11 && argc != 12 )
      printf( "Invalid argument set\n" );

   if( argc >= 9 )
   {
      littleEndian = atoi( argv[8] );
   }

   if( argc >= 12)
   {
       addClosure = atoi( argv[11] );
   }
   // Obtain the grammar string.  The argument may be a string or file name
   char *gram = getGrammar( (char*)argv[1]);

   printf( "Loading language, grammar, and model data\n");
   error = flex->LoadLanguage( argv[3], argv[4], "Dict", "Models", littleEndian,
           addClosure );
   if( error != CTIesrFlex::ErrNone )
   {
       printf( "LoadLanguage error\n");
       delete( flex );
       exit(1);
   }

   printf( "Parsing input grammar\n");
   if( argc < 8 )
      error = flex->ParseGrammar( gram );
   else
      error = flex->ParseGrammar( gram, atoi(argv[5]), atoi(argv[6]), atoi(argv[7]) );

   if( error != CTIesrFlex::ErrNone )
   {
      printf( "Grammar parse error\n" );
      delete( flex );
      exit(1);
   }

   /* Output means and variances in byte or short, no optimization */
   printf( "Writing out binary grammar network and model files\n");
   error = flex->OutputGrammar( argv[2], atoi(argv[9]), atoi(argv[10]), 0 );
   if( error != CTIesrFlex::ErrNone )
   {
      printf( "Grammar output error\n" );
      delete( flex );
      exit(1);
   }

   printf( "Grammar network and model files output successfully\n");
   delete( flex );

   exit(0);
}


/*-----------------------------------------------------------------------------------------------
 * getGrammar
 *
 *This function returns the grammar string.  The input argument may be the string, or a filename
 *of a file containing the string.  If it is a filename, then this function reads the file and
 *obtains the string.
 *---------------------------------------------------------------------------------------------*/
char* getGrammar( char* aString )
{
    FILE *fp;
    
    // Try to open the string as a filename, if it fails then assume the 
    // aString is the grammar string
    fp = fopen( aString, "r");
    if( fp == NULL )
        return ( aString );
    
    // Grammar is in a file
    fseek( fp, 0, SEEK_END );
    long fLen = ftell( fp );
    rewind( fp );
    
    // Allocate space for grammar string
    char* gram = (char*)malloc( (fLen+2)*sizeof(char));
    if( gram == NULL )
    {
        fclose(fp);
        return gram;
    }
    
    // Read string
    fread( gram, sizeof(char), fLen, fp );
    fclose(fp);
    gram[fLen] = '\n';
    return ( gram );
    
}
