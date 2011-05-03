/*=======================================================================

 *
 * TIesrDT.c
 *
 * TIesrDT API - determines word pronunciation from spelling.
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

 This file contains the source code that implements the TIesrDT
 decision tree API.

======================================================================*/


/* Windows specific include */
#if defined( WIN32 ) || defined( WINCE )
#include <windows.h>
#define strdup _strdup
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "TIesrDT_User.h"
#include "TIesrDT.h"


#if defined( WIN32 ) || defined( WINCE )
/* Define Windows entry point for TIesrDT API */
BOOL APIENTRY DllMain( HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
#endif


/*----------------------------------------------------------------
 TIesrDT_Create

 This function creates a TIesrDT_Struct object and initializes it
 with a list of attribute values and a list of phones.

 ----------------------------------------------------------------*/
TIESRDT_API
TIesrDT_Error_t TIesrDT_Create( TIesrDT_t* aTIesrDTPtr,
const char* aTreePath )
{
   TIesrDT_t TIesrDT;
   TIesrDT_Error_t dtError;
   int index;
   
   /* Create the structure for this instance */
   TIesrDT = (TIesrDT_t)malloc( sizeof(TIesrDT_Struct_t) );
   if( ! TIesrDT )
   {
      *aTIesrDTPtr = NULL;
      return TIesrDTErrorMemory;
   }
   
   
   /* Initially set decision trees not preloaded */
   TIesrDT->preLoaded = FALSE;
   
   /* initialize the treePointers and charAtt arrays */
   for( index = 0; index <= MAX_ASCII; index++ )
   {
      TIesrDT->treePointers[index] = NULL;
      TIesrDT->charAtt[index] = UNUSED_ATT;
   }
   
   
   /* Initialize phone string and tree structure pointers */
   for( index = 0; index < NUM_PHONES; index++ )
   {
      TIesrDT->phone[index] = NULL;
   }
   
   TIesrDT->tree = NULL;
   
   TIesrDT->root.AttVal = NULL;
   
   
   /* Obtain the path containing decision tree files */
   TIesrDT->treePath = strdup( aTreePath );
   if( ! TIesrDT->treePath )
   {
      free( TIesrDT );
      *aTIesrDTPtr = NULL;
      return TIesrDTErrorMemory;
   }
   
   
   /* Load the character attribute value indices into the character
    attribute array. */
   dtError = LoadAttributes( TIesrDT );
   if( dtError != TIesrDTErrorNone )
   {
      TIesrDT_Destroy( TIesrDT );
      *aTIesrDTPtr = NULL;
      return dtError;
   }
   
   
   /* Load the phone set and phone class attribute array */
   dtError = LoadPhones( TIesrDT );
   if( dtError != TIesrDTErrorNone )
   {
      TIesrDT_Destroy( TIesrDT );
      *aTIesrDTPtr = NULL;
      return dtError;
   }
   

   
   /* Everything initialized successfully */
   *aTIesrDTPtr = TIesrDT;
   return TIesrDTErrorNone;
}


/*----------------------------------------------------------------
 TIesrDT_CreatePreload

 This function creates a TIesrDT_Struct object and initializes it
 with a list of attribute values and a list of phones.  It also
 preloads all of the decision trees.
 ----------------------------------------------------------------*/
TIESRDT_API
TIesrDT_Error_t TIesrDT_CreatePreload( TIesrDT_t* aTIesrDTPtr,
const char* aTreePath )
{
   TIesrDT_t TIesrDT;
   TIesrDT_Error_t dtError;
   
   /* Try to create the TIesrDT instance prior to preload */
   dtError = TIesrDT_Create( aTIesrDTPtr, aTreePath );
   if( dtError != TIesrDTErrorNone )
   {
      return dtError;
   }
   
   TIesrDT = *aTIesrDTPtr;
   
   
   /* Preload the decision trees */
   
   dtError = PreLoadTrees( TIesrDT );
   if( dtError != TIesrDTErrorNone )
   {
      TIesrDT_Destroy( TIesrDT );
      *aTIesrDTPtr = NULL;
      return TIesrDTErrorLoad;
   }
   
   /* Trees loaded successfully */
   TIesrDT->preLoaded = TRUE;
   
   return TIesrDTErrorNone;
}


/*----------------------------------------------------------------
 PreLoadTrees

 This function pre-loads all of the decision trees on the heap,
 thus making them available for processing of all letters of all
 words without loading them from files again.
 ----------------------------------------------------------------*/
TIesrDT_Error_t PreLoadTrees( TIesrDT_t aTIesrDT )
{
   int ascii;
   FILE* fp = NULL;
   char fileName[MAX_FILENAME];
   TIesrDT_Error_t dtError;
   int treeBytes;
   unsigned char numBranches;
   size_t numRead;

   
   /* Process the trees for each character attribute */
   for( ascii = 0; ascii <= MAX_ASCII; ascii++ )
   {
      /* If the ascii character has an attribute index, this ascii character
       has a decision tree */
      if( aTIesrDT->charAtt[ascii] != UNUSED_ATT )
      {
         dtError = GetTreeFileName( aTIesrDT, (char)ascii, fileName );
         if( dtError != TIesrDTErrorNone )
            goto FailedPreLoad;
         
         fp = fopen( fileName, "rb");
         if( ! fp )
            goto FailedPreLoad;
         
         numRead = fread( &treeBytes, sizeof(int), 1, fp);
         numRead = fread( &numBranches, sizeof(unsigned char), 1, fp);
         if( numRead != 1)
            goto FailedPreLoad;

         /* Tree size must include initial int specifying size of tree structure
            and must include branch information */
         treeBytes +=  4 + (numBranches + 1);
         
         /* Allocate space for the tree */
         aTIesrDT->treePointers[ascii] = (unsigned char*)malloc( treeBytes*sizeof(unsigned char));
         if( aTIesrDT->treePointers[ascii] == NULL )
            goto FailedPreLoad;

         
         /* Go to beginning of tree file and read it */
         fseek( fp, 0, SEEK_SET );
         numRead = fread( aTIesrDT->treePointers[ascii], sizeof(unsigned char), treeBytes, fp);
         if( numRead != treeBytes)
            goto FailedPreLoad;

         fclose(fp);
      }
   }
   
   return TIesrDTErrorNone;
   
   
   /* A tree could not be loaded, so remove all preloaded trees */
   FailedPreLoad:
      if( fp )
         fclose(fp);
      
      for( ascii = 0; ascii <= MAX_ASCII; ascii++ )
      {
         if( aTIesrDT->treePointers[ascii] != NULL )
         {
            free( aTIesrDT->treePointers[ascii] );
            aTIesrDT->treePointers[ascii] = NULL;
         }
      }
      return TIesrDTErrorFail;
}



/*----------------------------------------------------------------
 TIesrDT->Pron

 This function returns the pronunciation string corresponding to an
 input word.  The user of this function provides a string array and
 its size, into which the function will place the pronunciation.

 ----------------------------------------------------------------*/
TIESRDT_API
TIesrDT_Error_t TIesrDT_Pron( TIesrDT_t aTIesrDT,
const char *aWord,
char *aPronString, size_t aStringSize )
{
   TIesrDT_Error_t dtError;
   unsigned char noAtt;
   char* word;
   char* wordCopy;
   short chr;
   short nCharacters;
   short pos;
   unsigned char context[DT_CONTEXT_SIZE];
   short cxtIndex;
   char* phone;
   unsigned char phoneIndex;
   short phoneLen;
   short pronLen;
   
   
   /* Clear initial pronunciation */
   aPronString[0] = '\0';
   
   
   /* The "NO_ATTRIBUTE" attribute, which is always the last class attribute. */
   noAtt = aTIesrDT->classAtt[NUM_PHONE_CLASS - 1];
   
   
   /* Initialize the prior phone class context */
   for( cxtIndex = DT_CONTEXT_SIZE - DT_LEFT_CLASS;
   cxtIndex < DT_CONTEXT_SIZE; cxtIndex++ )
   {
      context[cxtIndex] = noAtt;
   }
   
   
   /* Make a local copy of the word, and prepare it. */
   wordCopy = strdup(aWord);
   if( wordCopy == NULL )
   {
      return TIesrDTErrorMemory;
   }
   
   /* Remove leading blanks from the word, and use only first part of word */
   word = Deblank( wordCopy );
   
   /* Convert word to upper case */
   ToUpper( word );
   
   
   /* Loop over all characters of the word, getting phones for each character */
   nCharacters = strlen(word);
   for( chr = 0; chr < nCharacters; chr++ )
   {
      
      /* Determine the character's context within the word */
      cxtIndex = 0;
      
      /* Left character context */
      for( pos = chr - DT_LEFT_CHAR; pos < chr; pos++ )
      {
         context[cxtIndex++] = ( pos < 0 || pos >= nCharacters ) ?
         noAtt  :  aTIesrDT->charAtt[ (unsigned char)word[pos] ];
      }
      
      /* Right character context */
      for( pos = chr + 1; pos <= chr + DT_RIGHT_CHAR;  pos++ )
      {
         context[cxtIndex++] = ( pos < 0 || pos >= nCharacters ) ?
         noAtt  :  aTIesrDT->charAtt[ (unsigned char)word[pos] ];
      }
      
      
      /* Get the phone index from the decision tree for this character
       based on this character's context. */
      dtError = GetDTPhoneIndex( aTIesrDT, word[chr], context, &phoneIndex );
      if( dtError != TIesrDTErrorNone )
      {
         free( wordCopy );
         return dtError;
      }
      
      /* Determine the phone and its string length */
      phone = aTIesrDT->phone[phoneIndex];
      phoneLen = strlen(phone);
      
      /* Concatenate phone to the pronunciation string if the phone
       is not all blanks */
      if( strspn(phone, " ") != phoneLen )
      {
         
         /* Must check to ensure the string size has not been used up */
         pronLen = strlen( aPronString );
         if( aStringSize >= (size_t)(pronLen + phoneLen + 2) )
         {
            if( pronLen > 0 ) strcat( aPronString, " " );
            strcat( aPronString, phone );
         }
         else
         {
            free( wordCopy );
            return TIesrDTErrorMemory;
         }
      }
      
      
      /* If more characters remain to be processed, update left phone
       class context */
      if( DT_LEFT_CLASS > 0 && chr < nCharacters - 1 )
      {
         pos = DT_CONTEXT_SIZE - DT_LEFT_CLASS;
         while( pos < DT_CONTEXT_SIZE - 1 )
         {
            context[pos] = context[pos+1];
            pos++;
         }
         context[pos] = aTIesrDT->phoneAtt[phoneIndex];
      }
   }
   
   /* Finished determining word pronunciation */
   free( wordCopy );
   return TIesrDTErrorNone;
}



/*----------------------------------------------------------------
 TIesrDT_Destroy

 This function destroys the contents of the TIesrDT object that
 was created with TIesrDT_Create.  After this function is called,
 the pointer to the TIesrDT object is no longer valid.

 ----------------------------------------------------------------*/
TIESRDT_API
TIesrDT_Error_t TIesrDT_Destroy( TIesrDT_t aTIesrDT )
{
   int index;
   
   /* Free phone list strings */
   for( index = 0; index < NUM_PHONES; index++ )
   {
      if( aTIesrDT->phone[index] != NULL )
      {
         free( aTIesrDT->phone[index] );
         aTIesrDT->phone[index] = NULL;
      }
   }
   
   
   /* Free preloaded trees, or any remaining individual tree */
   if( aTIesrDT->preLoaded )
   {
      for (index = 0; index <= MAX_ASCII; index++ )
      {
         if( aTIesrDT->treePointers[index] != NULL )
         {
            free( aTIesrDT->treePointers[index] );
            aTIesrDT->treePointers[index] = NULL;
         }
      }
   }
   else
   {
      /* Free any existing tree */
      if( aTIesrDT->tree != NULL )
      {
         free( aTIesrDT->tree );
         aTIesrDT->tree = NULL;
      }
   }
   
   if (aTIesrDT->root.AttVal)
   {
      free(aTIesrDT->root.AttVal);
      aTIesrDT->root.AttVal = NULL;
   }
   
   /* Free any existing tree path */
   if( aTIesrDT->treePath != NULL )
   {
      free( aTIesrDT->treePath );
      aTIesrDT->treePath = NULL;
   }
   
   /* destroy the TIesrDT object itself */
   free( aTIesrDT );
   
   return TIesrDTErrorNone;
}

/*----------------------------------------------------------------
 LoadAttributes

 This function loads the character attributes value array in the
 TIesrDT structure, and the class attribute value array.  The
 character attributes value array, charAtt, is an unsigned byte
 array, indexed by the ASCII value of the character.  It returns the
 attribute value corresponding to a character, for example,
 charAtt['A'].  The charAtt array is sized to hold attribute values
 for ASCII characters up to and including ASCII value MAX_ASCII.
 However, the decision tree code is such that it limits the number of
 attribute values that can be assigned to NUM_ATT_VAL.  Thus some
 charAtt array entries may not be used.

 The class attribute value array classAtt, specifies that attribute value
 for each of the class attributes; voiced, unvoiced, etc.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t LoadAttributes( TIesrDT_t aTIesrDT )
{
   FILE* fp;
   char* attFile;
   short isClassAtt;
   unsigned char attString[MAX_STRING];
   short nClass;
   
   /* Open the ascii character attribute file */
   attFile = (char*)malloc( strlen( aTIesrDT->treePath ) +
   strlen(FN_ATTVALUE) + 2 );
   if( ! attFile )
      return TIesrDTErrorMemory;
   
   strcpy( attFile, aTIesrDT->treePath );
   strcat( attFile, "/" );
   strcat( attFile, FN_ATTVALUE );
   
   fp = fopen( attFile, "r" );
   if( ! fp )
   {
      free( attFile );
      return TIesrDTErrorFile;
   }
   
   
   /* Read all attributes and assign values to the attributes. */
   
   aTIesrDT->numAtt = 0;
   while(  fscanf( fp, "%s", attString ) != EOF )
   {
      
      /* Can not have more than allowed number of attributes */
      if( aTIesrDT->numAtt >= NUM_ATT_VAL )
      {
         fclose(fp);
         free( attFile );
         return TIesrDTErrorLoad;
      }
      
      
      /* Check for class attribute, and if so assign its value */
      isClassAtt = FALSE;
      for( nClass = 0; nClass < NUM_PHONE_CLASS; nClass++ )
      {
         if( ! strcmp( (const char *)attString, PHONECLASSSTRING[nClass] ) )
         {
            isClassAtt = TRUE;
            aTIesrDT->classAtt[nClass] = (unsigned char)aTIesrDT->numAtt++;
            break;
         }
      }
      
      /* Present attribute is phone class attribute, go get next attribute. */
      if( isClassAtt )
         continue;
      
      
      /* The attribute must be a character attribute.  Assign its value. */
      if( strlen( (const char *)attString) != 1 || attString[0] > MAX_ASCII )
      {
         fclose(fp);
         free( attFile );
         return TIesrDTErrorLoad;
      }
      
      aTIesrDT->charAtt[ attString[0] ] = (unsigned char)aTIesrDT->numAtt++;
   }
   
   /* Assigned all attribute values successfully */
   fclose( fp );
   free( attFile );
   return TIesrDTErrorNone;
}



/*----------------------------------------------------------------
 LoadPhones

 This function loads the phones and their respective class attributes
 from the FN_NEWPHONE file into the TIesrDT structure. The phone
 string array provides the phone string corresponding to the phone
 index value used in the decision tree.  The phone class array
 indicates whether a phone has a particular class value, such as
 voiced, unvoiced, or short pause.

 The FN_NEWPHONE file has the format one entry per line consisting of
 a phone string followed by whitespace followed by the phone's class
 string.  The class string is one of the strings in the
 phoneClassString array.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t LoadPhones( TIesrDT_t aTIesrDT )
{
   FILE* fp;
   char* phoneFile;
   char phoneString[MAX_STRING];
   char classString[MAX_STRING];
   int isClass;
   int nClass;
   int numRead;
   char* phone;
   
   /* Open the phone list file */
   phoneFile = (char*)malloc( strlen( aTIesrDT->treePath ) + strlen(FN_NEWPHONE) + 2 );
   if( ! phoneFile )
      return TIesrDTErrorMemory;
   
   strcpy( phoneFile, aTIesrDT->treePath );
   strcat( phoneFile, "/" );
   strcat( phoneFile, FN_NEWPHONE );
   
   fp = fopen( phoneFile, "r" );
   if( ! fp )
   {
      free( phoneFile );
      return TIesrDTErrorFile;
   }
   
   
   /* Read phones, and their classes which indicate voiced, unvoiced,
    short pause, or perhaps other classes in the future. */
   aTIesrDT->numPhones = 0;
   
   numRead = fscanf( fp, "%s %s", phoneString, classString );
   while( numRead != EOF )
   {
      /* Must have both phoneString and classString. */
      if( numRead != 2 )
      {
         fclose(fp);
         free( phoneFile );
         return TIesrDTErrorLoad;
      }
      
      /* Save the phone string ordered by location in file */
      aTIesrDT->phone[aTIesrDT->numPhones] = strdup( phoneString );
      if( aTIesrDT->phone[aTIesrDT->numPhones] == NULL )
      {
         fclose(fp);
         free( phoneFile );
         return TIesrDTErrorMemory;
      }
      
      /* Change '_' in a pseudo-phone to a space, which will make
       output faster, if many words are being looked up */
      for( phone = aTIesrDT->phone[aTIesrDT->numPhones];
      *phone; phone++ )
      {
         if( *phone == '_' )
            *phone = ' ';
      }
      
      
      /* Determine the class of the phone, and assign its attribute */
      isClass = FALSE;
      for( nClass = 0; nClass < NUM_PHONE_CLASS; nClass++ )
      {
         if( ! strcmp( classString, PHONECLASSSTRING[nClass] ) )
         {
            isClass = TRUE;
            aTIesrDT->phoneAtt[aTIesrDT->numPhones] =
            aTIesrDT->classAtt[nClass];
            break;
         }
      }
      
      
      /* Phone must have a valid class attribute */
      if( ! isClass )
      {
         fclose(fp);
         free( phoneFile );
         return TIesrDTErrorLoad;
      }
      
      
      /* Completed reading the current phone and class. Go to next one. */
      aTIesrDT->numPhones++;
      numRead = fscanf( fp, "%s %s", phoneString, classString );
   }
   
   
   /* Completed loading phones and classes successfully */
   fclose( fp );
   free( phoneFile );
   return TIesrDTErrorNone;
}


/*----------------------------------------------------------------
 LoadTreeFile

 This function loads the tree file corresponding to the given
 character from the path in the TIesrDT object, and puts the pointer
 to the tree in the TIesrDT object.  If the trees have not been preloaded 
 as indicated by the TIesrDT preLoaded flag not being set, then space will
 be allocated for the entire tree, and it will be loaded into heap memory.
 Otherwise, pointers to the proper tree data will be set in the TIesrDT
 object without loading any tree data from a file.

 Now loads *.olmdtpm
 ----------------------------------------------------------------*/
static TIesrDT_Error_t LoadTreeFile( TIesrDT_t aTIesrDT, char aChar )
{
   char treeFile[MAX_FILENAME];
   int treeBytes;
   size_t numRead;
   FILE* fp;
   unsigned char nBranch;
   unsigned char nValue;
   short iTmp;
   int fromFile;
   unsigned char *treePtr;
   TIesrDT_Error_t dtError;
   
   
   /* Determine if tree info comes from file or preLoaded info */
   fromFile = ! aTIesrDT->preLoaded;
   
   /* Open a file pointer to the tree to preload or load from file */
   if( fromFile )
   {
      dtError = GetTreeFileName( aTIesrDT, aChar, treeFile );
      
      /* Open the tree file if possible */
      fp = fopen( treeFile, "rb" );
      if( ! fp )
      {
         return TIesrDTErrorFail;
      }
   }
   else
   {
      /* Tree comes from preLoaded location */
      treePtr = aTIesrDT->treePointers[(int)aChar];
   }
   
   
   /* Size of the tree exclusive of branch info */
   if( fromFile )
   {
      numRead = fread( &treeBytes,  sizeof(int), 1, fp );
      if( numRead  != 1 )
      {
         fclose( fp );
         return TIesrDTErrorLoad;
      }
   }
   else
   {
      treeBytes = *( (int*)treePtr );
      treePtr += sizeof(int);
   }
   
   
   
   /* Number of branches */
   if( fromFile )
   {
      numRead = fread(&nBranch, sizeof(char), 1, fp);
      if( numRead  != 1 )
      {
         fclose( fp );
         return TIesrDTErrorLoad;
      }
   }
   else
      nBranch = *treePtr++;

   aTIesrDT->stAttInfo.numAttTypes = (short)nBranch;
   
  
   /* Branch info */
   for (iTmp = 0; iTmp < nBranch; iTmp++)
   {
      if( fromFile )
      {
         numRead = fread(&nValue, sizeof(char), 1, fp);
         if( numRead  != 1 )
         {
            fclose( fp );
            return TIesrDTErrorLoad;
         }
      }
      else
         nValue = *treePtr++;
      
      aTIesrDT->stAttInfo.numAttValPerAttType[iTmp] = (short) nValue;
   }
   
   
   /* Read tree from file or get pointer to it. */
   if( fromFile )
   {
      aTIesrDT->tree = (unsigned char*)malloc( treeBytes*sizeof(unsigned char) );
      if( ! aTIesrDT->tree )
      {
         fclose( fp );
         return TIesrDTErrorMemory;
      }
      
      numRead = fread( aTIesrDT->tree, sizeof(unsigned char), treeBytes, fp );
      fclose(fp);
      if( numRead != treeBytes )
      {
         free( aTIesrDT->tree );
         aTIesrDT->tree = NULL;
         return TIesrDTErrorLoad;
      }
   }
   else
   {
      aTIesrDT->tree = treePtr;
   }

   aTIesrDT->treeSize = treeBytes;
   return TIesrDTErrorNone;
}

/*-----------------------------------------------------------------
 GetTreeFileName

 Get the name of the file holding the decision tree for a character
 attribute.
 ------------------------------------------------------------------*/
static TIesrDT_Error_t GetTreeFileName( TIesrDT_t aTIesrDT, char aChar, char* aFileName )
{
   int isSpecial;
   int chrIndex;
   int pos;

   
   strcpy( aFileName, aTIesrDT->treePath );
   strcat( aFileName, "/" );
   
   /* Must determine if the character is a special character that has a special
    file name associated with it. */
   isSpecial = FALSE;
   for( chrIndex = 0; chrIndex < NUM_FN_CHAR; chrIndex++ )
   {
      if( aChar == FNCHAR[chrIndex] )
      {
         isSpecial = TRUE;
         strcat( aFileName, FNNAME[chrIndex] );
         break;
      }
   }
   
   /* If not special, then use character as file name */
   if( ! isSpecial )
   {
      pos = strlen( aFileName );
      aFileName[pos++] = aChar;
      aFileName[pos] = '\0';
   }
   
   strcat( aFileName, EXT_LMDTPM );
   return TIesrDTErrorNone;
}


/*----------------------------------------------------------------
 AtLeaf

 This function determines if the present offset is at a leaf node
 set, which is indicated by the top bit of the current byte having a
 zero.  This can be put in a macro later, but for now it is a
 function for debugging.

 ----------------------------------------------------------------*/
static short AtLeaf( const unsigned char* aTree, int aOffset )
{
   unsigned char byte;
   
   byte = aTree[ aOffset ];
   
   return ( ( byte & LEAFBIT ) == 0 );
}

static eNodeType TypeOfNode( const unsigned char* aTree, int aOffset )
{
   unsigned char byte;
   
   byte = aTree[ aOffset ];
   
   if (( byte & LEAFBIT ) == 0 )
      return LEAFSET;
   else
   {
      if (byte & NODE_QUESTION)
         return BRANCHSET;
      else
         return QUESTION;
   }
}


/*----------------------------------------------------------------
 GetBranchInfo

 This function retrieves the information encoded in the bit fields
 of the two-byte branch node pointed to by the offset into the tree.

 ----------------------------------------------------------------*/
// for loading *.rlmdtpm
#ifdef DEV_OFFSET
static TIesrDT_Error_t GetBranchInfo( const unsigned char* aTree,
int aOffset,
unsigned short* aBranchOffset,
unsigned char* aBranchLeafContext,
unsigned int * pSize )
{
   unsigned short branch;
   
   if( ( aTree[aOffset] | LEAFBIT ) == 0 )
      return TIesrDTErrorFail;
   
   branch = aTree[aOffset] << 8;
   branch |= (unsigned short) aTree[aOffset+1];
   
   *aBranchOffset = ( branch & BRANCHOFFSET )  ;
   *aBranchLeafContext = (( branch & BRANCHLEAFCONTEXT_RLM ) >> BLEAFCXTSHIFT_SHIFT);
   
   if (pSize) (*pSize) += sizeof(short); // increase number of bytes
   return TIesrDTErrorNone;
}

/* GetQuestion

 return the question of a leaf set
 @param aTree pointer to the tree of the decision tree
 @param aOffset offset in the tree of the decision tree
 @param aLeafContext pointer to the returned leaf context (question)
 @param aptrISize pointer the offset after reading the information
 */
static TIesrDT_Error_t GetQuestion( const unsigned char* aTree,
int aOffset,
unsigned char* aLeafContext,
unsigned int * aptrISize )
{
   unsigned char branch;
   
   if( ( aTree[aOffset] | LEAFBIT ) == 0 )
      return TIesrDTErrorFail;
   
   branch = (aTree[aOffset] &  QUESTION_IN_QUESTION_NODE);
   *aLeafContext = branch;
   
   if (aptrISize) (*aptrISize) += sizeof(unsigned char); // increase number of bytes
   return TIesrDTErrorNone;
}

#else
static TIesrDT_Error_t GetBranchInfo( const unsigned char* aTree,
int aOffset,
unsigned char* aBranchPhone,
unsigned char* aBranchAttribute,
unsigned char* aBranchLeafContext,
unsigned int * pSize )
{
   unsigned short branch;
   
   
   if( ( aTree[aOffset] | LEAFBIT ) == 0 )
      return TIesrDTErrorFail;
   
   branch = aTree[aOffset] << 8;
   branch |= (unsigned short) aTree[aOffset+1];
   
   *aBranchPhone = ( branch & BRANCHPHONE )  ;
   *aBranchAttribute = (( branch & BRANCHATTRIBUTE_RLM ) >> BATTSHIRT);
   *aBranchLeafContext = (( branch & BRANCHLEAFCONTEXT_RLM ) >> BLEAFCXTSHIFT_RLM);
   
   if (pSize) (*pSize) += sizeof(short); // increase number of bytes
   return TIesrDTErrorNone;
}

/*----------------------------------------------------------------
 FindNextTopNode

 This function searches through the tree, starting at the global
 offset location, for the next leaf node set or branch node
 corresponding to the top branch node context.

 ---------------------------------------------------------------*/
static TIesrDT_Error_t FindNextTopNode( TIesrDT_t aTIesrDT )
{
   TIesrDT_Error_t dtError;
   unsigned char* tree;
   int offset;
   
   unsigned char topContext;
   
   unsigned char branchContext;
   unsigned char branchAttribute;
   unsigned char branchLeafContext;
   
   
   /* Start from global offset location */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   topContext = aTIesrDT->topBranchContext;
   
   /* Search through tree. */
   while( offset < aTIesrDT->treeSize )
   {
      /* If at a leaf node set, then this set is outside of a branch */
      if( AtLeaf( tree, offset ) )
      {
         aTIesrDT->treeOffset = offset;
         return TIesrDTErrorNone;
      }
      
      
      /* At a branch node. */
      else
      {
         /* Check if branch is a top context branch */
         dtError = GetBranchInfo( tree, offset,
         &branchContext,
         &branchAttribute,
         &branchLeafContext , NULL );
         
         if( branchContext == topContext )
         {
            aTIesrDT->treeOffset = offset;
            return TIesrDTErrorNone;
         }
         
         else
         {
            /* Go past branch node, and check for end of file, which
             should not happen since all branches should have either
             other branch nodes or leaf nodes under them. */
            offset+=2;
            if( offset >= aTIesrDT->treeSize )
               return TIesrDTErrorFail;
            
            
            /* skip past the leaf node set corresponding to this branch,
             if it exists. */
            if( AtLeaf( tree, offset ) )
            {
               SkipPastLeafSet( tree, &offset , NULL);
            }
            
         }
      }
   }
   
   
   /* Could not find a next top node. This should never happen. */
   return TIesrDTErrorFail;
}


/*----------------------------------------------------------------
 SearchTopLeafSet

 The current offset into the tree is at a leaf node set that should
 be compared with the top node context to determine if any of the
 leaf nodes contain a match to the context.  If a node matches the
 context, then output the node's phone index.  Otherwise, there is
 not a match, and the search must continue.

 If the top branch context has not yet been defined, then a search
 will be made for the context of the top branch prior to doing a
 search for a context match.  If no top context exists, then it is
 assumed that the tree is degenerate, that is it contains only one
 leaf node which defines the phone index to output.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t SearchTopLeafSet( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short* aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   
   
   dtError = SearchLeafSet( aTIesrDT, aContext,
   aTIesrDT->topBranchContext,
   aPhoneIndex , NULL);
   
   return dtError;
}

#endif

/*----------------------------------------------------------------
 SearchLeafSet

 This function searches a leaf set to determine if any of the leaf
 nodes of the set contain the attribute value that matches the
 specified context.

 ----------------------------------------------------------------*/
#ifdef DEV_OFFSET
static TIesrDT_Error_t  SearchLeafSet( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short aMatchContext,
short *aPhoneIndex )
{
   unsigned char *tree;
   int offset;
   unsigned char matchAttribute = CXT_NONE;
   short nodeCount;
   unsigned char phoneIndex;
   short numNodeWithPhone;
   short attCount;
   char nodeAtt, lastAtt;
   short iLastRead, sizeLeafSet;
   
   
   /* Find the word context attribute defining a match.  If aMatchContext is
    CXT_NONE, then the first leaf node is the only leaf node and it matches
    by default. */
   if( aMatchContext != CXT_NONE )
   {
      matchAttribute = aContext[aMatchContext];
   }
   
   /* Search the leaf node set pointed to by the global tree offset
    * for a match */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   iLastRead = offset;
   
   /* Number of leaf nodes in set */
   sizeLeafSet = (short)tree[offset++];
   
   /* Check leaf nodes corresponding to each phone index in the set */
   nodeCount = 0;
   while( (offset - iLastRead) < sizeLeafSet )
   {
      /* Phone index for a set of attribute values */
      phoneIndex = tree[offset++];
      
      if( aMatchContext == CXT_NONE ||
      (offset - iLastRead == sizeLeafSet) )
      {
         /* Two conditions to return phone:
          1) Context free phone
          2) the last phone of the set.
          In the second case, the letters and phone classes do not need to be saved */
         *aPhoneIndex = phoneIndex;
         return TIesrDTErrorNone;
      }
      
      /* Number of leaf nodes with attribute values for this phone */
      numNodeWithPhone = tree[offset++];
      
      attCount = 0;
      while( attCount < numNodeWithPhone )
      {
         /* Attribute value, or if negative, a negative value
          indicating the end of a sequence of attribute values
          corresponding to the present phone index. */
         nodeAtt = (char)tree[offset++];
         
         /* Individual attribute value or start of sequence. */
         if( ! (nodeAtt & 0x80) )
         {
            
            /* Found a match, so done with the tree */
            if( matchAttribute == nodeAtt )
            {
               /* Don't have to do this, since done with the tree.  Its here
                just to remind that there has been an offset.
               
                TIesrDT->treeOffset = offset; */
               
               *aPhoneIndex = phoneIndex;
               return TIesrDTErrorNone;
            }
            
            /* Increment attribute count, and keep track of this
             attribute value in case the next attribute is negative
             indicating an attribute sequence. */
            attCount++;
            lastAtt = nodeAtt;
         }
         
         /* nodeAtt indicates end of a sequence of attribute values */
         else
         {
            
            /* Check if the match attribute is contained in the sequence.
             If so, found a match and done with the tree. */
            if( matchAttribute > lastAtt && matchAttribute <= (nodeAtt & 0x7f) )
            {
               /* Don't have to do this, since done with the tree.  Its here
                just to remind that there has been an offset.
               
                TIesrDT->treeOffset = offset; */
               
               *aPhoneIndex = phoneIndex;
               return TIesrDTErrorNone;
            }
            
            /* Update count of attributes encountered for this phone
             given by the attribute sequence. */
            attCount += ( (nodeAtt & 0x7f)  - lastAtt );
         }
      }
      
      nodeCount += numNodeWithPhone;
   }
   
   
   /* Did not find a match in any of the leaf nodes */
   aTIesrDT->treeOffset = offset;
   *aPhoneIndex = -1;
   
   return TIesrDTErrorNone;
}

#else
static TIesrDT_Error_t  SearchLeafSet( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short aMatchContext,
short *aPhoneIndex ,
short * ptrNumLeaves )
{
   unsigned char *tree;
   int offset;
   
   unsigned char matchAttribute;
   
   short numLeafNodes;
   short nodeCount;
   unsigned char phoneIndex;
   short numNodeWithPhone;
   short attCount;
   char nodeAtt;
   
   
   /* Find the word context attribute defining a match.  If aMatchContext is
    CXT_NONE, then the first leaf node is the only leaf node and it matches
    by default. */
   if( aMatchContext != CXT_NONE )
   {
      matchAttribute = aContext[aMatchContext];
   }
   
   /* Search the leaf node set pointed to by the global tree offset
    * for a match */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   /* Number of leaf nodes in set */
   numLeafNodes = (short)tree[offset++];
   
   /* Check leaf nodes corresponding to each phone index in the set */
   nodeCount = 0;
   while( nodeCount < numLeafNodes )
   {
      /* Phone index for a set of attribute values */
      phoneIndex = tree[offset++];
      
      /* Number of leaf nodes with attribute values for this phone */
      numNodeWithPhone = tree[offset++];
      
      attCount = 0;
      while( attCount < numNodeWithPhone )
      {
         /* Attribute value, or if negative, a negative value
          indicating the end of a sequence of attribute values
          corresponding to the present phone index. */
         nodeAtt = (char)tree[offset++];
         
         /* Individual attribute value or start of sequence. */
         if( (nodeAtt & 0x80) == 0)
         {
            
            /* Found a match, so done with the tree */
            if( aMatchContext == CXT_NONE || matchAttribute == nodeAtt )
            {
               /* Don't have to do this, since done with the tree.  Its here
                just to remind that there has been an offset.
               
                TIesrDT->treeOffset = offset; */
               
               *aPhoneIndex = phoneIndex;
               return TIesrDTErrorNone;
            }
            
         }
         
         /* nodeAtt indicates end of a sequence of attribute values */
         else
         {
            
            /* Check if the match attribute is contained in the sequence.
             If so, found a match and done with the tree. */
            //	    if( matchAttribute > lastAtt && matchAttribute <= (0x7f & nodeAtt) )
            if( matchAttribute == (0x7f & nodeAtt) )
            {
               /* Don't have to do this, since done with the tree.  Its here
                just to remind that there has been an offset.
               
                TIesrDT->treeOffset = offset; */
               
               *aPhoneIndex = phoneIndex;
               return TIesrDTErrorNone;
            }
            
         }
         
         attCount ++;
      }
      
      nodeCount += numNodeWithPhone;
   }
   
   
   /* Did not find a match in any of the leaf nodes */
   aTIesrDT->treeOffset = offset;
   *aPhoneIndex = -1;
   
   if (ptrNumLeaves)
      *ptrNumLeaves = numLeafNodes;
   
   return TIesrDTErrorNone;
}
#endif

#ifdef DEV_OFFSET
/*  SearchBranchSet

 This function decode an integer index of a branch that is
 embedded into a run-length code in branchset.
 For example, branches with letter A, B, C will be compressed to
 A, -C. If the branch with letter B is to be locataed, its index is
 obtained as B-A = 1, which is the second branch with index 1.
 @param aTIesrDT pointer to TIesrDT
 @param aContext char pointer to context
 @param aMatchContext the requested context
 @param aBranchIndex pointer to branch index to be returned
 */
static TIesrDT_Error_t  SearchBranchSet( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short aMatchContext,
short *aBranchIndex )
{
   unsigned char *tree;
   int offset;
   unsigned char matchAttribute;
   short attCount = 0;
   char nodeAtt;
   short lastAtt;
   short sizeBranchSet;
   short byteCount;
   
   /* Search the leaf node set pointed to by the global tree offset
    * for a match */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   *aBranchIndex = -1;
   
   /* Number of bytes of the branch set */
   sizeBranchSet = (short)(tree[offset++] & ((~NODE_BRANCHSET) & 0xff));
   
   if( aMatchContext != CXT_NONE )
   {
      matchAttribute = aContext[aMatchContext];
   }
   
   /* Check leaf nodes corresponding to each phone index in the set */
   byteCount = 1;
   while( byteCount < sizeBranchSet )
   {
      /* Attribute value, or if negative, a negative value
       indicating the end of a sequence of attribute values
       corresponding to the present phone index. */
      nodeAtt = (char)tree[offset++];
      byteCount++;
      
      /* Individual attribute value or start of sequence. */
      if( ! (nodeAtt & 0x80) )
      {
         
         /* Found a match, so done with the tree */
         if( matchAttribute == nodeAtt )
         {
            /* Don't have to do this, since done with the tree.  Its here
             just to remind that there has been an offset.
            
             TIesrDT->treeOffset = offset; */
            
            *aBranchIndex = attCount;
            break;
         }
         
         /* Increment attribute count, and keep track of this
          attribute value in case the next attribute is negative
          indicating an attribute sequence. */
         attCount++;
         lastAtt = nodeAtt;
      }
      
      /* nodeAtt indicates end of a sequence of attribute values */
      else
      {
         
         /* Check if the match attribute is contained in the sequence.
          If so, found a match and done with the tree. */
         if( matchAttribute > lastAtt && matchAttribute <= (nodeAtt & 0x7f) )
         {
            /* Don't have to do this, since done with the tree.  Its here
             just to remind that there has been an offset.
            
             TIesrDT->treeOffset = offset; */
            
            attCount += (matchAttribute - lastAtt - 1);
            *aBranchIndex = attCount;
            
            break;
         }
         
         /* Update count of attributes encountered for this phone
          given by the attribute sequence. */
         attCount += ( (nodeAtt & 0x7f) - lastAtt );
      }
   }
   
   /* Did not find a match in any of the leaf nodes */
   aTIesrDT->treeOffset += sizeBranchSet;
   return TIesrDTErrorNone;
}

/* SearchBranchSetOffset

 Move the offset to the correct offset of sublevel branches or the same
 level leafset.
 1) if the requested BranchIndex is given, then
 move the offset to the sublevel branch, also obtain the sublevel question
 2) else move the offset to the same level leaf set which contains
 pronunciation .

 @param aTIesrDT pointer to the decision tree
 @param aContext context of the letter to be pronunced
 @param aMatchContext the question of this level
 @param aBranchIndex the sub-level branch to be located
 @param aLeafContext question of the sublevel branch
 */
static TIesrDT_Error_t  SearchBranchSetOffset( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short aMatchContext,
short aBranchIndex,
unsigned char * aLeafContext)
{
   unsigned char *tree;
   int offset;
   short numBranches;
   short i;
   short lastPos;
   int offsetToTheBranch = 0;
   short branchSiblingOffset;
   TIesrDT_Error_t dtError;
   
   /* Search the leaf node set pointed to by the global tree offset
    * for a match */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   /* get the number of branches */
   numBranches = (short) tree[offset++];
   lastPos = offset;
   
   if (aBranchIndex >= 0)
   {
      /* the requested branch index is given */
      
      /* accumulate offset */
      for (i=0;i<aBranchIndex;i++)
      {
         /* Determine the branch context and attribute to match. */
         /* Move past branch to its leaf node set */
         dtError = GetBranchInfo( tree, offset,
         (unsigned short *)&branchSiblingOffset,
         aLeafContext,
         (unsigned int*)&offset);
         if (dtError != TIesrDTErrorNone)
         {
            return dtError;
         }
         
         offsetToTheBranch += branchSiblingOffset;
         
      }
      
      dtError = GetBranchInfo( tree, offset,
      (unsigned short *)&branchSiblingOffset,
      aLeafContext,
      (unsigned int *)&offset);
      if (dtError != TIesrDTErrorNone)
      {
         return dtError;
      }
   }
   
   /* offset to position after the end of the BranchOffset data */
   offset = lastPos + numBranches*sizeof(short);
   
   /* need to check if the following read data belongs to the same level */
   if (numBranches < aTIesrDT->stAttInfo.numAttValPerAttType[aMatchContext]
   && aBranchIndex >= 0 )
   {
      /* still in the same level */
      /* the following data should be leaf set */
      
      if (!AtLeaf(tree, offset))
      {
         return dtError;
      }
      
      /* offset to the sublevel branches */
      offset += (int) tree[offset];
      
   }
   
   /* return the offset in the tree of the first child */
   aTIesrDT->treeOffset = offsetToTheBranch + offset;
   
   return TIesrDTErrorNone;
}
#endif

/*----------------------------------------------------------------
 FindTopBranchContext

 This function searches through the decision tree to find the first
 branch node.  Its context defines the top branch context.  If the
 decision tree has no branch nodes, then it is assumed that the
 decision tree consists of a single leaf node that defines the output
 phone index for all possible contexts.

 ----------------------------------------------------------------*/
#ifdef DEV_OFFSET
static TIesrDT_Error_t FindTopBranchContext( TIesrDT_t aTIesrDT )
{
   TIesrDT_Error_t dtError;
   unsigned char* tree;
   int treeOffset;
   unsigned short branchOffset;
   unsigned char branchLeafContext;
   
   
   /* Start at top of the tree */
   tree = aTIesrDT->tree;
   treeOffset = 0;
   aTIesrDT->treeOffset = treeOffset;
   
   /* Offset is at the first branch node.  Get its info */
   dtError = GetBranchInfo( tree, treeOffset,
   &branchOffset,
   &branchLeafContext ,
   (unsigned int *)&aTIesrDT->treeOffset);
   
   if (dtError == TIesrDTErrorFail)
   {
      aTIesrDT->topBranchContext = CXT_NONE;
      return TIesrDTErrorNone;
   }
   
   aTIesrDT->topBranchContext = branchLeafContext;
   
   return TIesrDTErrorNone;
}
#else
static TIesrDT_Error_t FindTopBranchContext( TIesrDT_t aTIesrDT )
{
   TIesrDT_Error_t dtError;
   unsigned char* tree;
   int treeOffset;
   
   unsigned char branchPhone;
   unsigned char branchAttribute;
   unsigned char branchLeafContext;
   
   
   /* Start at top of the tree */
   tree = aTIesrDT->tree;
   treeOffset = 0;
   aTIesrDT->treeOffset = treeOffset;
   
   /* Offset is at the first branch node.  Get its info */
   dtError = GetBranchInfo( tree, treeOffset,
   &branchPhone,
   &branchAttribute,
   &branchLeafContext ,
   &aTIesrDT->treeOffset);
   if (dtError == TIesrDTErrorFail)
   {
      aTIesrDT->topBranchContext = CXT_NONE;
      return TIesrDTErrorNone;
   }
   
   aTIesrDT->topBranchContext = branchLeafContext;
   aTIesrDT->cxtfrPhone = branchPhone;
   
   return TIesrDTErrorNone;
}


/* If tree starts with top branch context leaf set, go past them to
 get to the top branch node. */
if( AtLeaf(tree, treeOffset) )
{
   SkipPastLeafSet( tree, &treeOffset, NULL );
   
   /* All nodes in tree are a single leaf set, no branch nodes */
   if( treeOffset >= aTIesrDT->treeSize )
   {
      aTIesrDT->topBranchContext = CXT_NONE;
      return TIesrDTErrorNone;
   }
}


/* Offset is at the first branch node.  Get its info */
dtError = SFB_GetBranchInfo( tree, treeOffset,
&branchContext, d
&branchAttribute,
&branchLeafContext );


aTIesrDT->topBranchContext = branchContext;
return TIesrDTErrorNone;
}

/*----------------------------------------------------------------
 SkipPastLeafSet

 The offset pointer argument is pointing to the start of a leaf set
 of a tree. Move the offset pointer past the leaf set.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t SFB_SkipPastLeafSet( const unsigned char* aTree,
int *aOffset)
{
   int offset;
   
   short numLeafNodes;
   unsigned char phoneIndex;
   short nodeCount;
   short numNodeWithPhone;
   short attCount;
   short nodeAtt;
   short lastAtt;
   
   offset = *aOffset;
   
   /* Number of leaf nodes in set */
   numLeafNodes = (short)aTree[offset++];
   
   /* Skip past leaf nodes corresponding to each phone index in the set */
   nodeCount = 0;
   while( nodeCount < numLeafNodes )
   {
      /* Phone index for a set of attribute values */
      phoneIndex = aTree[offset++];
      
      /* Number of leaf nodes with attribute values for this phone */
      numNodeWithPhone = aTree[offset++];
      
      attCount = 0;
      while( attCount < numNodeWithPhone )
      {
         /* Attribute value, or if negative, a negative value
          indicating the end of a sequence of attribute values
          corresponding to the present phone index. */
         nodeAtt = (char)aTree[offset++];
         
         /* Individual attribute value or start of sequence. */
         if( nodeAtt >= 0 )
         {
            attCount++;
            lastAtt = nodeAtt;
         }
         
         /* nodeAtt indicates end of a sequence of attribute values */
         else
         {
            attCount += ( -nodeAtt  - lastAtt );
         }
      }
      
      nodeCount += numNodeWithPhone;
   }
   
   *aOffset = offset;
   
   return TIesrDTErrorNone;
}

static TIesrDT_Error_t SkipPastLeafSet( const unsigned char* aTree,
int *aOffset ,
short *nVisitedLeaves)
{
   int offset;
   
   short numLeafNodes;
   unsigned char phoneIndex;
   short nodeCount;
   short numNodeWithPhone;
   short attCount;
   short nodeAtt;
   short lastAtt;
   
   offset = *aOffset;
   
   /* Number of leaf nodes in set */
   numLeafNodes = (short)aTree[offset++];
   
   if (nVisitedLeaves) (*nVisitedLeaves) = numLeafNodes;
   
   /* Skip past leaf nodes corresponding to each phone index in the set */
   nodeCount = 0;
   while( nodeCount < numLeafNodes )
   {
      /* Phone index for a set of attribute values */
      phoneIndex = aTree[offset++];
      
      /* Number of leaf nodes with attribute values for this phone */
      numNodeWithPhone = aTree[offset++];
      
      attCount = 0;
      offset += numNodeWithPhone;
      
      nodeCount += numNodeWithPhone;
   }
   
   *aOffset = offset;
   
   return TIesrDTErrorNone;
}


/*----------------------------------------------------------------
 SearchTopBranch

 This function examines a top branch node to determine if the branch
 node context and attribute match the word context.  If so, this
 function continues down the branch to find the branch and leaf node
 of the branch that correspond to the word context.  The phone index
 of the matching leaf node is output.

 If this top branch node context and attribute do not match the word
 context, then the leaf nodes of the branch are skipped, and this
 function does not return a valid phoneIndex.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t SFB_SearchTopBranch( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short* aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   
   short phoneIndex;
   short noMatch;
   unsigned char  branchContext;
   unsigned char  branchAttribute;
   unsigned char  branchLeafContext;
   unsigned char  curContext;
   
   
   /* Maintain a copy of the current branch context */
   dtError = SFB_GetBranchInfo( aTIesrDT->tree, aTIesrDT->treeOffset,
   &branchContext, &branchAttribute,
   &branchLeafContext );
   
   /* Determine if the top branch matches */
   dtError = SFB_SearchBranch( aTIesrDT, aContext, &noMatch, &phoneIndex );
   
   /* If the top branch itself does not match, then return immediately */
   if( noMatch )
   {
      *aPhoneIndex = -1;
      return dtError;
   }
   
   /* If the top branch matches, and a leaf node of the top branch
    leaf node set matches the top branch leaf context,
    then the phone index has been found */
   if( phoneIndex >= 0 )
   {
      *aPhoneIndex = phoneIndex;
      return dtError;
   }
   
   /* The top branch context matches, but no leaf node matches the top branch
    node leaf context. Follow the branch nodes down the tree to find the
    branch and leaf node that match.  We are guaranteed that one of the
    branches will contain a leaf node that matches. */
   
   /* The current branch context to be searched */
   curContext = branchLeafContext;
   
   while( aTIesrDT->treeOffset < aTIesrDT->treeSize )
   {
      /* Find the next branch that has branch context that matches current
       desired branch context. */
      dtError = FindNextBranch( aTIesrDT, curContext );
      if( dtError != TIesrDTErrorNone )
      {
         return dtError;
      }
      
      /* Obtain current branch context */
      dtError = SFB_GetBranchInfo( aTIesrDT->tree, aTIesrDT->treeOffset,
      &branchContext, &branchAttribute,
      &branchLeafContext );
      if( dtError != TIesrDTErrorNone )
         break;
      
      /* Determine if the current branch has a leaf node set that matches
       the context, and therefore the phone has been found. */
      dtError = SFB_SearchBranch( aTIesrDT, aContext,
      &noMatch, &phoneIndex );
      if( dtError != TIesrDTErrorNone )
         break;
      
      
      /* The present branch matches branch context and contains a leaf
       that matches the branch leaf context, so the phone has been
       found. */
      if( phoneIndex >= 0 )
      {
         *aPhoneIndex = phoneIndex;
         return TIesrDTErrorNone;
      }
      
      /* If the present branch context did match, but no leaf matched
       the present branch leaf context, change the branch context
       for further searching to the present branch leaf context.
       Otherwise, further searching will use the next branch having
       the same current branch context. */
      if( ! noMatch )
      {
         curContext = branchLeafContext;
      }
   }
   
   
   /* Something is wrong with the tree, and a matching leaf node was
    not found when it should have been found. */
   *aPhoneIndex = -1;
   return TIesrDTErrorFail;
}

static TIesrDT_Error_t SearchTopBranch( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short* aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   short iChecked = 0;
   short phoneIndex;
   short noMatch;
   unsigned char  branchPhone;
   unsigned char  branchAttribute;
   unsigned char  branchLeafContext;
   unsigned char  curContext;
   
   
   /* Maintain a copy of the current branch context */
   dtError = GetBranchInfo( aTIesrDT->tree, aTIesrDT->treeOffset,
   &branchPhone, &branchAttribute,
   &branchLeafContext , NULL);
   
   curContext = aTIesrDT->topBranchContext;
   /* Determine if the top branch matches */
   dtError = SearchBranch( aTIesrDT, curContext, aContext, &noMatch, &phoneIndex ,
   &iChecked);
   
   /* If the top branch itself does not match, then return immediately */
   if( noMatch )
   {
      *aPhoneIndex = -1;
      return dtError;
   }
   
   /* If the top branch matches, and a leaf node of the top branch
    leaf node set matches the top branch leaf context,
    then the phone index has been found */
   if( phoneIndex >= 0 )
   {
      *aPhoneIndex = phoneIndex;
      return dtError;
   }
   
   /* since the iChecked is not numBranch, the remainings are
    branchs that need to be checked */
   
   /* The top branch context matches, but no leaf node matches the top branch
    node leaf context. Follow the branch nodes down the tree to find the
    branch and leaf node that match.  We are guaranteed that one of the
    branches will contain a leaf node that matches. */
   
   /* The current branch context to be searched */
   curContext = branchLeafContext;
   
   while( aTIesrDT->treeOffset < aTIesrDT->treeSize &&
   iChecked < aTIesrDT->stAttInfo.numAttValPerAttType[curContext] )
   {
      dtError = GetBranchInfo( aTIesrDT->tree, aTIesrDT->treeOffset,
      &branchPhone, &branchAttribute,
      &branchLeafContext , NULL);
      if (branchAttribute != aContext[curContext])
      {
         dtError = SkipThisBranch(aTIesrDT);
         if( dtError != TIesrDTErrorNone )
         {
            return dtError;
         }
      }else break;
      iChecked ++;
      
   }
   
   /* Determine if the current branch has a leaf node set that matches
    the context, and therefore the phone has been found. */
   dtError = SearchThisBranch( aTIesrDT, curContext, aContext,
   &noMatch, &phoneIndex );
   if( dtError == TIesrDTErrorNone && phoneIndex >= 0 )
   {
      *aPhoneIndex = phoneIndex;
      return TIesrDTErrorNone;
   }
   
   /* Something is wrong with the tree, and a matching leaf node was
    not found when it should have been found. */
   *aPhoneIndex = -1;
   return TIesrDTErrorFail;
}


/*----------------------------------------------------------------
 SearchBranch

 This function searches a branch to see if it matches the word
 context.  There are two conditions that must hold true in order to
 match the word context.

 First, the branch context and attribute must match the word context
 attribute.  If this does not match, then this function sets the
 aNoMatch flag, skips the leaf node set corresponding to the branch
 node, and returns.

 Second, the leaf node set associated with the branch must contain an
 attribute that matches the branch node leaf context specified in the
 branch node.  If a match is found, then the aPhoneIndex will contain
 a valid phone index.  Otherwise it will contain a negative number.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t SFB_SearchBranch( TIesrDT_t aTIesrDT,
const unsigned char* aContext,
short *aNoMatch, short *aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   unsigned char *tree;
   int offset;
   
   unsigned char branchContext;
   unsigned char branchAttribute;
   unsigned char branchLeafContext;
   
   unsigned char matchAttribute;
   short phoneIndex;
   
   
   /* Process the tree branch located at global offset */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   
   /* Determine the branch context and attribute to match. */
   /* Move past branch to its leaf node set */
   dtError = SFB_GetBranchInfo( tree, offset,
   &branchContext,
   &branchAttribute,
   &branchLeafContext );
   
   /* Move past branch node.  This should not be the end of the tree. */
   offset += 2;
   if( offset >= aTIesrDT->treeSize )
      return TIesrDTErrorFail;
   
   
   /* Check if the branch context and attribute does not match the
    word context attribute. If it does not match, skip this
    branch node and its leaf nodes. */
   matchAttribute = aContext[ branchContext ];
   if( matchAttribute != branchAttribute )
   {
      /* The branch may not be follwed by a leaf node set */
      if( AtLeaf( tree, offset ) )
      {
         dtError = SkipPastLeafSet( tree, &offset , NULL);
      }
      aTIesrDT->treeOffset = offset;
      *aPhoneIndex = -1;
      *aNoMatch = TRUE;
      return dtError;
   }
   else
   {
      /* The branch context does match */
      *aNoMatch = FALSE;
      aTIesrDT->treeOffset = offset;
   }
   
   
   /* Check if this branch node is followed immediately by another
    branch node.  In this case, there is no leaf context to match,
    so there can not be a phone index returned.  This acts as if
    there were no match of any leaf to the branch node leaf
    context. */
   if( ! AtLeaf( tree, offset ) )
   {
      *aPhoneIndex = -1;
      return TIesrDTErrorNone;
   }
   
   
   /* Search the leaf node set corresponding to this branch node */
   dtError = SearchLeafSet( aTIesrDT, aContext,
   branchLeafContext,
   &phoneIndex , NULL);
   
   
   *aPhoneIndex = phoneIndex;
   return dtError;
}

static TIesrDT_Error_t SearchBranch( TIesrDT_t aTIesrDT, unsigned char branchContext,
const unsigned char* aContext,
short *aNoMatch, short *aPhoneIndex ,
short * iNumChecked)
{
   TIesrDT_Error_t dtError;
   unsigned char *tree;
   int offset;
   short iNumLeaves = 0;
   unsigned char branchPhone;
   unsigned char branchAttribute;
   unsigned char branchLeafContext;
   
   unsigned char matchAttribute;
   short phoneIndex;
   
   /* Process the tree branch located at global offset */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   
   /* Determine the branch context and attribute to match. */
   /* Move past branch to its leaf node set */
   dtError = GetBranchInfo( tree, offset,
   &branchPhone,
   &branchAttribute,
   &branchLeafContext ,
   &offset);
   
   /* Move past branch node.  This should not be the end of the tree. */
   if( offset >= aTIesrDT->treeSize )
      return TIesrDTErrorFail;
   
   
   /* Check if the branch context and attribute does not match the
    word context attribute. If it does not match, skip this
    branch node and its leaf nodes. */
   matchAttribute = aContext[ branchContext ];
   if( matchAttribute != branchAttribute )
   {
      /* The branch may not be follwed by a leaf node set */
      if( AtLeaf( tree, offset ) )
      {
         dtError = SkipPastLeafSet( tree, &offset , NULL);
      }
      aTIesrDT->treeOffset = offset;
      *aPhoneIndex = -1;
      *aNoMatch = TRUE;
      return dtError;
   }
   else
   {
      /* The branch context does match */
      *aNoMatch = FALSE;
      aTIesrDT->treeOffset = offset;
   }
   
   
   /* Check if this branch node is followed immediately by another
    branch node.  In this case, there is no leaf context to match,
    so there can not be a phone index returned.  This acts as if
    there were no match of any leaf to the branch node leaf
    context. */
   if( ! AtLeaf( tree, offset ) )
   {
      *aPhoneIndex = -1;
      return TIesrDTErrorNone;
   }
   
   
   /* Search the leaf node set corresponding to this branch node */
   dtError = SearchLeafSet( aTIesrDT, aContext,
   branchLeafContext,
   &phoneIndex , &iNumLeaves);
   
   if (iNumChecked) (*iNumChecked) += iNumLeaves;
   
   *aPhoneIndex = phoneIndex;
   return dtError;
}
#endif

#ifdef DEV_OFFSET
/* SearchThisBranch

 This recursive function uses offset information to achieve quick decision
 tree lookup. Three types of nodes are read.
 1) branchset : find index of the branch that answers upper-level question
 2) quesetion : the read question is used for the following leaf set
 3) leaf set  : pronunciation has to be in the leaf set

 @param aTIesrDT pointer to decision tree
 @param aBranchContext the context (question) to be answered
 @param aContext the context information, together with the aBranchContext to
 obtain the question to be answered
 @param aNoMatch pointer to the returned information that if pronunciation has
 been found
 @param aPhoneIndex pointer to the found phone index

 */
static TIesrDT_Error_t SearchThisBranch( TIesrDT_t aTIesrDT,
unsigned char aBranchContext,
const unsigned char* aContext,
short *aNoMatch, short *aPhoneIndex )
{
   TIesrDT_Error_t dtError = TIesrDTErrorNone;
   unsigned char *tree;
   int offset;
   unsigned char branchLeafContext;
   short phoneIndex;
   short branchIndex;
   
   /* Process the tree branch located at global offset */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   /* three types of nodes in the decision tree.
    */
   switch(TypeOfNode(tree, offset))
   {
      case BRANCHSET:
         /* branch set
         
          branches are compressed into the branchset using run-length coding,
          the same as that done in compressing leaves to leaf set. */
         dtError = SearchBranchSet(aTIesrDT, aContext,
         aBranchContext,
         &branchIndex );
         if (dtError != TIesrDTErrorNone)
         {
            
            *aPhoneIndex = -1;
            *aNoMatch = TRUE;
            return TIesrDTErrorFail;
         }
         
         /* find the location of the requested sub-level branch or move to
          the leafSet at this Level*/
         dtError = SearchBranchSetOffset( aTIesrDT,
         aContext,
         aBranchContext,
         branchIndex,
         &branchLeafContext);
         
         if (dtError != TIesrDTErrorNone)
         {
            
            *aPhoneIndex = -1;
            *aNoMatch = TRUE;
            return TIesrDTErrorFail;
         }
         
         if (branchIndex >= 0)
         {
            /* go the sublevel */
            dtError = SearchThisBranch( aTIesrDT,
            branchLeafContext,
            aContext,
            aNoMatch, &phoneIndex );
            if (phoneIndex >= 0 && dtError == TIesrDTErrorNone)
            {
               *aPhoneIndex = phoneIndex;
               *aNoMatch = FALSE;
               return dtError;
            }
         }
         break;
      case QUESTION:
         
         /* read the question of the leaf set */
         dtError = GetQuestion( aTIesrDT->tree,
         aTIesrDT->treeOffset,
         &aBranchContext,
         (unsigned int *)&aTIesrDT->treeOffset);
         if (dtError != TIesrDTErrorNone)
         {
            return dtError;
         }
         
         break;
      default:
         break;
   }
   
   /* Search the leaf node set */
   dtError = SearchLeafSet( aTIesrDT, aContext,
   aBranchContext,
   &phoneIndex);
   if (phoneIndex >= 0 && dtError == TIesrDTErrorNone)
   {
      *aPhoneIndex = phoneIndex;
      *aNoMatch = FALSE;
      return dtError;
   }
   
   if (*aNoMatch == FALSE && *aPhoneIndex >= 0 && dtError == TIesrDTErrorNone)
      return TIesrDTErrorNone;
   
   return dtError;
}
#else
static TIesrDT_Error_t SearchThisBranch( TIesrDT_t aTIesrDT, unsigned char branchContext,
const unsigned char* aContext,
short *aNoMatch, short *aPhoneIndex )
{
   TIesrDT_Error_t dtError = TIesrDTErrorNone;
   unsigned char *tree;
   int offset;
   short iNumLeaves = 0;
   unsigned char branchPhone;
   unsigned char branchAttribute;
   unsigned char branchLeafContext;
   
   unsigned char matchAttribute;
   short phoneIndex;
   
   /* Process the tree branch located at global offset */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   
   /* Determine the branch context and attribute to match. */
   /* Move past branch to its leaf node set */
   dtError = GetBranchInfo( tree, offset,
   &branchPhone,
   &branchAttribute,
   &branchLeafContext ,
   &offset);
   
   /* Move past branch node.  This should not be the end of the tree. */
   if( offset >= aTIesrDT->treeSize )
      return TIesrDTErrorFail;
   
   
   /* Check if the branch context and attribute does not match the
    word context attribute. If it does not match, skip this
    branch node and its leaf nodes. */
   matchAttribute = aContext[ branchContext ];
   if( matchAttribute != branchAttribute )
   {
      iNumLeaves = 0;
      /* The branch may not be follwed by a leaf node set */
      if( AtLeaf( tree, offset ) )
         dtError = SkipPastLeafSet( tree, &offset , &iNumLeaves);
      aTIesrDT->treeOffset = offset;
      
      while(iNumLeaves<aTIesrDT->stAttInfo.numAttValPerAttType[branchLeafContext])
      {
         SkipThisBranch(aTIesrDT);
         iNumLeaves++;
      }
   }
   else
   {
      /* The branch context does match */
      *aNoMatch = FALSE;
      aTIesrDT->treeOffset = offset;
      
      if( ! AtLeaf( tree, offset ) )
      {
         dtError =
         SearchThisBranch(aTIesrDT, branchLeafContext, aContext, aNoMatch, aPhoneIndex);
         
         if (*aNoMatch == FALSE && *aPhoneIndex >= 0 && dtError == TIesrDTErrorNone)
            return TIesrDTErrorNone;
      }
      else
      {
         /* Search the leaf node set corresponding to this branch node */
         dtError = SearchLeafSet( aTIesrDT, aContext,
         branchLeafContext,
         &phoneIndex , &iNumLeaves);
         if (phoneIndex >= 0 && dtError == TIesrDTErrorNone)
         {
            *aPhoneIndex = phoneIndex;
            *aNoMatch = FALSE;
            return dtError;
         }
         
         while(iNumLeaves < aTIesrDT->stAttInfo.numAttValPerAttType[branchLeafContext])
         {
            dtError =
            SearchThisBranch(aTIesrDT, branchLeafContext, aContext, aNoMatch, aPhoneIndex);
            
            if (*aNoMatch == FALSE && *aPhoneIndex >= 0 && dtError == TIesrDTErrorNone)
               return TIesrDTErrorNone;
            iNumLeaves++;
         }
      }
   }
   return dtError;
}

static TIesrDT_Error_t SkipThisBranch( TIesrDT_t aTIesrDT)
{
   TIesrDT_Error_t dtError;
   unsigned char *tree;
   int offset;
   short iNumLeaves = 0, i;
   unsigned char branchPhone;
   unsigned char branchAttribute;
   unsigned char branchLeafContext;
   unsigned char matchAttribute;
   short phoneIndex;
   
   /* Process the tree branch located at global offset */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   
   /* Determine the branch context and attribute to match. */
   /* Move past branch to its leaf node set */
   dtError = GetBranchInfo( tree, offset,
   &branchPhone,
   &branchAttribute,
   &branchLeafContext ,
   &offset);
   
   /* Move past branch node.  This should not be the end of the tree. */
   if( offset >= aTIesrDT->treeSize )
      return TIesrDTErrorFail;
   
   
   /* The branch may not be follwed by a leaf node set */
   if( AtLeaf( tree, offset ) )
   {
      dtError = SkipPastLeafSet( tree, &offset , &iNumLeaves);
   }
   aTIesrDT->treeOffset = offset;
   
   i=iNumLeaves;
   while(i<aTIesrDT->stAttInfo.numAttValPerAttType[branchLeafContext])
   {
      SkipThisBranch(aTIesrDT);
      i++;
   }
   return TIesrDTErrorNone;
}
#endif

/*----------------------------------------------------------------
 GetDTPhoneIndex

 This function traverses a decision tree to determine the phone index
 that should be output for the given character of a word in its context.

 ----------------------------------------------------------------*/
#ifdef DEV_OFFSET
// read *.olmdtpm
static TIesrDT_Error_t  GetDTPhoneIndex( TIesrDT_t aTIesrDT, char aChar,
const unsigned char* aContext,
unsigned char* aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   short phoneIndex;
   short noMatch;
   
   /* Load the tree file corresponding to the current character. */
   LoadTreeFile( aTIesrDT, aChar );
   
   
   /* Start processing at beginning of tree */
   aTIesrDT->treeOffset = 0;
   
   
   /* Get the top branch context.  This is saved in the TIesrDT object. */
   dtError = FindTopBranchContext( aTIesrDT );
   if( dtError != TIesrDTErrorNone )
      goto SearchFail;
   
   /* Search the tree file to find the phone index to output */
   if ( aTIesrDT->treeOffset < aTIesrDT->treeSize )
      SearchThisBranch( aTIesrDT, aTIesrDT->topBranchContext,
      aContext,  &noMatch, &phoneIndex );
   
   /* If the top branch itself does not match, then return immediately */
   if( noMatch || phoneIndex < 0 )
      goto SearchFail;
   
   /* If the top branch matches, and a leaf node of the top branch
    leaf node set matches the top branch leaf context,
    then the phone index has been found */
   *aPhoneIndex = (unsigned char)phoneIndex;
  
   if( ! aTIesrDT->preLoaded )
   {
      free( aTIesrDT->tree );
      aTIesrDT->tree = NULL;
   }
  
   return TIesrDTErrorNone;
   
   /* For some reason the tree failed to provide a phone index for
    this character and context. This should never happen unless the
    tree is corrupted. */
   
   SearchFail:
      if( ! aTIesrDT->preLoaded )
      {
         free( aTIesrDT->tree );
         aTIesrDT->tree = NULL;
      }
      
      *aPhoneIndex = -1;
      
      return TIesrDTErrorFail;
}
#else
// read *.clmdtpm
static TIesrDT_Error_t  SFB_GetDTPhoneIndex( TIesrDT_t aTIesrDT, char aChar,
const unsigned char* aContext,
unsigned char* aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   short phoneIndex;
   
   /* Load the tree file corresponding to the current character. */
   LoadTreeFile( aTIesrDT, aChar );
   
   
   /* Start processing at beginning of tree */
   aTIesrDT->treeOffset = 0;
   
   
   /* Get the top branch context.  This is saved in the TIesrDT object. */
   dtError = FindTopBranchContext( aTIesrDT );
   if( dtError != TIesrDTErrorNone )
      goto SearchFail;
   
   
   /* Search the tree file to find the phone index to output */
   while( aTIesrDT->treeOffset < aTIesrDT->treeSize )
   {
      /* Current offset is at a leaf node set corresponding to the top
       branch context. The information in these leaf nodes is
       compressed, and a search must be done through the set to see
       if any of the leaf node attributes match the top branch
       context word attribute.  If so, the matching leaf node
       contains the phone index to output.  */
      if( AtLeaf( aTIesrDT->tree, aTIesrDT->treeOffset ) )
      {
         
         dtError = SearchTopLeafSet( aTIesrDT, aContext, &phoneIndex );
         if( dtError != TIesrDTErrorNone )
            goto SearchFail;
         
         /* If no matching phone index for the top branch context was
          found, continue searching */
         if( phoneIndex < 0 )
         {
            continue;
         }
         else
         {
            /* Phone index to output has been found in the top branch
             context leaf node set. */
            *aPhoneIndex = (unsigned char)phoneIndex;
            free( aTIesrDT->tree );
            aTIesrDT->tree = NULL;
            return TIesrDTErrorNone;
         }
      }
      
      
      /* Current offset is at a top context branch node.  Test to see if the
       branch node attribute and context match the word context.  If
       so, then search this branch to find the output phone. */
      else
      {
         dtError = SearchTopBranch( aTIesrDT, aContext, &phoneIndex );
         if( dtError != TIesrDTErrorNone )
            goto SearchFail;
         
         /* This top context branch did not match the word context.
          Go to the next top context node, which may be a leaf node
          set or another top context branch node. */
         if( phoneIndex < 0 )
         {
            dtError = FindNextTopNode( aTIesrDT );
            if( dtError != TIesrDTErrorNone )
               goto SearchFail;
            
            /* Process next top node, which is either leaf or branch. */
            continue;
         }
         else
         {
            /* Phone index to output has been found by following this
             top branch. */
            *aPhoneIndex = (unsigned char)phoneIndex;
            free( aTIesrDT->tree );
            aTIesrDT->tree = NULL;
            return TIesrDTErrorNone;
         }
      }
   }
   /* For some reason the tree failed to provide a phone index for
    this character and context. This should never happen unless the
    tree is corrupted. */
   
   SearchFail:
      free( aTIesrDT->tree );
      aTIesrDT->tree = NULL;
      
      *aPhoneIndex = -1;
      
      return TIesrDTErrorFail;
}

static TIesrDT_Error_t  GetDTPhoneIndex( TIesrDT_t aTIesrDT, char aChar,
const unsigned char* aContext,
unsigned char* aPhoneIndex )
{
   TIesrDT_Error_t dtError;
   short phoneIndex;
   short i;
   short bFound= FALSE;
   unsigned char matchAttribute;
   short noMatch;
   short nRemainingNodes = aTIesrDT->root.numBranch, iTmp = 0;
   
   /* Load the tree file corresponding to the current character. */
   LoadTreeFile( aTIesrDT, aChar );
   
   
   /* Start processing at beginning of tree */
   aTIesrDT->treeOffset = 0;
   
   
   /* Get the top branch context.  This is saved in the TIesrDT object. */
   dtError = FindTopBranchContext( aTIesrDT );
   if( dtError != TIesrDTErrorNone )
      goto SearchFail;
   
   if( aTIesrDT->topBranchContext != CXT_NONE &&
   aTIesrDT->root.numBranch > 1 /* there are questions to be asked at the root*/ )
   {
      matchAttribute = aContext[aTIesrDT->topBranchContext];
   }else
   {
      /* no question on the context,
       so just return the context free phone */
      *aPhoneIndex = (unsigned char)aTIesrDT->cxtfrPhone;
      free( aTIesrDT->tree );
      aTIesrDT->tree = NULL;
      return TIesrDTErrorNone;
   }
   
   for (i=0;i<(short)aTIesrDT->root.numBranch;i++)
   {
      if (aTIesrDT->root.AttVal[i] == matchAttribute)
      {
         bFound= TRUE;
         break;
      }
   }
   
   if (bFound == FALSE)
      goto SearchFail;
   
   aTIesrDT->treeOffset = aTIesrDT->root.BranchOffset[i];
   
   /* Search the tree file to find the phone index to output */
   while( aTIesrDT->treeOffset < aTIesrDT->treeSize )
   {
      /* Current offset is at a leaf node set corresponding to the top
       branch context. The information in these leaf nodes is
       compressed, and a search must be done through the set to see
       if any of the leaf node attributes match the top branch
       context word attribute.  If so, the matching leaf node
       contains the phone index to output.  */
      if( AtLeaf( aTIesrDT->tree, aTIesrDT->treeOffset ) )
      {
         
         dtError =  SearchLeafSet( aTIesrDT, aContext,
         aTIesrDT->topBranchContext,
         &phoneIndex, &iTmp);
         if( dtError != TIesrDTErrorNone )
            goto SearchFail;
         
         nRemainingNodes = aTIesrDT->root.numBranch - iTmp;
         
         /* If no matching phone index for the top branch context was
          found, continue searching */
         if( phoneIndex < 0 )
         {
            continue;
         }
         else
         {
            /* Phone index to output has been found in the top branch
             context leaf node set. */
            *aPhoneIndex = (unsigned char)phoneIndex;
            free( aTIesrDT->tree );
            aTIesrDT->tree = NULL;
            return TIesrDTErrorNone;
         }
      }
      
      
      /* Current offset is at a top context branch node.  Test to see if the
       branch node attribute and context match the word context.  If
       so, then search this branch to find the output phone. */
      else
      {
         dtError = SearchTopBranch( aTIesrDT, aContext, &phoneIndex );
         if( dtError != TIesrDTErrorNone )
            goto SearchFail;
         
         /* This top context branch did not match the word context.
          Go to the next top context node, which may be a leaf node
          set or another top context branch node. */
         if( phoneIndex < 0 )
         {
            printf("should not come to here\n");
            dtError = FindNextTopNode( aTIesrDT );
            if( dtError != TIesrDTErrorNone )
               goto SearchFail;
            
            /* Process next top node, which is either leaf or branch. */
            continue;
         }
         else
         {
            /* Phone index to output has been found by following this
             top branch. */
            *aPhoneIndex = (unsigned char)phoneIndex;
            free( aTIesrDT->tree );
            aTIesrDT->tree = NULL;
            return TIesrDTErrorNone;
         }
      }
   }
   //   SearchBranch( aTIesrDT,  aTIesrDT->topBranchContext,
   //	 aContext, &noMatch, &phoneIndex );
   
   /* If the top branch itself does not match, then return immediately */
   if( noMatch || phoneIndex < 0 )
      goto SearchFail;
   
   /* If the top branch matches, and a leaf node of the top branch
    leaf node set matches the top branch leaf context,
    then the phone index has been found */
   *aPhoneIndex = (unsigned char)phoneIndex;
   free( aTIesrDT->tree );
   aTIesrDT->tree = NULL;
   
   return TIesrDTErrorNone;
   
   /* For some reason the tree failed to provide a phone index for
    this character and context. This should never happen unless the
    tree is corrupted. */
   
   SearchFail:
      free( aTIesrDT->tree );
      aTIesrDT->tree = NULL;
      
      *aPhoneIndex = -1;
      
      return TIesrDTErrorFail;
}


/*----------------------------------------------------------------
 FindNextBranch

 This function searches through the tree to find the next branch that
 has the desired branch context.

 ----------------------------------------------------------------*/
static TIesrDT_Error_t  FindNextBranch( TIesrDT_t aTIesrDT, unsigned char aBranchContext )
{
   TIesrDT_Error_t dtError;
   
   unsigned char* tree;
   int offset;
   
   unsigned char branchContext;
   unsigned char branchAttribute;
   
   unsigned char branchLeafContext;
   
   /* Process the tree branch located at global offset */
   tree = aTIesrDT->tree;
   offset = aTIesrDT->treeOffset;
   
   
   while( offset < aTIesrDT->treeSize )
   {
      
      /* Go past a leaf node set */
      if( AtLeaf( tree, offset ) )
      {
         SkipPastLeafSet( tree, &offset , NULL);
      }
      
      
      /* At a branch node.  Check if the branch node context matches
       the desired branch node context.  */
      else
      {
         dtError = SFB_GetBranchInfo( tree, offset, &branchContext,
         &branchAttribute, &branchLeafContext );
         if( dtError != TIesrDTErrorNone )
            break;
         
         if( branchContext == aBranchContext )
         {
            aTIesrDT->treeOffset = offset;
            return TIesrDTErrorNone;
         }
         else
         {
            /* Go past the branch node */
            offset += 2;
         }
      }
   }
   
   /* The desired branch not found */
   return TIesrDTErrorFail;
}

#endif

/*----------------------------------------------------------------
 ToUpper

 Make a string uppercase.

 ----------------------------------------------------------------*/
static void ToUpper(char *string )
{
   char *chr = string;
   while (*chr != '\0')
   {
      *chr = toupper(*chr);
      chr++;
   }
}


/*----------------------------------------------------------------
 Deblank

 Find location past leading whitespace of a string, and make the string
 such that it is only the first non-blank portion of the original
 string.

 ----------------------------------------------------------------*/
static char*  Deblank( char *word )
{
   char* endword;
   
   /* Find first non-space in the word */
   while( *word != '\0' && isspace( (unsigned char)*word ) )
   {
      word++;
   }
   
   
   endword = word;
   while( *endword != '\0' && !isspace( (unsigned char)*endword ) )
   {
      endword++;
   }
   *endword = '\0';
   
   return word;
}
