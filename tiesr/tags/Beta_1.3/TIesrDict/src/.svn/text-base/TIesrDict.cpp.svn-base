/*=======================================================================

 *
 * TIesrDict.cpp
 *
 * TIesrDict API - obtains pronunciation of a word.
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
 
 This source file defines the TIesrDict API class, which provides a
 method to determine phonetic word pronunciation from word
 spelling. 

 ======================================================================*/


/* Windows OS specific headers */
#if defined ( WIN32 ) || defined ( WINCE )
// #include <commctrl.h>
#include <windows.h>
#endif


// C++ includes
#include <new>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// API includes
#include "TIesrDict.h"
#include "TIesrDictLocal.h"

// Decision tree word to pronunciation
#ifdef _TIESRDICT_USE_TIESRDT

#include <TIesrDT_User.h>

#else

#include <dtmakeup_user.h>

#endif



//----------------------------------------------------------------
// Class public functions
//----------------------------------------------------------------


//----------------------------------------------------------------
// CTIesrDict
//
// Constructor for the class.  Initialization of pointers and variables.
//--------------------------------
   CTIesrDict::CTIesrDict( )
   {
      // The constructor sets all pointers that will be allocated to heap memory
      // to NULL.  In this way, the class can track if any failures occurred
      // and which objects contain valid pointers. The assignment to these
      // pointers are done in LoadDictionary.
      
      // pointer to string of present directory
      direct = NULL;
      
      // pointer to language string
      lang = NULL;
      
      // pointer to dictionary identifier
      ident = NULL;
      
      
      // If TIesrDT is used, then clear allocation of the TIesrDT object
#ifdef _TIESRDICT_USE_TIESRDT
      tiesrDT = NULL;
#endif
      
      
      // pointer to the allocated binary dictionary
      dict_beg = NULL;
      
      // phone list array is initially empty
      phone = NULL;
      n_phone = 0;
      
      // state for last word looked up
      m_word = NULL;
      m_defaultPron = NULL;
      m_entryLocation = 0;
      
      // No pronunciation
      // pronstr = NULL;
      // pron = NULL;
      
      return;
   }
   
   
//----------------------------------------------------------------
// ~CTIesrDict
//
// Destructor of the class
//--------------------------------
   CTIesrDict::~CTIesrDict()
   {
      delete [] direct;
      delete [] lang;
      delete [] ident;
      
      delete [] phone;
      
      delete [] m_word;
      delete [] m_defaultPron;
      
      //delete pronstr;
      //delete [] pron;
      
      // dict_beg malloced
      if( dict_beg != NULL )
         free( dict_beg );
      
      
      // If TIesrDT used, destroy any existing copy of the object
#ifdef _TIESRDICT_USE_TIESRDT
      if( tiesrDT != NULL )
      {
         TIesrDT_Destroy( tiesrDT );
      }
#endif
      
   }
   
   
//----------------------------------------------------------------
// LoadDictionary
//
// This function loads the dictionary object with a specific dictionary. In
// addition, if the code is compiled to use the new TIesrDT decision tree
// searching code, then this function will also create an instantiation of the
// TIesrDT object.  It returns ErrNone if the loading was successful,
// otherwise, it returns a non-zero error status value, which indicates the
// error type. This function can be called multiple times, Each instance
// removes the old dictionary information, and installs new dictionary
// information.  In this way, the user can declare one dictinonary object
// instance and reuse it for multiple dictionary types by reloading.  Loading
// requires the location of a directory containing all language information.
// The aDictDir argument should be the base directory, for example my/dict.
// The location of the dictionary itself is in directory
// my/dict/aLanguage/aDictID.  This allows multiple languages, and multiple
// dictionary types for each language.  Under aDictDir the aLanguage directory
// should hold the decision trees for generating candidate pronunciations for
// a given language.  The directory aDictID should hold the binary dictionary
// file dict.bin and the listing of phones in file phone.lst, where the
// dictionary and phone list have been created by compressing a dictionary
// using Yu-Hung's dictionary compression tools.  See
// /speech/tools2/util/asr/context_dep_phone/dictproc.
//
//--------------------------------
   CTIesrDict::Errors  CTIesrDict::LoadDictionary( const char *aDictDir,
           const char *aLanguage,
           const char *aDictID,
           int addClosure )
   {
      char *pathName = NULL;
      char *fileName = NULL;
      int dirLen, fileLen;
      int status;
      
      // Remove any prior location information
      delete [] direct;
      direct = NULL;
      delete [] lang;
      lang = NULL;
      delete [] ident;
      ident = NULL;


      // Store status of adding stop closures
      doClosure = addClosure;
      
      // If using TIesrDT and a prior TIesrDT object exists, then remove it.
#ifdef _TIESRDICT_USE_TIESRDT
      if( tiesrDT != NULL )
      {
         TIesrDT_Destroy( tiesrDT );
         tiesrDT = NULL;
      }
#endif
      
      
      // Save the location of information in class, not used right
      // now, but may be valuable in the future
      try
      {
         direct = new char[ strlen( aDictDir ) + 1 ];
         lang = new  char[ strlen( aLanguage ) + 1 ];
         ident = new char[ strlen( aDictID ) + 1];
      }
      catch( std::bad_alloc &ex )
      {
         return ErrMemory;
      }
      
      strcpy( direct, aDictDir );
      strcpy( lang, aLanguage );
      strcpy( ident, aDictID );
      
      
      // directories containing the decision tree and dictionary information
      dirLen = strlen(direct) + strlen(lang) + strlen(ident) + 4;
      
      try
      { pathName = new char[ dirLen ]; }
      catch( std::bad_alloc &ex )
      {
         return ErrMemory;
      }
      strcpy( pathName, direct );
      strcat( pathName, PATH_SEPARATOR );
      strcat( pathName, lang );
      
      
      // If using TIesrDT, then create the instance of the object.
#ifdef _TIESRDICT_USE_TIESRDT
      TIesrDT_Error_t dtError;
      dtError = TIesrDT_CreatePreload( &tiesrDT, pathName );
      if( dtError != TIesrDTErrorNone )
      {
         delete [] pathName;
         return ErrFail;
      }
#endif
      
      
      // Complete construction of the pathName of the dictionary
      strcat( pathName, PATH_SEPARATOR );
      strcat( pathName, ident );
      strcat( pathName, PATH_SEPARATOR );
      
      // Construct the phone information into phone[][] variable
      fileLen = strlen(pathName) + strlen(PHONELIST) + 1;
      try
      { fileName = new char[ fileLen ]; }
      catch( std::bad_alloc &ex )
      {
         delete [] pathName;
         return ErrMemory;
      }
      strcpy( fileName, pathName );
      strcat( fileName, PHONELIST );
      
      
      // should be some way to abort if this fails
      status = read_phone( fileName );
      delete [] fileName;
      if( status != ErrNone )
      {
         delete [] pathName;
         return ErrFail;
      }
      
      // find the location of the phonetic dictionary
      fileLen = strlen( pathName ) + strlen( DICTFILE ) + 2;
      try
      { fileName = new char[ fileLen ]; }
      catch( std::bad_alloc )
      {
         delete [] pathName;
         return ErrMemory;
      }
      strcpy( fileName, pathName );
      strcat( fileName, DICTFILE );
      
      // read the dictionary
      status = read_binary_dictionary( fileName );
      
      delete [] fileName;
      delete [] pathName;
      
      if( status != ErrNone )
         return ErrFail;
      else
         return ErrNone;
   }
   
   
//--------------------------------
// GetPhoneCount
//
// Return the number of phones in the loaded language
//
   int CTIesrDict::GetPhoneCount()
   {
      return n_phone;
   }
   
   
//--------------------------------
// GetPhoneName
//
// Copy the name of the phone in the list at position aIndex to a
// user supplied location aPhoneName.  Note that the use must
// guarantee space available in aPhoneName, and is responsible for
// managing that space.
//
   CTIesrDict::Errors CTIesrDict::GetPhoneName( unsigned int aIndex,
           phoneName aPhoneName )
   {
      if( n_phone == 0 )
         return ErrFail;
      
      if(  aIndex >= n_phone )
         return ErrInput;
      
      strcpy( aPhoneName, phone[aIndex] );
      
      return ErrNone;
   }
   
   
   /*----------------------------------------------------------------
  GetDefaultPron
 
  Get the default, that is rule-based pronunciation for a word.
 
  ----------------------------------------------------------------*/
   CTIesrDict::Errors CTIesrDict::GetDefaultPron( const char* aWord, char aPron[] )
   {
      int idx;
      unsigned int dirLen;
      
      char *pathName = NULL;
      char  *pron_ascii = NULL;
      char *pron_ascii_2 = NULL;
      char  *buf = NULL;
      
      int maxString;
      
      CTIesrDict::Errors error = ErrNone;
      
#ifndef _TIESRDICT_USE_TIESRDT
      Boolean result;

      
      
      // Protect epd_make_case.  It can't handle non-alphabetic chars
      for( idx=0; idx < (int)strlen(aWord); idx++ )
      {
         if( ! ( (aWord[idx] > 64 && aWord[idx] < 91) ||
                 (aWord[idx] > 96 && aWord[idx] < 123 ) ) )
         {
            return ErrInput;
         }
      }

#else
      // Allow alphabetic characters, and the characters ' - .
      for( idx=0; idx < (int)strlen(aWord); idx++ )
      {
         if( ! ( (aWord[idx] > 64 && aWord[idx] < 91) ||
                 (aWord[idx] > 96 && aWord[idx] < 123 ) ||
                 (aWord[idx] == 39) || (aWord[idx] == 45) ||
                 (aWord[idx] == 46) ) )
         {
            return ErrInput;
         }
      }

#endif

      // directory containing the decision tree information
      dirLen = strlen(direct) + strlen(lang) +3;
      try
      { pathName = new char[ dirLen ]; }
      catch( std::bad_alloc &ex )
      {
         error = ErrMemory;
         goto FinishPron;
      }
      
      // Make pathName
      strcpy( pathName, direct );
      strcat( pathName, PATH_SEPARATOR );
      strcat( pathName, lang );
      strcat( pathName, PATH_SEPARATOR );
      
      // word pronunciation phone string length must not exceed this estimate
      maxString = MAXPHONESPERWORD*MAXPHONENAME;
      
      try
      {
         pron_ascii = new char[maxString];
         pron_ascii_2 = new char[maxString];
         
         // Modifiable buffer holding word to look up
         buf = new char[ strlen(aWord) + 1];
      }
      catch( std::bad_alloc &ex )
      {
         error = ErrMemory;
         goto FinishPron;
      }
      
      
      // Copy word to buf and convert to uppercase
      strcpy(buf, aWord);
      chrtoupper( buf );
      
      
      // get default pronunciation, using TIesrDT or dtmakeup depending
      // on which API was compiled.
#ifdef _TIESRDICT_USE_TIESRDT
      TIesrDT_Error_t dtError;
      dtError = TIesrDT_Pron( tiesrDT, buf, pron_ascii, maxString );
      
      // TIesrDT can return an empty string for default pronunciation
      if( dtError != TIesrDTErrorNone /*|| strlen( pron_ascii ) == 0 */ )
      {
         error = ErrFail;
         goto FinishPron;
      }
      
#else
      result = epd_make_case(buf, pron_ascii, maxString, pathName, true );
      if( result != true || strlen( pron_ascii ) == 0  )
      {
         error = ErrFail;
         goto FinishPron;
      }
#endif
      
      
      // Map to include stop closures in word phones if needed
      if( doClosure )
      {
          map_pron(pron_ascii, pron_ascii_2);
      }
      else
      {
          strcpy(pron_ascii_2, pron_ascii );
      }
      
      // Convert ascii to phone indices
      pron2bin(pron_ascii_2, aPron);
      
      
      FinishPron:
      delete [] pathName;
      delete [] pron_ascii;
      delete [] pron_ascii_2;
      delete [] buf;
      
      return error;
   }
   
   
//--------------------------------
// GetPron
//
// Get the pronunciation of the word provided as the input argument.
// The pron[] array will be filled with the indices of the phones,
// and the pronstr will be filled with the string of phones.
//
   CTIesrDict::Errors CTIesrDict::GetPron( const char *aWord,
           char aPron[],
           char *aPronString )
   {
      CTIesrDict::Errors error;
      int   idx;
      char  *ucWord;
      
      
      // Delete any existing lookup for GetPronEntry and GetNextEntry
      delete [] m_word;
      m_word = NULL;
      
      delete [] m_defaultPron;
      m_defaultPron = NULL;
      
      m_entryLocation = 0;
      m_entryNumber = 0;
      
      // Get the default pronunciation
      error = GetDefaultPron( aWord, aPron );
      if( error != ErrNone )
      {
         return ErrFail;
      }
      
      
      // Modifiable buffer holding word to look up in upper case
      try
      { ucWord = new char[ strlen(aWord) + 1]; }
      catch( std::bad_alloc &ex )
      {
         return ErrMemory;
      }
      
      strcpy( ucWord, aWord );
      chrtoupper(ucWord);
      
      
      /* dictionary lookup */
      idx = lookup( ucWord );
      
      // If word found in dictionary, do use the exception
      if ( idx != -1 )
      {
         /* use exception.*/
         // Note: decode_entry finds and uses first word entry in dictionary
         decode_entry(ucWord, idx, aPron);
      }
      
      
      // Done with word text
      delete [] ucWord;
      
      
      // output the pronunciation string from dictionary lookup
      if( aPronString != NULL )
      {
         PronToString( aPron, aPronString );
      }
      
      return ErrNone;
   }
   
   
   
   /*----------------------------------------------------------------
  GetNumberEntries
 
  Get the number of entries in the dictionary for a word specified by
  aWord.  Note that this does NOT include the rule-based default
  pronunciation, only the number of pronunciations in the dictionary.
  Hence the number returned in aNumberEntries can be zero.
 
  ----------------------------------------------------------------*/
   CTIesrDict::Errors CTIesrDict::GetNumberEntries( const char* aWord, unsigned int *aNumberEntries )
   {
      int startLocation;
      int pronLocation;
      int noMatch;
      char dictWord[MAX_STR];
      char *ucWord;
      int numEntries;
      
      *aNumberEntries = 0;
      numEntries = 0;
      
      // convert the word to upper case.
      try
      { ucWord = new char[ strlen(aWord) + 1]; }
      catch( std::bad_alloc &ex )
      {
         return ErrMemory;
      }
      
      strcpy( ucWord, aWord );
      chrtoupper( ucWord );
      
      // Check for existence of the word in the dictionary
      startLocation = lookup( ucWord );
      if( startLocation == -1 )
      {
         delete [] ucWord;
         return ErrNone;
      }
      
      // Found an entry in the dictionary
      numEntries++;
      pronLocation = startLocation;
      noMatch = 0;
      
      // Search backward in dictionary to find all prior entries that match
      while( ! noMatch && pronLocation > first )
      {
         // Go to prior entry location in dictionary
         pronLocation = dec_entry( pronLocation );
         
         // Prior word at the entry location
         dictWord[0] = '\0';
         expand_str( dictWord, pronLocation );
         
         // If the dictionary word matches the word we want, then increment count
         noMatch = compare_str( ucWord, dictWord );
         if( ! noMatch )
         {
            numEntries++;
         }
      }
      
      // Look forward in the dictionary for subsequent matching entries
      pronLocation = startLocation;
      noMatch = 0;
      
      while( ! noMatch && pronLocation < last )
      {
         // Go to next entry location in dictionary
         pronLocation = inc_entry( pronLocation );
         
         // Prior word at the entry location
         dictWord[0] = '\0';
         expand_str( dictWord, pronLocation );
         
         // If the dictionary word matches the word we want then increment count
         noMatch = compare_str( ucWord, dictWord );
         if( ! noMatch )
         {
            numEntries++;
         }
      }
      
      *aNumberEntries = numEntries;
      
      delete [] ucWord;
      return ErrNone;
   }
   
   
   
   /*----------------------------------------------------------------
  GetPronEntry
 
  Get the Nth pronunciation entry of a word specified by aWord.  The
  value of N is specified by the argument aEntryNumber.  If
  aEntryNumber is zero, then the default rule-based pronunciation is
  returned without consulting the dictionary.  Otherwise the Nth
  pronunciation of the word in the dictionary is returned.  If the Nth
  pronunciation does not exist, aPron will have zero in the first
  character, and if aPronString is not NULL, it will be set to "".
 
  ----------------------------------------------------------------*/
   CTIesrDict::Errors CTIesrDict::GetPronEntry( const char *aWord,
           const unsigned int aEntryNumber,
           char aPron[], char *aPronString )
   {
      Errors error;
      int phone;
      
      
      // No dictionary lookup yet
      m_entryLocation = 0;
      m_entryNumber = 0;
      
      // Reallocate space to hold the word and its default pronunciation
      delete [] m_defaultPron;
      m_defaultPron = NULL;
      
      delete [] m_word;
      m_word = NULL;
      
      try
      {
         m_word = new char[ strlen( aWord ) + 1 ];
         
         // Create space to hold the phones of the default pronunciation of the word.
         m_defaultPron = new char[ MAXPHONESPERWORD + 1];
      }
      catch( std::bad_alloc &ex )
      {
         return ErrMemory;
      }
      
      // Copy of the word in upper case
      strcpy( m_word, aWord );
      chrtoupper( m_word );
      
      
      // Get default pronunciation and check for failure
      error = GetDefaultPron( m_word, m_defaultPron );
      if( error != ErrNone )
      {
         strcpy( m_word, "" );
         return ErrFail;
      }
      
      
      // Copy default pronunciation phone indices and pron string to output.
      // This will be replaced if the desired dictionary entry is found.
      for( phone=0; phone <= m_defaultPron[0]+1; phone++ )
      {
         aPron[phone] = m_defaultPron[phone];
      }
      
      if( aPronString )
         PronToString( m_defaultPron, aPronString );
      
      
      // User only wants the rule-based pronunciation
      if( aEntryNumber == 0 )
         return ErrNone;
      
      
      // User requested a pronunciation from the dictionary.  Get the
      // location of the requested entry in the dictionary for this word.
      m_entryLocation = LocatePron( m_word, aEntryNumber );
      if( m_entryLocation == 0 )
         return ErrNotInDictionary;
      
      
      // Get the pronunciation of the word according to the dictionary
      print_pron( m_word, m_entryLocation, m_defaultPron, aPron );
      if( aPronString )
         PronToString( aPron, aPronString );
      
      m_entryNumber = aEntryNumber;
      
      return ErrNone;
   }
   
   
   /*----------------------------------------------------------------
  GetNextEntry
 
  This function should only be called after a call to GetPronEntry.  It
  provides a means of obtaining the next pronunciation of the word
  specified in GetPronEntry.  If the next pronunciation does not exist,
  aPron will have zero in the first character, and if aPronString is
  not NULL, it will be set to "", and the fuction will return
  ErrNotInDictionary.  This function call is provided so that
  dictionary search does not need to be done for each of multiple
  pronunciations looked up in the dictionary.
 
  ----------------------------------------------------------------*/
   CTIesrDict::Errors CTIesrDict::GetNextEntry( char aPron[], char *aPronString )
   {
      int pronLocation;
      
      
      // Check that a word has been looked up by GetPronEntry
      if( m_word == NULL || strcmp(m_word, "") == 0 )
      {
         aPron[0] = 0;
         if( aPronString )
            aPronString = "";
         return ErrFail;
      }
      
      
      // Try to find a valid dictionary entry location for the next
      // word entry.
      
      // If no dictionary entry has been looked up yet,
      // try to find the first dictionary entry.
      if( m_entryLocation == 0 )
      {
         pronLocation = LocatePron( m_word, 1 );
      }
      
      // Otherwise, if not at end of dictionary try to find next entry.
      else if( m_entryLocation < last )
      {
         char dictWord[MAX_STR];
         dictWord[0] = '\0';
         
         pronLocation = inc_entry( m_entryLocation );
         expand_str( dictWord, pronLocation );
         
         // Next word entry does not match word
         if( strcmp(dictWord, m_word ) != 0 )
         {
            pronLocation = 0;
         }
      }
      
      // At the end of dictionary
      else
         pronLocation = 0;
      
      
      // If no valid location, then no more entries of this word exist
      // in the dictionary
      if( pronLocation < first || pronLocation > last )
      {
         aPron[0] = 0;
         if( aPronString )
            aPronString = "";
         
         return ErrNotInDictionary;
      }
      
      
      // A valid word exists in the next dictionary location
      m_entryLocation = pronLocation;
      
      // Get the pronunciation of the word according to the dictionary
      print_pron( m_word, m_entryLocation, m_defaultPron, aPron );
      if( aPronString )
         PronToString( aPron, aPronString );
      
      m_entryNumber++;
      
      return ErrNone;
   }
   

/*----------------------------------------------------------------
  LocatePron
 
  This function locates the one-based Nth pronunciation for a word in
  the dictionary.  N is specified by the argument aEntryNumber.  This
  function returns the byte entry location in the dictionary if the
  Nth pronunciation for the word exists, otherwise it returns 0.
 
 ----------------------------------------------------------------*/
   int CTIesrDict::LocatePron( char* aWord, unsigned int aEntryNumber )
   {
      int startLocation;
      int pronLocation;
      char dictWord[MAX_STR];
      unsigned int entryNumber;
      int noMatch;
      
      // User should not request the zeroth entry.
      if( aEntryNumber == 0 )
         return 0;
      
      
      // Check for existence of the word in the dictionary
      startLocation = lookup( aWord );
      if( startLocation == -1 )
      {
         return 0;
      }
      
      
      // Found an entry for the word in the dictionary.
      // Search backward in dictionary to find the first entry of this word.
      noMatch = 0;
      
      while( ! noMatch && startLocation > first )
      {
         // Go to prior entry location in dictionary
         pronLocation = dec_entry( startLocation );
         
         // Prior word at the entry location
         dictWord[0] = '\0';
         expand_str( dictWord, pronLocation );
         
         // If the dictionary word matches the word we want, then continue searching backward
         noMatch = compare_str( aWord, dictWord );
         if( ! noMatch )
         {
            startLocation = pronLocation;
         }
      }
      
      
      // If user wants the first entry, it is the present one
      if( aEntryNumber == 1 )
      {
         return startLocation;
      }
      
      
      // User wants an entry number > 1 for the word.
      // Try searching forward for the entry number the user wants
      entryNumber = 1;
      noMatch = 0;
      while( ! noMatch && startLocation < last )
      {
         // Go to next entry location in dictionary
         pronLocation = inc_entry( startLocation );
         
         // Word at the present entry location
         dictWord[0] = '\0';
         expand_str( dictWord, pronLocation );
         
         // If the dictionary word matches the word wanted then increment count
         // and determine if it is the entry number wanted
         noMatch = compare_str( aWord, dictWord );
         if( ! noMatch )
         {
            entryNumber++;
            
            // Return this entry, which is the entry number wanted
            if( entryNumber == aEntryNumber )
               return pronLocation;
            
            // Continue searching, have not found desired entry number yet
            startLocation = pronLocation;
         }
         else
         {
            // No more words match, and the wanted entry has not been found
            return 0;
         }
         
      }
      
      // Did not find the wanted entry number for the wanted word
      return 0;
      
   }
   
   
   
   /*----------------------------------------------------------------
  PronToString
 
  Converts a phone index-based pronunciation into a string.
 
  ----------------------------------------------------------------*/
   CTIesrDict::Errors CTIesrDict::PronToString( char* aPron, char* aPronString )
   {
      int phnIndex;
      aPronString[0] = '\0';
      for( phnIndex = 1; phnIndex <= aPron[0]; phnIndex++ )
      {
         strcat( aPronString, phone[ aPron[phnIndex] ] );
         if( phnIndex < aPron[0] )
            strcat( aPronString, " " );
      }
      
      return ErrNone;
   }
   
   
   
//----------------------------------------------------------------
// Class private functions
//----------------------------------------------------------------
   
   
//--------------------------------
// read_phone
//
// This function reads the list of phones from the file phone.lis
// and loads them in the CTIesrDict object.
//
   CTIesrDict::Errors CTIesrDict::read_phone( char *fname )
   {
      FILE  *fp;
      phoneName phName;
      int phnidx;
      
      
      // clear any preexisting phone list
      if( n_phone > 0 )
      {
         delete [] phone;
         phone = NULL;
         n_phone = 0;
      }
      
      
      // open new phone file
      fp = fopen(fname, "r");
      if ( fp == NULL )
      {
         return ErrFail;
      }
      
      
      // Determine number of phones, and ensure proper formatting
      while ( fscanf(fp, "%s", phName) != EOF )
      {
         n_phone++;
      }
      
      
      // Create the phone list
      try
      {phone = new phoneName[n_phone+1]; }
      catch( std::bad_alloc )
      {
         fclose(fp);
         return ErrFail;
      }
      
      fseek( fp, 0, SEEK_SET );
      clearerr( fp );

      phnidx = 0;
      while ( fscanf(fp, "%s", phName) != EOF )
      {
         strcpy( phone[phnidx], phName );
         phnidx++;
      }
      
      
      // n_phone does not count this BOUNDARY phone
      strcpy( phone[ phnidx ], BOUNDARY );
      
      fclose( fp );
      
      return ErrNone;
   }
   
   
//--------------------------------
   CTIesrDict::Errors CTIesrDict::read_binary_dictionary( char *fname )
   {
      FILE  *fp;
      int   size;
      
      // if dictionary exists, free it
      if( dict_beg != NULL )
         free( dict_beg );
      
      /* read binary dictionary */
      fp = fopen(fname, "rb");
      if ( fp == NULL )
      {
         return ErrFail;
      }
      
      fread(&size, sizeof(int), 1, fp);
      dict_beg = (char *) malloc( size );
      
      if( !dict_beg )
         return ErrFail;
      
      
      fread(dict_beg, sizeof(char), size, fp);
      fclose( fp );
      
      
      // Set offsets for first and last indices
      first = 1;
      
      last = size - 1;
      while( dict_beg[last] != ALIGN ) last--;
      last++;
      
      return ErrNone;
   }
   
   
//----------------------------
// map_phone
//
// map one phone into one or more phones
//
   void CTIesrDict::map_phone(char *iphone, char *ophone)
   {
      int   i;
      
      for ( i = 0; i < N_PAIR; i++ )
      {
         if ( strcmp( iphone, mapping_pair[i].from ) == 0 )
         {
            strcpy( ophone, mapping_pair[i].to );
            return;
         }
      }
      
      strcpy( ophone, iphone );
      
   }
   
   
//--------------------------
// map_pron
//
// map ASCII pron (phone sequence)
//
   void CTIesrDict::map_pron(char *ipron, char *opron)
   {
      char  *ptr;
      char  buf[ MAX_STR ], ophone[ MAX_STR ];
      
      *opron = '\0';
      strcpy( buf, ipron );
      
      ptr = strtok( buf, DELIMIT );
      
      while ( ptr )
      {
         
         map_phone( ptr, ophone );
         strcat( opron, ophone );
         strcat( opron, " " );
         
         ptr = strtok( NULL, DELIMIT );
      }
      
   }
   
   
//------------------------
   /* ---------------------------------------------------------------------------
string can be terminated by 0 or negative
---------------------------------------------------------------------- */
   void CTIesrDict::copy_str(char *str1, char *str2)
   {
      while ( *str2 && ( ( *str2 & MASK_3 ) == '\0' ) )
         *str1++ = *str2++;
      
      *str1 = '\0';
      
   }
   
   
//--------------------------------
   /* ---------------------------------------------------------------------------
return -1 0 1 if str1 < = > str2
---------------------------------------------------------------------- */
   int CTIesrDict::compare_str(char *str1, char *str2)
   {
      while ( *str1 && *str2 )
      {
         
         if ( *str1 > *str2 )
            return 1;
         else if ( *str1 < *str2 )
            return -1;
         
         str1++;
         str2++;
      }
      
      if ( ( *str1 == 0 ) && ( *str2 != 0 ) )
         return -1;
      else if ( ( *str1 != 0 ) && ( *str2 == 0 ) )
         return 1;
      else
         return 0;
      
   }
   
   
//---------------------------
   /* ---------------------------------------------------------------------------
offset computation
---------------------------------------------------------------------- */
   int CTIesrDict::mid_entry(int i1, int i2)
   {
      int   i;
      
      if ( i1 == i2 ) return i1;
      
      i = ( i1 + i2 ) >> 1;
      
      while ( dict_beg[ i ] != ALIGN ) i++;
      i++;
      return i;
      
   }
   
   
//------------------------
   int CTIesrDict::inc_entry(int i)
   {
      if ( i == last )
      {
         
         return i + 1;
         
      } else
      {
         
         while ( dict_beg[ i ] != ALIGN ) i++;
         i++;
         return i;
      }
      
   }
   
   
//--------------------------
   int CTIesrDict::dec_entry(int i)
   {
      if ( i == first )
      {
         
         return i - 1;
         
      } else
      {
         
         i -= 2;
         
         while ( dict_beg[ i ] != ALIGN ) i--;
         i++;
         return i;
      }
      
   }
   
   
//---------------------------
   /* ---------------------------------------------------------------------------
must do buf[0] = '\0' before calling expand_str()
---------------------------------------------------------------------- */
   void CTIesrDict::expand_str(char *buf, int idx)
   {
      int   len;
      
      if ( (signed char) dict_beg[ idx ] >= 0 )
      {
         
         copy_str(buf, &dict_beg[ idx ]);
         
      } else
      {
         
         expand_str(buf, dec_entry( idx ) );
         len = - (signed char) dict_beg[ idx ];
         buf += len;
         copy_str(buf, &dict_beg[ idx + 1 ]);
      }
      
   }
   
   
//-----------------------------
   /* ---------------------------------------------------------------------------
get the offset for pronunciation for the word offset idx
---------------------------------------------------------------------- */
   int CTIesrDict::get_pron_offset(int idx)
   {
      idx++;               /* first char may be negative, skip */
      
      while ( ( dict_beg[ idx ] & MASK_3 ) == '\0' )
         idx++;
      
      return idx;
      
   }
   
   
//-------------------------------
   /* ---------------------------------------------------------------------------
convert ascii phone sequence to binary
pron[0] = length in bytes, total length is pron[0] + 1
---------------------------------------------------------------------- */
   void CTIesrDict::pron2bin(char *str, char *pron)
   {
      int   i = 1;
      char  *ptr;
      
      ptr = strtok(str, DELIMIT);
      while ( ptr )
      {
         pron[ i++ ] = get_phone_idx( ptr );
         ptr = strtok( NULL, DELIMIT );
      }
      
      pron[ 0 ] = i - 1;
      
   }
   
   
//----------------------
   /* ---------------------------------------------------------------------------
convert pron_err sequence to pron,
pron[0] = length in bytes, total length is pron[0] + 1
---------------------------------------------------------------------- */
   void CTIesrDict::err2pron(char *pron_def, int len_def, char *pron_err, char *pron)
   {
      int   i;
      int   len, idx1, idx2, pos, insc;
      
      /* go through pron_err and reconstruct pron */
      
      insc = 0;
      idx1 = idx2 = 0;
      len = pron_err[0];
      
      i = 1;
      
      while ( i <= len )
      {
         
         if ( ( pron_err[i] & MASK ) == SUB )
         {            /* SUB */
            
            pos = pron_err[i] & 0x3f;   /* position */
            while ( (idx1 + insc) < pos )
               pron[ 1 + idx1++ ] = pron_def[ idx2++ ];
            
            pron[ 1 + idx1++ ] = pron_err[ i + 1 ];
            idx2++;
            
            i += 2;
            
         } else if ( ( pron_err[i] & MASK ) == DEL )
         {     /* DEL */
            
            pos = pron_err[i] & 0x3f;   /* position */
            while ( (idx1 + insc) < pos )
               pron[ 1 + idx1++ ] = pron_def[ idx2++ ];
            
            pron[ 1 + idx1++ ] = pron_err[ i + 1 ];
            
            i += 2;
            
         } else
         {                                          /* INS */
            
            pos = pron_err[i] & 0x3f;   /* position */
            while ( (idx1 + insc) < pos )
               pron[ 1 + idx1++ ] = pron_def[ idx2++ ];
            
            idx2++;
            
            insc++;
            
            i++;
         }
      }
      
      /* the rest, including no err */
      
      while ( idx2 < len_def )
         pron[ 1 + idx1++ ] = pron_def[ idx2++ ];
      
      pron[0] = idx1;
      
   }
   
   
//-----------------------
   /* ---------------------------------------------------------------------------
must do buf[0] = '\0' before calling expand_pron()
very similar to expand_str(), but the format is different.
 
  "String" is terminated by '\0'.
 
    "Pronunciation" is prefixed by length, followed by length bytes,
    there is no termination char.
---------------------------------------------------------------------- */
   void CTIesrDict::expand_pron(char *buf, int idx)
   {
      int   len, base;
      int   i;
      int   ipron;
      
      ipron = get_pron_offset( idx );
      
      if ( ( dict_beg[ ipron ] & MASK_2 ) == 0 )
      {  /* remove the negative bit */
         
         buf[0] = '\0';
         
      } else if ( ( dict_beg[ ipron + 1 ] & MASK ) != MATCH )
      {
         
         len = dict_beg[ ipron ] & MASK_2;
         for (i = 1; i <= len; i++) buf[ i ] = dict_beg[ ipron + i ];
         buf[0] = len;
         
      } else
      {                                    /* delta encoded */
         
         expand_pron(buf, dec_entry( idx ) );
         len = dict_beg[ ipron + 1 ];              /* len of match with previous */
         base = len + 1;
         len = ( dict_beg[ ipron ] & MASK_2 ) - 1; /* len of this pron */
         for (i = 0; i < len; i++) buf[ base + i ] = dict_beg[ ipron + 2 + i ];
         buf[0] = base + len - 1;
      }
      
   }
   
   
//------------------------------
   /* ---------------------------------------------------------------------------
    get_phone index
    ---------------------------------------------------------------------- */
   int CTIesrDict::get_phone_idx(char *name)
   {
      unsigned int i;
      
      for(i = 0; i < n_phone; i++)
      {
         if ( strcmp(name, phone[i]) == 0 )
            return i;
      }
      
      //fprintf(stderr, "Error: %s not a phone\n", phone);
      return( -1 );
   }
   
   
//----------------------------
   /* ---------------------------------------------------------------------------
    print pron
    ---------------------------------------------------------------------- */
   void CTIesrDict::print_pron(char *word, int idx, char *pron_def, char *pron)
   {
      char  pron_err[ MAX_PRON ];
      
      pron_err[0] = '\0';                         /* get entry in dict */
      expand_pron(pron_err, idx);
      err2pron( &pron_def[1], (int) pron_def[0], pron_err, pron );
   }
   
   
//------------------------
   /* ---------------------------------------------------------------------------
    decode one entry
    ---------------------------------------------------------------------- */
   void CTIesrDict::decode_entry(char *word, int idx, char *pron)
   {
      char  buf[ MAX_STR ];
      char  pron_def[ MAX_PRON ];
      int   idx2, i;
      
      for (i = 0; i < pron[0] + 1; i++)
         pron_def[i] = pron[i];
      
      /* go up, may be more than one entry, find the first entry of this word */
      
      idx2 = idx;
      
      idx2 = dec_entry( idx2 );
      if ( idx2 >= first )
      {
         buf[0] = '\0';
         expand_str(buf, idx2);
      }
      while ( ( idx2 >= first ) && compare_str(word, buf) == 0 )
      {
         
         idx = idx2;
         
         idx2 = dec_entry( idx2 );
         if ( idx2 >= first )
         {
            buf[0] = '\0';
            expand_str(buf, idx2);
         }
      }
      
      /* go down, starting from first entry of this word */
      
      idx2 = idx;
      
      buf[0] = '\0';
      expand_str(buf, idx2);
      
      while ( ( idx2 <= last ) && compare_str(word, buf) == 0 )
      {
         
         print_pron( word, idx2, pron_def, pron );
         
         return;                /* only the first pron, discard multiple pron */
         
         idx2 = inc_entry( idx2 );
         if ( idx2 <= last )
         {
            buf[0] = '\0';
            expand_str(buf, idx2);
         }
      }
      
   }
   
   
//------------------------------
   /* ---------------------------------------------------------------------------
    convert to uppercase
    ---------------------------------------------------------------------- */
   void CTIesrDict::chrtoupper(char *str)
   {
      while ( *str )
      {
         
         if ( *str >= 'a' && *str <= 'z' )
            *str -= 'a' - 'A';
         
         str++;
      }
      
   }
   
   
//----------------------
   /* ---------------------------------------------------------------------------
    dictionary lookup, binary search, it returns:
    
    index: first <= index <= last
    -1: not found
    ---------------------------------------------------------------------- */
   int CTIesrDict::lookup(char *word)
   {
      int   idx, icompare, imax, imin;
      char  buf[ MAX_STR ];
      
      imax = last;
      imin = first;
      
      while ( imax >= imin )
      {
         
         idx = mid_entry( imin, imax );
         
         buf[0] = '\0';
         expand_str(buf, idx);
         
         icompare = compare_str(word, buf);
         if ( icompare == 0 )
         {
            return idx;
         } else if ( icompare > 0 )
         {
            imin = inc_entry( idx );
         } else
         {
            imax = dec_entry( idx );
         }
      }
      
      return -1;
   }
