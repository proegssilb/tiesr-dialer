/*=======================================================================

 *
 * TIesrDict.h
 *
 * Header file for TIesrDict API interface.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 2.1 as
 * published by the Free Software Foundation and with respect to the additonal
 * permission below.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * As an additional permission to the GNU Lesser General Public License version
 * 2.1, the object code form of a "work that uses the Library" may incorporate
 * material from a header file that is part of the Library.  You may distribute
 * such object code under terms of your choice, provided that:
 *   (i)   the header files of the Library have not been modified; and
 *   (ii)  the incorporated material is limited to numerical parameters, data
 *         structure layouts, accessors, macros, inline functions and
 *         templates; and
 *   (iii) you comply with the terms of Section 6 of the GNU Lesser General
 *         Public License version 2.1.
 *
 * Moreover, you may apply this exception to a modified version of the Library,
 * provided that such modification does not involve copying material from the
 * Library into the modified Library's header files unless such material is
 * limited to (i) numerical parameters; (ii) data structure layouts;
 * (iii) accessors; and (iv) small macros, templates and inline functions of
 * five lines or less in length.
 *
 * Furthermore, you are not required to apply this additional permission to a
 * modified version of the Library.
 *
 

This header contains the description of the CTIesrDict class API.
This class provides the user with a method to look up phonetic 
pronunciation of words from their spelling.  

The method used is to create a candidate phonetic pronunciation
using  decision tree technology.  Then look up the
word in the delta-encoded dictionary.  If there is an entry for the
word, then modify the candidate according to the dictionary,
otherwise use the candidate pronunciation itself.  

The sequence of usage is to first to instantiate an instance of the
TIesrDict object.

Next the TIesrDict object is loaded with a dictionary by
LoadDictionary.  This allows one to utilize a particular dictionary
for a language.

To get word pronunciations, use GetPron. The result will be an array
of byte indices in the variable pron that describes the indices of
the phones corresponding to a word, and in pronstr there will be a
string of phones that make up the word, separated by whitespace.

A convenience function, GetPhoneCount can be used to determine the
number of phones used in the dictionary.

A convenience function, GetPhoneName returns a phone spelling for each
phone used in the dictionary.

======================================================================*/

#ifndef _TIESRDICT_H
#define _TIESRDICT_H


/* Windows method of exporting DLL functions */

#if defined( WIN32 ) || defined ( WINCE )

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TIESRDICT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TIESRDICT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TIESRDICT_EXPORTS
#define TIESRDICT_API __declspec(dllexport)
#else
#define TIESRDICT_API __declspec(dllimport)
#endif

#else
#define TIESRDICT_API
#endif


//--------------------------------
// The TIesrDict API may use either the old dtmakeup decision tree API
// or it may use the newer, faster and smaller TIesrDT API (recommended).
// The flag defined here determines which one is used.  If commented out,
// then the older dtmakeup is used.  Eventually dtmakeup will no longer
// be supported, and so it is deprecated.
#define _TIESRDICT_USE_TIESRDT

#ifdef _TIESRDICT_USE_TIESRDT
#include <TIesrDT_User.h>
#endif

//--------------------------------
//  CTIesrDict
//
//  The TIesrDict API class.
//
class TIESRDICT_API CTIesrDict 
{
public:

      // Enumeration of errors returned by the class
      enum Errors
      {
	 ErrNone,
	 ErrFail,
	 ErrMemory,
	 ErrInput,
	 ErrNotInDictionary
      };


      // Enumeration of flags to select how dictionary lookup is performed
      enum Flags
      {
          // Using English closure phones
          AddClosure = 0x1,

          // Do not lookup default pronunciation using decision trees
          // Binary dictionary contains full word pronunciations
          NoDTLookup = 0x2
      };


      // The phoneName is a character array containing a character
      // string holding the name of a phone.  The last character will
      // be \0.  Phone names can have a maximum length of
      // MAXPHONENAME.  This constant and typedef are provide publicly
      // for the app user.
      //
      static const unsigned int MAXPHONENAME = 10;
      typedef char phoneName[MAXPHONENAME+1];


      // No word may have more than this number of phones
      // in its pronunciation.
      static const unsigned int MAXPHONESPERWORD = 100;


   public:

      // Public interface to CTIesrDict

      // Constructor
      CTIesrDict();

      // Destructor
      ~CTIesrDict();
	

      // Load a dictionary into the TIesrDict object.  Specify the top
      // level directory, the language, and the dictionary identifier.
      // The decision trees should be in aDictDir/aLanguage, and the
      // delta-encoded dictionary and phone list should be in
      // aDictDir/aLanguage/aDictID. If the dictionary contains added stop phone
      // closures, then confFlags should have AddClosure set. Adding closures
      // is the assumed default. The confFlags may also have the NoDTLookup
      // flag set, in which case no DT lookup is performed. This is useful for
      // small vocabularies where no decision trees are necessary.
      Errors LoadDictionary( const char *aDictDir, 
			     const char *aLanguage, 
			     const char *aDictID,
                             int useFlags = AddClosure );
      

      // Return the number of phones in the loaded dictionary
      int GetPhoneCount();


      // Get the phone indexed by aIndex according to the phone list
      // in aPhoneName.
      Errors GetPhoneName( unsigned int aIndex, phoneName aPhoneName );
	

      // Get the number of entries in the dictionary for a word specified by
      // aWord.  Note that this does NOT include the rule-based default
      // pronunciation, only the number of pronunciations in the dictionary.
      // Hence the number returned in aNumberEntries can be zero.
      Errors GetNumberEntries( const char* aWord, unsigned int *aNumberEntries );


      // Get the pronunciation of the word specified by aWord.  Return the
      // indices of the phones according to the phonelist in aPron, and return
      // the pronunciation string in aPronString.  If aPronString is NULL, no
      // string is output.  aPron[0] is the number of phones in the
      // pronunciation.  The pronunciation provided by this call is either the
      // default pronunciation provided by the rule-based system, or the first
      // pronunciation located in the dictionary.  This function is supplied
      // for legacy reasons.
      Errors GetPron( const char *aWord, char aPron[], 
		      char *aPronString = NULL );


      // Get the Nth pronunciation entry of a word specified by aWord.  The
      // value of N is specified by the argument aEntryNumber.  If
      // aEntryNumber is zero, then the default rule-based pronunciation is
      // put into aPron and aPronString, without consulting the dictionary.
      // Otherwise this function will attempt to find and supply the Nth
      // pronunciation of the word in the dictionary.  If the Nth
      // pronunciation does not exist, it will output the rule-based
      // pronunciation, and will return ErrNotInDictionary.
      Errors GetPronEntry( const char *aWord, const unsigned int aEntryNumber,
			   char aPron[], char *aPronString = NULL );


      // This function should only be called after a call to GetPronEntry.  If
      // GetPronEntry has not been called, ErrFail will be returned.  This
      // function provides a means of efficiently obtaining the next
      // pronunciation of the word specified in GetPronEntry.  If the next
      // pronunciation does not exist, aPron will have zero in the first
      // character, and if aPronString is not NULL, it will be set to "", and
      // the function will return ErrNotInDictionary.  This function call is
      // provided so that dictionary search does not need to be done for each
      // of multiple pronunciations looked up in the dictionary.
      Errors GetNextEntry( char aPron[], char *aPronString = NULL );


   private:

      // class internal functions that manipulate the dictionary

      Errors read_phone( char *aFname );
      Errors read_letter_map( char* aFname, unsigned int *doLetterMap );
      Errors map_dt_letters( char *aWord );
      Errors read_binary_dictionary( char *aFname );
      int compare_str( char *str1, char *str2 );
      int inc_entry( int i );
      int mid_entry( int i1, int i2 );
      void copy_str( char *str1, char *str2 );
      void chrtoupper(char *str);
      int lookup( char *word );
      void decode_entry( char *word, int idx, char *pron);
      void print_pron( char *word, int idx, char *pron_def, char *pron);
      int get_phone_idx( char *name );
      void expand_pron( char *buf, int idx );
      void err2pron( char *pron_def, int len_def, char *pron_err, char *pron );
      void pron2bin( char *str, char *pron );
      int get_pron_offset( int idx );
      void expand_str( char *buf, int idx );
      int dec_entry( int i );
      void map_pron( char *ipron, char *opron );
      void map_phone( char *iphone, char *ophone );


      // Locate the one-based Nth dictionary pronunciation where
      // N = aEntryNumber.
      int LocatePron( char* aWord, unsigned int aEntryNumber );

      // Get default rule-based pronunciation
      Errors GetDefaultPron( const char* aWord, char aPron[] );

      // Convert a pronunciation representation into a string
      Errors PronToString( char* aPron, char* aPronString );


   private:

      // Top level directory containing decision trees, dictionary,
      // and phone list
      char * direct;
   
      // Variables that define the dictionary type
      char *lang;
      char *ident;

      // Variable that indicates dictionary contains stop closures
      int doClosure;

      // Variable that indicates default pronunciation lookup should be done
      // using TIesrDT. Else, default pronunciation defaults to empty string.
      int doDTLookup;

    // If using TIesrDT, then TIesrDict contains a TIesrDT object
#ifdef _TIESRDICT_USE_TIESRDT
    TIesrDT_t tiesrDT;
#endif

    //The pointer to the beginning of the phonetic dictionary entries
    char *dict_beg;

    // variables for doing dictionary lookup
    int last;
    int first;

    // phone list, and number of phones in the language
    phoneName *phone;
    unsigned int n_phone;


    // Worst case length of phone string for present word
    unsigned int maxString;

#ifdef _TIESRDICT_USE_TIESRDT
    // Letter to TIesrDT dictionary index map,
    // and number of letters in the language
    unsigned char letterMap[256];
    unsigned int nLetters;
    unsigned int doLetterMap;
#endif

    // The last word for which a pronunciation was looked up using
    // GetPronEntry.
    char* m_word;

    // The number of the entry for the pronunciation last requested by
    // GetPronEntry or GetNextEntry.
    unsigned int m_entryNumber;

    // The entry index in the dictionary of the last word looked up using
    // GetPronEntry or GetNextEntry
    int m_entryLocation;

    // Default pronunciation for last word looked up using GetPronEntry or
    // GetNextEntry.
    char* m_defaultPron;

};



#endif // _TIESRDICT_H
