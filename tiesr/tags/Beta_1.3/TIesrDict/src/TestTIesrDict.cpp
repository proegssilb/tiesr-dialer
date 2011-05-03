/*=======================================================================

 *
 * TestTIesrDict.cpp
 *
 * Program to test the TIesrDict API.
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
 
 This program tests TIesrDict.  To test TIesrDict, you must have the
 TIesrDT API available, since TIesrDict uses TIesrDT. The command line is:

 TestTIesrDict dictDir lang dictID bUseRule numProns bUseClosure fileName
 
 dictDir is the top level directory that contains the information necessary
 to look up a pronunciation.  

======================================================================*/
#include <TIesrDict.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char* DELIMIT = " \n\t";

int main( int argc, char** argv )
{
   CTIesrDict dict;
   CTIesrDict::phoneName phoneName;
   CTIesrDict::Errors result;
   char word[100], pron[300], pronstr[3000];
   char *wordptr;
   int idx;
   unsigned int numEntries;
   unsigned int entry;
   int isEnd;
   
   char* dictDir;
   char* lang;
   char* dictID;
   
   int bUseRule = 0;
   int numProns = 0;
   int bUseFile = 0;
   int bUseClosure = 1;
   
   FILE *fp;
   
   if( argc < 4 )
      return (1);
   
   
   
   // Location of dictionary to use
   dictDir = argv[1];
   lang = argv[2];
   dictID = argv[3];
   
   // Use TIesrDT rule as a pronunciation
   if( argc > 4 )
      bUseRule = atoi( argv[4] );

   // Number of pronunciations to output
   if( argc > 5 )
      numProns = atoi( argv[5] );

   // Use closures for stop phones
   if( argc > 6 )
       bUseClosure = atoi( argv[6] );
   
   // A file of items to look up is given, or else prompt user
   bUseFile = ( argc > 7 );
   
   
   dict.LoadDictionary( dictDir, lang, dictID, bUseClosure );
   
   if( !bUseFile )
   {
      printf( "Number of phones used: %d\n", dict.GetPhoneCount() );
      
      for( idx=0; idx < dict.GetPhoneCount(); idx++ )
      {
         dict.GetPhoneName( idx, phoneName );
         printf( "%s\n", phoneName );
      }
   }
   else
   {
      fp = fopen( argv[7], "r" );
      if( !fp )
      {
         printf( "Can not open %s", argv[7] );
         exit(1);
      }
   }
   
   // Get first word
   if( !bUseFile )
   {
      printf("\nEnter Name: " );
      gets( word );
   }
   else
   {
      isEnd = fscanf( fp, "%s", word );
   }
   
   wordptr = strtok( word, DELIMIT );
   
   // Loop as long as a valid word is found
   while(  wordptr != NULL && strlen(wordptr) > 0 && strcmp(wordptr, ".") != 0
           && ( !bUseFile || ( bUseFile && isEnd != EOF ) ) )
   {
      int numPron = 0;
      
      
      // Find number of entries of the word in the dictionary
      result = dict.GetNumberEntries( wordptr, &numEntries );
      if( result == CTIesrDict::ErrNone )
      {
         if( !bUseFile )
         {
            printf( "%s has %d entries in dictionary\n", wordptr, numEntries );
         }
      }
      else
      {
         printf( "Invalid input\n" );
      }
      
      // Get rule pronunciation
      result = dict.GetPronEntry( wordptr, 0, pron, pronstr );
      if( result == CTIesrDict::ErrNone )
      {
         if( !bUseFile )
         {
            printf( "Rule:  %s ---> %s\n", wordptr, pronstr );
         }
         else if( ( bUseRule || numEntries == 0 ) && numPron < numProns )
         {
            printf( "%s ---> %s.\n", wordptr, pronstr );
            numPron++;
         }
      }
      else
         printf( "Invalid input\n" );
      
      
      
      // Loop over all entries in the dictionary
      for( entry = 1; entry <= numEntries; entry++ )
      {
         result = dict.GetNextEntry( pron, pronstr );
         if( result == CTIesrDict::ErrNone )
         {
            if( !bUseFile )
            {
               printf( "Dict(%d) for %s:  %s\n", entry, wordptr, pronstr );
            }
            else if( numPron < numProns )
            {
               printf( "%s ---> %s.\n", wordptr, pronstr );
               numPron++;
               if( numPron >= numProns )
                  break;
            }
         }
         else
            printf( "Invalid input\n" );
      }
      
      
      // Default pronunciation - first dictionary entry, or rule if no entry
      if( !bUseFile )
      {
         result = dict.GetPron( wordptr, pron, pronstr );
         if( result == CTIesrDict::ErrNone )
            printf("Used now: %s\n", pronstr );
         else
            printf( "Invalid input\n" );
      }
      
      // Get next word
      if( !bUseFile )
      {
         printf("\nEnter Name: " );
         gets( word );
      }
      else
      {
         isEnd = fscanf( fp, "%s", word );
      }
      
      wordptr = strtok( word, DELIMIT );
   }
   
   
   
   // Close file when done
   if( bUseFile )
   {
      fclose(fp);
   }
   
   return(0);
}
