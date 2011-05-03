/*=======================================================================
 TIesrDT_User.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header file exposes the implementation of the TIesrDT API,
 which is a Text-to-Pronunciation (TTP) tool.  This tool accepts as
 input the spelling of a word, and returns the phonetic
 pronunciation.

======================================================================*/


#ifndef _TIESRDT_USER_H
#define _TIESRDT_USER_H


/* This code enables generating a Windows DLL */
#if defined( WIN32 ) || defined( WINCE )

/* The following ifdef block is the standard way of creating macros which make exporting 
from a DLL simpler. All files within this DLL are compiled with the TIESRFA_EXPORTS
symbol defined on the command line. This symbol should not be defined on any project
that uses this DLL. This way any other project whose source files include this file see 
TIESRFA_API functions as being imported from a DLL, whereas this DLL sees symbols
defined with this macro as being exported. */
#ifdef TIESRDT_EXPORTS
#define TIESRDT_API __declspec(dllexport)
#else
#define TIESRDT_API __declspec(dllimport)
#endif

#else

#define TIESRDT_API

#endif 

#include <stddef.h>


/*--------------------------------
  Typedefs and defines 
  --------------------------------*/


/* Error return values for this API */
typedef enum 
{
   TIesrDTErrorNone,
   TIesrDTErrorFail,
   TIesrDTErrorMemory,
   TIesrDTErrorLoad,
   TIesrDTErrorFile
} TIesrDT_Error_t;


/* The instance structure pointer.  A pointer will be returned 
   by TIesrDT_Create. */
typedef struct TIesrDT_Struct* TIesrDT_t;



/*----------------------------------------------------------------
  Function interface
  ----------------------------------------------------------------*/



/*----------------------------------------------------------------
  TIesrDT_Create

  This function creates an instance of the TIesrDT machine.  It
  accepts as input the path to the directory containing the decision
  trees and the supporting files.  Once properly created, this
  instance may be used with the function TIesrDT_Pron multiple times
  to determine pronunciation of words using the decision trees.  This
  function allocates information on the heap, and so TIesrDT_Destroy
  must be called when done determining pronunciations in order to
  release all allocated resources.  Note that the first argument is
  the location that holds a pointer to the TIesrDT structure.  An
  example of usage is:

  TIesrDT_t aTIesrDT;
  TIesrDT_Error_t dtError;

  dtError = TIesrDT_Create( &aTIesrDT, aTreePath);

  ----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

    TIESRDT_API
    TIesrDT_Error_t TIesrDT_Create( TIesrDT_t* aTIesrDTPtr,
            const char* aTreePath );

#ifdef __cplusplus
}
#endif


/*----------------------------------------------------------------
 TIesrDT_CreatePreload

 This function creates an instance of the TIesrDT machine.  It
 accepts as input the path to the directory containing the decision
 trees and the supporting files.  This function will preload all
 decision trees by creating heap space and loading them into memory.  
 Thus the decision trees will not have to be reloaded for each letter 
 of each word during calculation of pronunciations. This requires
 more memory, but pronunciations are obtained much faster. Once properly
 created, this instance may be used with the function TIesrDT_Pron multiple
 times to determine pronunciation of words using the decision trees.  This
 function allocates information on the heap, and so TIesrDT_Destroy
 must be called when done determining pronunciations in order to
 release all allocated resources.  Note that the first argument is
 the location that holds a pointer to the TIesrDT structure.  An
 example of usage is:

  TIesrDT_t aTIesrDT;
  TIesrDT_Error_t dtError;

  dtError = TIesrDT_CreatePreload( &aTIesrDT, aTreePath);

  ----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif
    TIESRDT_API
    TIesrDT_Error_t TIesrDT_CreatePreload( TIesrDT_t* aTIesrDTPtr,
            const char* aTreePath );

#ifdef __cplusplus
}
#endif


/*--------------------------------
  TIesrDT_Pron

  This is the interface function of the TIesrDT API that determines a
  word pronunciation.  This function accepts as input a word in the
  character string aWord, and outputs a string, aPronString, which contains
  the sequential list of phones that describes the pronunciation of
  the word, each phone separated by a space.  The user must supply the
  output character string buffer with length aStringSize.

  --------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif
    TIESRDT_API
    TIesrDT_Error_t TIesrDT_Pron( TIesrDT_t aTIesrDT,
            const char *aWord,
            char *aPronString, size_t aStringSize );

#ifdef __cplusplus
}
#endif



/*----------------------------------------------------------------
  TIesrDT_Destroy

  This function destroys an instance of the TIesrDT machine created by
  TIesrDT_Create. This function releases all allocated resources.

  ----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif
    TIESRDT_API
    TIesrDT_Error_t TIesrDT_Destroy( TIesrDT_t aTIesrDT );

#ifdef __cplusplus
}
#endif

#endif /* _TIESRDT_USER_H */
