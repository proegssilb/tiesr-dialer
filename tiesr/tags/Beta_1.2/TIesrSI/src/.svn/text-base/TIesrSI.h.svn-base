/*=======================================================================
 TIesrSI.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 This file defines the functions and data structures that are internal to
 the TIesrSI API that the user does not need access to.

======================================================================*/

#ifndef _TIESRSI_H
#define _TIESRSI_H

/* Standard IO for various file outputs */
#include <stdio.h>

/* The TIesr Engine API for SI recognition with live audio */
#include <tiesr_engine_api_sireco.h>

/* The TIesr frame audio capture API */
#include <TIesrFA_User.h>

/* The TIesrSI User API */
#include "TIesrSI_User.h"

/* Multi-threading support APIs */
#if defined (LINUX)
#include <pthread.h>
#include <semaphore.h>
#elif defined (WIN32) || defined (WINCE)
#endif


/* Debug logging flag.  This results in output debug logging to files
   for each thread. This line should normally be commented out in order
   to build a version of TIESRSI that does not do debug logging. Debug
   logging is not an option that the user sees within the API. */

/* #define TIESRSI_DEBUG_LOG */

#ifdef TIESRSI_DEBUG_LOG

#if defined (WINCE)

#if ( _WIN32_WCE >= 0x500 )
/* Log file locations for WindowsMobile 5.0 Smartphone */
  #define LOGFILE "\\Application Data\\TIesr\\SIlog.txt"
  #define THREADFILE "\\Application Data\\TIesr\\SIthread.txt"
#else
/* Log file locations for WindowsMobile 2003SE Smartphone */
#define LOGFILE "\\Storage\\Application Data\\TIesr\\SIlog.txt"
#define THREADFILE "\\Storage\\Application Data\\TIesr\\SIthread.txt"
#endif

#else
/* Define location of log files for other platforms. */
#endif

#endif


/* Audio default constants */


/* Blocking flag for audio */
#define BLOCKING 1


/* Sample rate in Hz */
#define SAMPLE_RATE 8000

#if defined (LINUX)

/* Number of frames of data in circular buffer to maintain real time */
#define CIRCULAR_FRAMES 100
/* Time between audio device read attempts (if needed) in microseconds */
#define AUDIO_READ_RATE 20000
/* Number of audio device read rate frames of data in audio device buffer */
#define AUDIO_FRAMES 10
/* Audio priority of the frame audio thread */
#define  AUDIO_PRIORITY 20

#elif defined (WIN32) || defined (WINCE)

/* Number of frames of data in circular buffer to maintain real time */
#define CIRCULAR_FRAMES 500
/* Number of audio buffers holding data read from the audio device */
#define AUDIO_FRAMES 10
/* Audio priority of the frame audio thread */
#define  AUDIO_PRIORITY  THREAD_PRIORITY_ABOVE_NORMAL
/* For Windows TIesrFA this is the number of frames of audio data that
each audio buffer returned from the audio device driver will hold */
#define AUDIO_READ_RATE 2

#endif


/* Define TRUE/FALSE */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/*--------------------------------*/
/* Type definition of error types for local functions */
typedef enum TIesrSIL_Errors
{
  TIesrSILErrNone,
  TIesrSILErrFail,
  TIesrSILErrAudio,
  TIesrSILErrReco
} TIesrSIL_Error_t;


/*--------------------------------*/
/*
  Type definition of the allowable states of the TIesrSI recognizer.
*/
typedef enum TIesrSIL_State
{
  TIesrSIStateClosed,
  TIesrSIStateOpen,
  TIesrSIStateRunning
}TIesrSIL_State_t;


/*--------------------------------*/
/* 
   Type definition of the TIesrSI_Object which is a structure that
   holds all information and data necessary to encapsulate an instance
   of the TIesrSI recognizer.
*/

typedef struct TIesrSI_Object
{
      /* The TIesrSI recognizer engine */
      TIesrEngineSIRECOType engine;
      TIesr_t srchmemory;
      short* framedata;
      char* jacfile;      
      char* sbcfile;
      char* grammardir;
      TIesrEngineStatusType recostatus;
      TIesrEngineJACStatusType jacstatus;

    /* The audio channel and its parameters */
    TIesrFA_t audiodevice;
    unsigned int samplerate;
    unsigned int circularframes;
    unsigned int audioreadrate;
    unsigned int audioframes;
    int audiopriority;
    char* devicename;
    TIesrFA_Error_t audiostatus;


      /* TIesrSI API recognizer parameters */
      int state; /* State of the recognizer */


      /* audio output file pointer, allowing capture of 
      audio sent to recognizer. */
      FILE* audiofp;
     

      /* log output file pointer, allowing capture of
	 debug logging data */
#if defined (TIESRSI_DEBUG_LOG)
      FILE* logfp;
      FILE* threadfp;
#endif

#if defined (LINUX)
      /* Recognizer thread while running recognition */
      pthread_t threadid; 
      
      /* Recognizer mutex protecting shared variables */
      pthread_mutex_t recomutex; 
      
      /* Semaphore to detect recognizer started */ 
      sem_t startsemaphore; 

      /* Flag indicating recognizer thread started ok */
      int startok;

      /* Flag to indicate a request to stop recognition thread */
      int stopreco;
#elif defined (WIN32) || defined (WINCE)
      /* Recognizer thread while running recognition */
      HANDLE threadid; 
      
      /* Recognizer mutex protecting shared variables */
      HANDLE recomutex; 
      
      /* Event to detect recognizer started */ 
      HANDLE startevent; 

      /* Flag indicating recognizer thread started ok */
      int startok;

      /* Flag to indicate a request to stop recognition thread */
      int stopreco;

#endif
  
      /* Callback functions to generate recognition results */
      TIesrSI_Callback_t speakcb;
      TIesrSI_Callback_t donecb;
      void* cbdata;
  
      /* Recognizer variables */


} TIesrSI_Object_t;


/*--------------------------------*/
/* Local function prototypes */
static TIesrSIL_Error_t TIesrSIL_freeopen( TIesrSI_t aTIesrSI );

static TIesrSIL_Error_t TIesrSIL_initsync( TIesrSI_t aTIesrSI );

static TIesrSIL_Error_t TIesrSIL_resetsync( TIesrSI_t aTIesrSI );

#if defined (LINUX)
static void* TIesrSIL_thread( void* aTIesrSI );
#elif defined (WIN32) || defined (WINCE)
static DWORD TIesrSIL_thread( LPVOID aTIesrSI );
#endif

#endif /* _TIESRSI_H */
