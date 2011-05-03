/*=======================================================================

 *
 * TestTIesrSI.h
 *
 * Header for implementation of program to test TIesrFlex API.
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
 
 Include file for building an application to do speech
 recognition using the TIesr SI API. This is a simple command line
 app that runs recognition of an utterance and outputs the words
 recognized.

 The macros, structures, and declarations in this header are
 for the convenience in building the application.

======================================================================*/

/*----------------------------------------------------------------

 ----------------------------------------------------------------*/

#ifndef _TIesrReco_H
#define _TIesrReco_H

// Windows headers
#if defined (WIN32)
#include <windows.h>
#endif

// The TIesr SI top level APIs for grammar network and model creation
// and speech recognition
#include "TIesrSI_User.h"

// For synchronization with multi-threaded speech recognition APIs
#if defined (LINUX)
#include <semaphore.h>
#elif defined (WIN32)
#endif

/*--------------------------------
  Overall constants
  --------------------------------*/

#ifndef TRUE
static const int TRUE = 1;
#endif

#ifndef FALSE
static const int FALSE = 0;
#endif

static const int MAX_LINE = 1000;
static const int MAX_FILE = 2000;


/*--------------------------------
  Definition of flags for Gaussian output types.
  The ones used must agree with the TIesrEngine build configuration!
  --------------------------------*/
static const int BYTEMEANS = 1;
static const int SHORTMEANS = 0;

static const int BYTEVARS = 1;
static const int SHORTVARS = 0;


/*--------------------------------
 Parameters used within the file recognition program to 
 setup the TIesrSI recognizer.
 --------------------------------*/
#if defined (LINUX)

/* Specify the sample rate for testing */

//static const int SAMPLE_RATE = 8000;
static const int SAMPLE_RATE = 24000;

static const int CIRCULAR_FRAMES = 500;
static const int AUDIO_FRAMES = 10;
static const int AUDIO_READ_RATE = 20000;
static const int JAC_RATE = 400;
static const int PRUNE_FACTOR = -10;
static const int TRANSI_WEIGHT = 2;
static const int SAD_END_FRAMES = 80;
static const int AUDIO_PRIORITY = 20;
#elif defined (WIN32)
static const int CIRCULAR_FRAMES = 500;
static const int AUDIO_FRAMES = 40;
static const int AUDIO_READ_RATE = 2;
static const int JAC_RATE = 400;
static const int PRUNE_FACTOR = -5;
static const int TRANSI_WEIGHT = 2;
static const int SAD_END_FRAMES = 80;
static const int AUDIO_PRIORITY = THREAD_PRIORITY_ABOVE_NORMAL;
#endif

/*--------------------------------
  The reco app information.  An enum defining the response errors,
  and a structure holding the reco information.
  --------------------------------*/
typedef enum RecoErrors
{
   RecoErrNone,
   RecoErrFail,
   RecoErrNoGrammar,
   RecoErrOpen,
   RecoErrClosed,
   RecoErrRecognizing,
   RecoErrIdle
} RecoError_t;

typedef enum RecoState
{
   IDLE,
   RECOGNIZING,
} RecoState_t;

typedef struct Reco {
    int memorySize;
    char* audioDevice;
    char* jacFile;
    char* sbcFile;
    char *grammarDir;
    char *captureFile;

    RecoState_t state;
    int tiesrIsOpen;

    TIesrSI_t tiesrSI;
    TIesrSI_Error_t speakResult;
    TIesrSI_Error_t doneResult;

#if defined ( LINUX )
    sem_t* start_semaphore;
    sem_t* done_semaphore;
#elif defined( WIN32 )
    HANDLE start_event;
    HANDLE done_event;
#endif

} Reco_t;      


/*----------------------------------------------------------------
  The functions that make up the reco utility
  ---------------------------------------------------------------*/

/* Callbacks for TIesrSI */
void SpeakCallback( void* aArg, TIesrSI_Error_t aError );
void DoneCallback( void* aArg, TIesrSI_Error_t aError );

/* The functions making up the app */
RecoError_t OpenTIesr( Reco_t* reco );
RecoError_t StartTIesr( Reco_t* reco );
void OutputResult( TIesrSI_t aTIesrSI );
RecoError_t StopTIesr( Reco_t* reco );
RecoError_t CloseTIesr( Reco_t* reco );


#endif	/* _TIesrReco_H */

