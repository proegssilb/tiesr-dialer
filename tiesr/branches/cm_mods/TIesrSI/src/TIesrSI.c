/*=======================================================================

 *
 * TIesrSI.c
 *
 * TIesrSI API - top level user interface API for TIesr.
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
 This source contains the code necessary to implement the TIesrSI API
 as defined in TIesrSI_User.h, running the recognition as a
 separate thread.  TIesrSI utilizes the TIesrFA API for audio input
 and the TIesr Engine SI API for recognition.

======================================================================*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

/* Standard C operations */
#include <stdlib.h>
#include <string.h>

#if defined(LINUX)
#include <errno.h>
#endif

/* 
This contains the TIesrFA API and TIesr Engine SI API headers,
as well as the TIesrSI_User header.
*/
#include "TIesrSI.h"

/* Posix compliant implementation of some C Run-time library functions */
#if defined (WIN32) || defined (WINCE)
#define strdup  _strdup
#endif


#if defined( WIN32 ) || defined( WINCE )
/* Define entry point for the TIesrSI API DLL */
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
TIesrSI_create

This function creates and initializes an instance of the TIesrSI recognizer,
which is encapsulated in the TIesrSI_Object.
----------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_create( TIesrSI_t* aPtrToTIesrSI )
{
   TIesrSI_t aTIesrSI;

   /* Allocate memory for the TIesr object */
   *aPtrToTIesrSI = (TIesrSI_Object_t*)malloc( sizeof(TIesrSI_Object_t) );
   if( *aPtrToTIesrSI == NULL )
   {
      return TIesrSIErrNoMemory;
   }
   else
   {
      aTIesrSI = *aPtrToTIesrSI;
   }

#if defined (TIESRSI_DEBUG_LOG)
   aTIesrSI->threadfp = NULL;

   aTIesrSI->logfp = fopen(LOGFILE, "w" );
   if( aTIesrSI->logfp == NULL )
      return TIesrSIErrFail;

   fprintf( aTIesrSI->logfp, "SICreate\n" );
   fflush( aTIesrSI->logfp );
#endif


   /* Set status tracking variables, just in case TIesrSI_status is called
   immediately after TIesrSI_create. */
   aTIesrSI->audiostatus = TIesrFAErrNone;
   aTIesrSI->recostatus = eTIesrEngineSuccess;
   aTIesrSI->jacstatus = eTIesrEngineJACSuccess;


   /* Open the TIesr engine, which provides pointers to engine functions */
   TIesrEngineOpen( &(aTIesrSI->engine) );


   /* heap allocated objects are initialized here to ensure no misuse. */
   aTIesrSI->srchmemory = NULL;
   aTIesrSI->framedata = NULL;
   aTIesrSI->devicename = NULL;
   aTIesrSI->jacfile = NULL;
   aTIesrSI->sbcfile = NULL;
   aTIesrSI->audiofp = NULL;



#if defined (WIN32) || defined (WINCE)
   /* Windows synchronization objects initialized to NULL for bookkeeping */
   aTIesrSI->threadid = NULL;
   aTIesrSI->recomutex = NULL;
   aTIesrSI->startevent = NULL;
#endif

   /* Default settings just to be safe */
   aTIesrSI->startok = FALSE;
   aTIesrSI->stopreco = FALSE;


   /* Recognizer created ok, now in closed state */
   aTIesrSI->state = TIesrSIStateClosed;


#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SICreate success\n" );
   fflush( aTIesrSI->logfp );
#endif

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_open

This function opens the recognizer instance.  Opening a recognizer instance
consists of allocating memory resources for the recognizer search, loading
the grammar, doing JAC initialization, storing the callback functions,
and initializing an audio device.
-----------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_open( TIesrSI_t aTIesrSI,
			      const char* aAudioDevice,
			      const char* aGrammarDir,
			      const char* aJACFile,
			      const char* aSBCFile,
			      const unsigned int aMemorySize,
			      TIesrSI_Callback_t speakCallback,
			      TIesrSI_Callback_t doneCallback,
			      void* const cbData )
{
   TIesrEngineStatusType engStatus;
   TIesrEngineJACStatusType jacStatus;

   TIesrFA_Error_t faError;


   /* Recognizer must be in closed state */
   if( aTIesrSI->state != TIesrSIStateClosed )
   {
      return TIesrSIErrState;
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIOpen\n" );
   fflush( aTIesrSI->logfp );
#endif


   /* Set callback functions */
   aTIesrSI->speakcb = speakCallback;
   aTIesrSI->donecb = doneCallback;
   aTIesrSI->cbdata = cbData;


   /* Set default audio parameters */
   aTIesrSI->samplerate = SAMPLE_RATE;
   aTIesrSI->circularframes = CIRCULAR_FRAMES;
   aTIesrSI->audioreadrate = AUDIO_READ_RATE;
   aTIesrSI->audioframes = AUDIO_FRAMES;
   aTIesrSI->audiopriority = AUDIO_PRIORITY;


   /* Set status tracking variables */
   aTIesrSI->audiostatus = TIesrFAErrNone;
   aTIesrSI->recostatus = eTIesrEngineSuccess;
   aTIesrSI->jacstatus = eTIesrEngineJACSuccess;

   /* Set default conditions of variables */
   aTIesrSI->startok = FALSE;
   aTIesrSI->stopreco = FALSE;


   /* Allocate search memory */
   aTIesrSI->srchmemory = (TIesr_t)malloc( aMemorySize*sizeof(short) );
   if( aTIesrSI->srchmemory == NULL )
   {
      return TIesrSIErrNoMemory;
   }


   /* Allocate frame data memory */
   aTIesrSI->framedata = (short*)malloc( FRAME_LEN*sizeof(short) );
   if( aTIesrSI->framedata == NULL )
   {
      TIesrSIL_freeopen( aTIesrSI );
      return TIesrSIErrFail;
   }


   /* Save a copy of the audio device name */
   aTIesrSI->devicename = strdup( aAudioDevice );
   if( aTIesrSI->devicename == NULL )
   {
      TIesrSIL_freeopen( aTIesrSI );
      return TIesrSIErrFail;
   }


   /* Save a copy of the JAC file name */
   if( ! aJACFile )
   {
      TIesrSIL_freeopen( aTIesrSI );
      return TIesrSIErrFail;
   }

   aTIesrSI->jacfile = strdup( aJACFile );
   if( aTIesrSI->jacfile == NULL )
   {
      TIesrSIL_freeopen( aTIesrSI );
      return TIesrSIErrFail;
   }


   /* Save a copy of the SBC file name, if used */
   if( aSBCFile != NULL )
   {
      aTIesrSI->sbcfile = strdup( aSBCFile );
      if( aTIesrSI->sbcfile == NULL )
      {
         TIesrSIL_freeopen( aTIesrSI );
         return TIesrSIErrFail;
      }
   }


   /* Save a copy of the Grammar directory name */
   if( ! aGrammarDir )
   {
      TIesrSIL_freeopen( aTIesrSI );
      return TIesrSIErrFail;
   }

   aTIesrSI->grammardir = strdup( aGrammarDir );
   if( aTIesrSI->grammardir == NULL )
   {
      TIesrSIL_freeopen( aTIesrSI );
      return TIesrSIErrFail;
   }


   /* Open the recognizer engine */
   engStatus = (aTIesrSI->engine).OpenASR( (char*)aGrammarDir, 
      (int)aMemorySize, 
      aTIesrSI->srchmemory ); 

   if( engStatus != eTIesrEngineSuccess )
   {
      TIesrSIL_freeopen( aTIesrSI );
      aTIesrSI->recostatus = engStatus;
      return TIesrSIErrReco;
   }


   /* Initialize an audio channel */
   faError = TIesrFA_init( &aTIesrSI->audiodevice,
      aTIesrSI->samplerate,
      TIesrFALinear,
      FRAME_LEN,
      aTIesrSI->circularframes,
      aTIesrSI->audioframes,
      aTIesrSI->audioreadrate,
      aTIesrSI->devicename,
      aTIesrSI->audiopriority );

   if( faError != TIesrFAErrNone )
   {
      /* Close the successfully opened TIesr Engine instance */
      (aTIesrSI->engine).CloseASR( aTIesrSI->srchmemory );

      TIesrSIL_freeopen( aTIesrSI );

      aTIesrSI->audiostatus = faError;
      return TIesrSIErrAudio;
   }


   /* Load the JAC channel */
   jacStatus = (aTIesrSI->engine).JAC_load( aTIesrSI->jacfile, aTIesrSI->srchmemory,
      aTIesrSI->grammardir, aTIesrSI->sbcfile );

   if( jacStatus != eTIesrEngineJACSuccess )
   {
      /* Destroy the successfully initialized audio channel */
      faError = TIesrFA_destroy( &aTIesrSI->audiodevice );
      aTIesrSI->audiostatus = faError;

      /* Close the successfully opened TIesr Engine instance */
      (aTIesrSI->engine).CloseASR( aTIesrSI->srchmemory );

      TIesrSIL_freeopen( aTIesrSI );

      aTIesrSI->jacstatus = jacStatus;
      return TIesrSIErrJAC;
   }


   /* Successfully opened the recognizer and audio channel */
   aTIesrSI->state = TIesrSIStateOpen;

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIOpen success\n" );
   fflush( aTIesrSI->logfp );
#endif

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSIL_freeopen

Free objects allocated during TIesrSI_open.  This is a convenience function.
--------------------------------*/
TIesrSIL_Error_t TIesrSIL_freeopen( TIesrSI_t aTIesrSI )
{
   if(  aTIesrSI->devicename != NULL )
   {
      free( aTIesrSI->devicename);
      aTIesrSI->devicename = NULL;
   }

   if( aTIesrSI->srchmemory != NULL )
   {
      free( aTIesrSI->srchmemory );
      aTIesrSI->srchmemory = NULL;
   }

   if(  aTIesrSI->framedata != NULL )
   {
      free( aTIesrSI->framedata );
      aTIesrSI->framedata = NULL;
   }

   if(  aTIesrSI->jacfile != NULL )
   {
      free( aTIesrSI->jacfile);
      aTIesrSI->jacfile = NULL;
   }

   if(  aTIesrSI->sbcfile != NULL )
   {
      free( aTIesrSI->sbcfile);
      aTIesrSI->sbcfile = NULL;
   }

   if(  aTIesrSI->grammardir != NULL )
   {
      free( aTIesrSI->grammardir);
      aTIesrSI->grammardir = NULL;
   }

   return TIesrSILErrNone;
}


/*----------------------------------------------------------------
TIesrSI_getparams

Get recognizer parameter settings that are now in use.  This returns a 
structure that can be used to modify and set parameters.  This function
is commonly used to initialize parameter object with defaults.
-----------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_getparams( TIesrSI_t aTIesrSI, TIesrSI_Params_t* aParams )
{

   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIGetparams\n" );
   fflush( aTIesrSI->logfp );
#endif

   /* Recognizer settings */
   (aTIesrSI->engine).GetTIesrPrune( aTIesrSI->srchmemory, &aParams->pruneFactor );

   // This will be implemented when TwoPass research is complete
   // (aTIesrSI->engine).GetTIesrTwoPass( aTIesrSI->srchmemory, &aParams->useTwoPass ); 

   (aTIesrSI->engine).GetTIesrTransiWeight( aTIesrSI->srchmemory, &aParams->transiWeight );

   (aTIesrSI->engine).GetTIesrSAD( aTIesrSI->srchmemory, 
      &aParams->sadDelta,
      &aParams->sadMinDb,
      &aParams->sadBeginFrames,
      &aParams->sadEndFrames,
      &aParams->sadNoiseFloor );

   (aTIesrSI->engine).GetTIesrJAC( aTIesrSI->srchmemory,
      &aParams->jacRate,
      &aParams->jacLikelihood,
      &aParams->jacSegmentSize,
      &aParams->jacSmoothCoef,
      &aParams->jacDiscountFactor,
      &aParams->jacDeweightFactor,
      &aParams->jacSVAForgetFactor );


   (aTIesrSI->engine).GetTIesrSBC( aTIesrSI->srchmemory,
      &aParams->sbcForgetFactor );

   (aTIesrSI->engine).GetTIesrVolRange( aTIesrSI->srchmemory, 
      &aParams->lowVolume, 
      &aParams->highVolume );

   (aTIesrSI->engine).GetTIesrGaussSelection( 
      aTIesrSI->srchmemory, 
      &aParams->gselPctCoreClusters, 
      &aParams->gselPctInterClusters );

   (aTIesrSI->engine).GetTIesrOnlineReferenceModel( 
      &aParams->ormRefFrames,
      &aParams->ormNoiseLevelThreshold,
      &aParams->ormLowNoiseLLRThreshold,
      &aParams->ormHiNoiseLLRThreshold,
      &aParams->ormEOSLowLLRFrames,
      &aParams->ormEOSPctLLRThreshold,
      &aParams->ormEOSDelayFrames,
      &aParams->ormEOSFrmsAfterPeak, 
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).GetTIesrNoiseSubtraction( 
      &aParams->ssAlpha,
      &aParams->ssBeta,
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).GetTIesrConfidenceAdaptation( 
      &aParams->cnfAdaptThreshold,
      &aParams->cnfNbestThreshold, 
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).getRealTimeGuardTh( 
      &aParams->realTimeFrameThreshold,
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).getGetRealTGuard( 
      &aParams->realTimeGuard,
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).GetTIesrNBest(
      aTIesrSI->srchmemory, 
      &aParams->bNBest);


   /* Audio device settings */
   aParams->sampleRate = aTIesrSI->samplerate;
   aParams->circularFrames = aTIesrSI->circularframes;
   aParams->audioReadRate = aTIesrSI->audioreadrate;
   aParams->audioFrames = aTIesrSI->audioframes;
   aParams->audioPriority = aTIesrSI->audiopriority;


#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIGetparams success\n" );
   fflush( aTIesrSI->logfp );
#endif

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_setparams

Set the recognizer parameter settings that are to be used from the parameter structure.
--------------------------------*/
TIesrSI_Error_t TIesrSI_setparams( TIesrSI_t aTIesrSI, const TIesrSI_Params_t* aParams )
{
   TIesrFA_Error_t faError;

   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SISetparams\n" );
   fflush( aTIesrSI->logfp );
#endif

   /* Set TIesr recognizer settings */
   (aTIesrSI->engine).SetTIesrPrune( aTIesrSI->srchmemory, aParams->pruneFactor );

   // This will be implemented when TwoPass pass research is complete
   // (aTIesrSI->engine).SetTIesrTwoPass( aTIesrSI->srchmemory, aParams->useTwoPass ); 

   (aTIesrSI->engine).SetTIesrTransiWeight( aTIesrSI->srchmemory, aParams->transiWeight );

   (aTIesrSI->engine).SetTIesrSAD( aTIesrSI->srchmemory, 
      aParams->sadDelta,
      aParams->sadMinDb,
      aParams->sadBeginFrames,
      aParams->sadEndFrames,
      aParams->sadNoiseFloor );


   (aTIesrSI->engine).SetTIesrJAC( aTIesrSI->srchmemory,
      aParams->jacRate,
      aParams->jacLikelihood,
      aParams->jacSegmentSize,
      aParams->jacSmoothCoef,
      aParams->jacDiscountFactor,
      aParams->jacDeweightFactor,
      aParams->jacSVAForgetFactor );


   (aTIesrSI->engine).SetTIesrSBC( aTIesrSI->srchmemory,
      aParams->sbcForgetFactor );


   (aTIesrSI->engine).SetTIesrVolRange( aTIesrSI->srchmemory, 
      aParams->lowVolume, 
      aParams->highVolume );

   (aTIesrSI->engine).SetTIesrGaussSelection( 
      aTIesrSI->srchmemory, 
      aParams->gselPctCoreClusters, 
      aParams->gselPctInterClusters );


   (aTIesrSI->engine).SetTIesrOnlineReferenceModel( 
      aParams->ormRefFrames,
      aParams->ormNoiseLevelThreshold,
      aParams->ormLowNoiseLLRThreshold,
      aParams->ormHiNoiseLLRThreshold,
      aParams->ormEOSLowLLRFrames,
      aParams->ormEOSPctLLRThreshold,
      aParams->ormEOSDelayFrames,
      aParams->ormEOSFrmsAfterPeak,
      aTIesrSI->srchmemory );


   (aTIesrSI->engine).SetTIesrNoiseSubtraction( 
      aParams->ssAlpha,
      aParams->ssBeta,
      aTIesrSI->srchmemory );


   (aTIesrSI->engine).SetTIesrConfidenceAdaptation( 
      aParams->cnfAdaptThreshold,
      aParams->cnfNbestThreshold, 
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).setRealTimeGuardTh( 
      aParams->realTimeFrameThreshold,
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).setGetRealTGuard( 
      aParams->realTimeGuard,
      aTIesrSI->srchmemory );

   (aTIesrSI->engine).SetTIesrNBest(
      aTIesrSI->srchmemory, 
      aParams->bNBest);

    /* Determine if the audio channel needs to be re-initialized with new parameters */
    if( aParams->sampleRate != aTIesrSI->samplerate ||
            aParams->circularFrames != aTIesrSI->circularframes ||
            aParams->audioFrames != aTIesrSI->audioframes ||
            aParams->audioPriority != aTIesrSI->audiopriority ||
            aParams->audioReadRate != aTIesrSI->audioreadrate )
    {
        faError = TIesrFA_destroy( &aTIesrSI->audiodevice );
        aTIesrSI->audiostatus = faError;
        if( faError != TIesrFAErrNone )
        {
            return TIesrSIErrAudio;
        }

        aTIesrSI->samplerate = aParams->sampleRate;
        aTIesrSI->circularframes = aParams->circularFrames;
        aTIesrSI->audioreadrate = aParams->audioReadRate;
        aTIesrSI->audioframes = aParams->audioFrames;
        aTIesrSI->audiopriority = aParams->audioPriority;


        faError = TIesrFA_init( &aTIesrSI->audiodevice,
                aTIesrSI->samplerate,
                TIesrFALinear,
                FRAME_LEN,
                aTIesrSI->circularframes,
                aTIesrSI->audioframes,
                aTIesrSI->audioreadrate,
                aTIesrSI->devicename,
                aTIesrSI->audiopriority );

        aTIesrSI->audiostatus = faError;
        if( faError != TIesrFAErrNone )
        {
            return TIesrSIErrAudio;
        }
    }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SISetparams success\n" );
   fflush( aTIesrSI->logfp );
#endif

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_start

This function starts speech recognition of an utterance.  It first makes a
connection with the audio channel, and then starts a recognition thread
which starts collecting and processing the audio data through the TIesr
Engine.
-------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_start( TIesrSI_t aTIesrSI, int aPriority, char* aCaptureFile )
{
   TIesrSIL_Error_t silError;

#if defined (LINUX)
    int result;
    int threadresult;
    pthread_attr_t attr;
    struct sched_param sparam;
    void *threadError;

#elif defined (WIN32) || defined (WINCE)

#endif


   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }


#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStart\n" );
   fflush( aTIesrSI->logfp );
#endif


#ifdef TIESRSI_DEBUG_LOG
   aTIesrSI->threadfp = fopen( THREADFILE, "w" );
   if( aTIesrSI->threadfp == NULL )
      return TIesrSIErrFail;
#endif


   /* If the audio file name exists, try to open it for writing
   audio data */
   if( aCaptureFile )
   {
      aTIesrSI->audiofp = fopen( aCaptureFile, "wb" );
      if( !aTIesrSI->audiofp )
      {

#ifdef TIESRSI_DEBUG_LOG
         if( aTIesrSI->threadfp )
         {
            fclose(aTIesrSI->threadfp );
            aTIesrSI->threadfp = NULL;
         }
#endif          

         return TIesrSIErrCapture;
      }
   }
   else
   { 
      aTIesrSI->audiofp = NULL;
   }


   /* Initialize the sync objects used with the recognizer */
   silError = TIesrSIL_initsync( aTIesrSI );
   if( silError != TIesrSILErrNone )
   {
      if( aTIesrSI->audiofp )
      {
         fclose( aTIesrSI->audiofp );
         aTIesrSI->audiofp = NULL;
      }

#if defined (TIESRSI_DEBUG_LOG)
      if( aTIesrSI->threadfp )
      {
         fclose( aTIesrSI->threadfp );
         aTIesrSI->threadfp = NULL;
      }
#endif

      return TIesrSIErrFail;
   }


   /* Reset variables for the recognition */
   aTIesrSI->startok = FALSE;
   aTIesrSI->stopreco = FALSE;


   /* Start the recognition thread running */

#if defined (LINUX)

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStart create thread\n" );
#endif
   /* Set thread attributes - the requested priority is the only changed attribute */
   result = pthread_attr_init(&attr);
   result = pthread_attr_getschedparam( &attr, &sparam );
   sparam.sched_priority = aPriority;
   result = pthread_attr_setschedparam( &attr, &sparam );


   /* Start the recognition thread */
   threadresult = pthread_create( &(aTIesrSI->threadid), &attr,
      TIesrSIL_thread, (void *) aTIesrSI );

   result = pthread_attr_destroy(&attr);

   /* Check if the thread was created ok */
   if( threadresult != 0 )
   {
      TIesrSIL_resetsync( aTIesrSI );

      if( aTIesrSI->audiofp )
      {
         fclose( aTIesrSI->audiofp );
         aTIesrSI->audiofp = NULL;
      }

#if defined (TIESRSI_DEBUG_LOG)
      if( aTIesrSI->threadfp )
      {
         fclose( aTIesrSI->threadfp );
         aTIesrSI->threadfp = NULL;
      }
#endif

      return TIesrSIErrThread;
   }

#elif defined (WIN32) || defined (WINCE)

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStart create thread\n" );
#endif

   aTIesrSI->threadid = CreateThread( NULL, 0,
           (LPTHREAD_START_ROUTINE)TIesrSIL_thread, aTIesrSI,
      CREATE_SUSPENDED, NULL );
   if( aTIesrSI->threadid == NULL )
   {
      TIesrSIL_resetsync( aTIesrSI );

      /* Close audio capture file and log file if open */
      if( aTIesrSI->audiofp )
      {
         fclose( aTIesrSI->audiofp );
         aTIesrSI->audiofp = NULL;
      }
#if defined (TIESRSI_DEBUG_LOG)
      if( aTIesrSI->threadfp )
      {
         fclose( aTIesrSI->threadfp );
         aTIesrSI->logfp = NULL;
      }
#endif

      return TIesrSIErrThread;
   }

   SetThreadPriority( aTIesrSI->threadid, aPriority );

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStart run thread\n" );
   fflush( aTIesrSI->logfp );
#endif   
   
   ResumeThread( aTIesrSI->threadid );

#endif

   /* The thread was created ok.  Wait for it to signal that it has started. */
#if defined (LINUX)
   do
   {
      result = sem_wait( &( aTIesrSI->startsemaphore ) );
   }
   while( result && errno == EINTR );

#elif defined (WIN32) || defined (WINCE)
   WaitForSingleObject( aTIesrSI->startevent, INFINITE );
#endif

   /* If the recognition thread did not start ok, then wait for the thread to 
   terminate, and then notify the application that it failed */
   if( ! aTIesrSI->startok )
   {
#if defined (LINUX)
      pthread_join( aTIesrSI->threadid, &threadError );
#elif defined (WIN32) || defined (WINCE)
      WaitForSingleObject( aTIesrSI->threadid, INFINITE );
      CloseHandle( aTIesrSI->threadid );
      aTIesrSI->threadid = NULL;
#endif
      TIesrSIL_resetsync( aTIesrSI );

      /* Close audio capture file and log file if open */
      if( aTIesrSI->audiofp )
      {
         fclose( aTIesrSI->audiofp );
         aTIesrSI->audiofp = NULL;
      }

#if defined (TIESRSI_DEBUG_LOG)
      if( aTIesrSI->threadfp )
      {
         fclose( aTIesrSI->threadfp );
         aTIesrSI->threadfp = NULL;
      }
#endif

      return TIesrSIErrThread;
   }

   /* The recognizer started ok, and a recognition thread started ok */
   aTIesrSI->state = TIesrSIStateRunning;

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStart success\n" );
   fflush( aTIesrSI->logfp );
#endif
   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSIL_initsync

Initialize synchronization objects for the TIesrSI instance.
--------------------------------*/
TIesrSIL_Error_t TIesrSIL_initsync( TIesrSI_t aTIesrSI )
{

#if defined (LINUX)
   int error;

   error = pthread_mutex_init( &(aTIesrSI->recomutex), NULL );
   if( error )
   {
      return TIesrSILErrFail;
   }

   error = sem_init( &(aTIesrSI->startsemaphore), 0, 0 );   
   if( error )
   {
      error = pthread_mutex_destroy( &(aTIesrSI->recomutex) );

      return TIesrSILErrFail;
   }

#elif defined (WIN32) || defined (WINCE)

   aTIesrSI->recomutex = CreateMutex( NULL, FALSE, NULL );
   if( aTIesrSI->recomutex == NULL )
      return TIesrSILErrFail;

   aTIesrSI->startevent = CreateEvent( NULL, FALSE, FALSE, NULL );
   if( aTIesrSI->startevent == NULL )
   {
      CloseHandle( aTIesrSI->recomutex );
      aTIesrSI->recomutex = NULL;
      return TIesrSILErrFail;
   }

#endif

   return TIesrSILErrNone;
}


/*----------------------------------------------------------------
TIesrSIL_resetsync

Reset the synchronization objects for the TIesrSI instance.
--------------------------------*/
static TIesrSIL_Error_t TIesrSIL_resetsync( TIesrSI_t aTIesrSI )
{
   TIesrSIL_Error_t silError = TIesrSILErrNone;

#if defined (LINUX)

   int error = 0;

   error = pthread_mutex_destroy( &(aTIesrSI->recomutex) );
   if( error )
   {
      silError = TIesrSILErrFail;
   }

   error = sem_destroy( &(aTIesrSI->startsemaphore) );   
   if( error != 0 )
   {
      silError = TIesrSILErrFail;
   }

#elif defined (WIN32) || defined (WINCE)
   if( ! CloseHandle( aTIesrSI->recomutex ) )
   {
      silError = TIesrSILErrFail;
   }

   if( ! CloseHandle( aTIesrSI->startevent ) )
   {
      silError = TIesrSILErrFail;
   }

#endif


   return silError;
}


/*----------------------------------------------------------------
TIesrSIL_thread

This is the TIesr recognition thread function that is started by
a call to TIesrSI_start.
--------------------------------*/
#if defined (LINUX)
void* TIesrSIL_thread( void* aArg )
#elif defined (WIN32) || defined (WINCE)
DWORD TIesrSIL_thread( LPVOID aArg )
#endif
{
   TIesrFA_Error_t faError;
   TIesrFA_Error_t faStopError;
   TIesrFA_Error_t faCloseError;
   TIesrEngineStatusType engStatus;
   TIesrEngineJACStatusType jacStatus;
   TIesrSI_Error_t siError;

   TIesrSI_t aTIesrSI = (TIesrSI_t)aArg;

   unsigned int framesQueued;

   int recoEnded = FALSE;
   int startCalled = FALSE;


#ifdef TIESRSI_DEBUG_LOG
   int frame = 0;
   fprintf( aTIesrSI->threadfp, "SIThread\n" );
   fflush( aTIesrSI->threadfp );
#endif


   /* Ensure that initially flag set to indicate thread did not start ok */
   aTIesrSI->startok = FALSE;


   /* Open the search engine, which prepares it for recognition. On failure
   inform the start function via the start semaphore and exit the thread. */
   engStatus = (aTIesrSI->engine).OpenSearchEngine( aTIesrSI->srchmemory );
   aTIesrSI->recostatus = engStatus;
   if( engStatus != eTIesrEngineSuccess )
   {
#if defined (LINUX)
      sem_post( &aTIesrSI->startsemaphore );
      return (void*)TIesrSILErrReco;
#elif defined (WIN32) || defined (WINCE)
      SetEvent( aTIesrSI->startevent );
      return TIesrSILErrReco;
#endif
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->threadfp, "SIThread opening FA\n" );
   fflush( aTIesrSI->threadfp );
#endif

   /* Open an audio channel which connects to the audio device.  On failure
   inform the start function via the start semaphore and exit the thread. */
   faError = TIesrFA_open( &aTIesrSI->audiodevice );
   aTIesrSI->audiostatus = faError;
   if( faError != TIesrFAErrNone )
   {
#if defined (LINUX)
      sem_post( &aTIesrSI->startsemaphore );
      return (void*)TIesrSILErrAudio;
#elif defined (WIN32) || defined (WINCE)
      SetEvent( aTIesrSI->startevent );
      return TIesrSILErrAudio;
#endif
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->threadfp, "SIThread starting FA\n" );
   fflush( aTIesrSI->threadfp );
#endif

   /* Start collection of audio samples into the circular frame buffer. On
   failure, close the audio channel, inform the start function via the
   start semaphore and exit the thread. */
   faError = TIesrFA_start( &aTIesrSI->audiodevice );
   aTIesrSI->audiostatus = faError;
   if( faError != TIesrFAErrNone )
   {
      faCloseError = TIesrFA_close( &aTIesrSI->audiodevice );
#if defined (LINUX)
      sem_post( &aTIesrSI->startsemaphore );
      return (void*)TIesrSILErrAudio;
#elif defined (WIN32) || defined (WINCE)
      SetEvent( aTIesrSI->startevent );
      return TIesrSILErrAudio;
#endif
   }


   /* Audio collection has started successfully, and the recognizer engine is
   opened succesfully.  Notify the start function that recognition started
   successfully. */
   aTIesrSI->startok = TRUE;
#if defined (LINUX)
   sem_post( &aTIesrSI->startsemaphore );
#elif defined (WIN32) || defined (WINCE)
   SetEvent( aTIesrSI->startevent );
#endif

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->threadfp, "SIThread start loop\n" );
   fflush( aTIesrSI->threadfp );
#endif

   /* Get the first frame of data, blocking until it is received */
   faError = TIesrFA_getframe( &aTIesrSI->audiodevice, (unsigned char*)aTIesrSI->framedata, 
      BLOCKING, &framesQueued );


   /* Enter a processing loop processing frames of data until recognition complete,
   a request to terminate recognition, or failure */
   while( faError == TIesrFAErrNone && !aTIesrSI->stopreco )
   {
      /* If user wants to save audio data, write to a file */
      if( aTIesrSI->audiofp )
      {
         fwrite( (void*)aTIesrSI->framedata, sizeof(short),
            FRAME_LEN, aTIesrSI->audiofp );
      }

#if defined (TIESRSI_DEBUG_LOG)
      /* If debug logging, capture number of frames queued */
      if( aTIesrSI->threadfp )
      {
         fprintf( aTIesrSI->threadfp, "Queued: %d\n", framesQueued );
         fflush( aTIesrSI->threadfp );
      }
#endif

#ifdef TIESRSI_DEBUG_LOG
      fprintf( aTIesrSI->threadfp, "SIThread calling engine %d\n", frame );
      fflush( aTIesrSI->threadfp );
#endif

      /* Process the frame of data in the TIesr Engine */
      engStatus = (aTIesrSI->engine).CallSearchEngine( aTIesrSI->framedata,
         aTIesrSI->srchmemory, framesQueued );

#ifdef TIESRSI_DEBUG_LOG
      fprintf( aTIesrSI->threadfp, "SIThread engine done %d %d\n", frame, engStatus );
      fflush( aTIesrSI->threadfp );
#endif
      aTIesrSI->recostatus = engStatus;

      /* Catch recognition engine error and exit loop immediately */
      if( engStatus != eTIesrEngineSuccess )
      {
         break;
      }

      /* If LIVEMODE defined in tieser_config.h then when the utterance end is detected
      the recognizer should stop processing frames of data */

#ifdef LIVEMODE

      recoEnded = (aTIesrSI->engine).SpeechEnded( aTIesrSI->srchmemory );
      if( recoEnded )
      {  
         /* Can't print in an API.  Protect this with a debug define macro.
         printf("reco ended \n"); */
         break;
      }

#endif


      /* Determine if speech started early or if the JAC has completed one pass */
      if( !startCalled && aTIesrSI->speakcb != NULL &&
         (  (aTIesrSI->engine).GetCycleCount( aTIesrSI->srchmemory ) > 0  || 
         (aTIesrSI->engine).SpeechDetected( aTIesrSI->srchmemory ) ) )
      {
         startCalled = TRUE;

         if( (aTIesrSI->engine).GetCycleCount( aTIesrSI->srchmemory ) > 0 )
         {
            (aTIesrSI->speakcb)(aTIesrSI->cbdata, TIesrSIErrNone );
         }
         else
         {
            (aTIesrSI->speakcb)(aTIesrSI->cbdata, TIesrSIErrSpeechEarly );
         }
      }   

      /* Get next frame of data */
      faError = TIesrFA_getframe( &aTIesrSI->audiodevice, 
         (unsigned char*)aTIesrSI->framedata, BLOCKING,
         &framesQueued );

      /* If input is from a file, then recognition should stop if end of file
      condition returned */
      if ( faError == TIesrFAErrEOF )
      { 
         recoEnded = 1; 
         break;
      }
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->threadfp, "SIThread exit loop\n" );
   fflush( aTIesrSI->threadfp );
#endif

   /* The loop has been exited. Stop data collection and disconnect from the channel.
   Determine audio status at end of recognition */
   faStopError = TIesrFA_stop( &aTIesrSI->audiodevice );   
   faCloseError = TIesrFA_close( &aTIesrSI->audiodevice );
   if( faError != TIesrFAErrNone && faError != TIesrFAErrEOF )
      aTIesrSI->audiostatus = faError;
   else if( faStopError != TIesrFAErrNone )
      aTIesrSI->audiostatus = faStopError;
   else if( faCloseError != TIesrFAErrNone )
      aTIesrSI->audiostatus = faCloseError;
   else
      aTIesrSI->audiostatus = faError;


   /* Close the search engine, which among other things does a backtrace
   for the recognition result and sets volume status */
   if( recoEnded )
   {
      engStatus = (aTIesrSI->engine).CloseSearchEngine( engStatus, 
         aTIesrSI->srchmemory );

      aTIesrSI->recostatus = engStatus;
   }

   /* If recognition ended successfully, and user is not trying to shut down
   the recognizer, do JAC update */

   /*  Now recommend doing JAC update all the time, since
   modifications to JAC ensure all means updated if speech starts.  Thus there
   is not a need to check if at least one JAC cycle completed.

   if( recoEnded && engStatus == eTIesrEngineSuccess && !aTIesrSI->stopreco &&
   (aTIesrSI->engine).GetCycleCount( aTIesrSI->srchmemory ) > 0 )
   */
   if( recoEnded && engStatus == eTIesrEngineSuccess && !aTIesrSI->stopreco )        
   {
      jacStatus = (aTIesrSI->engine).JAC_update( aTIesrSI->srchmemory );

      aTIesrSI->jacstatus = jacStatus;
   }

   /* Notify the application of recognition thread completion */
   if( aTIesrSI->donecb != NULL )
   {
      if( ( aTIesrSI->audiostatus != TIesrFAErrNone && 
         aTIesrSI->audiostatus != TIesrFAErrEOF ) &&
         ( aTIesrSI->recostatus != eTIesrEngineSuccess || 
         ( (aTIesrSI->jacstatus != eTIesrEngineJACSuccess) && 
         (aTIesrSI->jacstatus != eTIesrEngineJACNoAlignment ) ) ) )
         siError = TIesrSIErrBoth;
      else if( aTIesrSI->audiostatus != TIesrFAErrNone && 
         aTIesrSI->audiostatus != TIesrFAErrEOF )
         siError = TIesrSIErrAudio;
      else if( aTIesrSI->recostatus != eTIesrEngineSuccess ||
         ( (aTIesrSI->jacstatus != eTIesrEngineJACSuccess) && 
         (aTIesrSI->jacstatus != eTIesrEngineJACNoAlignment ) ))
         siError = TIesrSIErrReco;
      else
         siError = TIesrSIErrNone;

      (aTIesrSI->donecb)( aTIesrSI->cbdata, siError );
   }

   /* Can't print in an API.  Protect this with a debug macro
   if( aTIesrSI->recostatus ==  eTIesrEngineBeamCellOverflow)  printf ("---BEAM OVER FLOW\n"); */

   /* Terminate the thread after a recognition process. */

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->threadfp, "SIThread ended\n" );
   fflush( aTIesrSI->threadfp );
#endif

#if defined (LINUX)
   return (void*)TIesrSILErrNone;
#elif defined (WIN32) || defined (WINCE)
   return TIesrSILErrNone;
#endif
}


/*----------------------------------------------------------------
TIesrSI_stop

This function stops recognition.  It may stop recognition in the middle
of the task, or after the recognition is complete and has signaled it
is done by calling the callback function.  This function ensures that
the threads have shut down appropriately.
---------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_stop( TIesrSI_t aTIesrSI )
{
#if defined(LINUX)
   TIesrSIL_Error_t threadError;
   void* threadReturn;
#endif

   /* Check state to ensure a recognizer thread is running */
   if( aTIesrSI->state != TIesrSIStateRunning )
   {
      return TIesrSIErrState;
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStop\n" );
   fflush( aTIesrSI->logfp );
#endif

   /* Set flag to force the recognizer thread to terminate, if that has
   not happened already due to normal end of recognition.  */
   aTIesrSI->stopreco = TRUE;


   /* Wait for the recognizer thread to stop */
#if defined (LINUX)
   pthread_join( aTIesrSI->threadid, &threadReturn );
   threadError = (TIesrSIL_Error_t)threadReturn;
#elif defined (WIN32) || defined (WINCE)
   WaitForSingleObject( aTIesrSI->threadid, INFINITE );
   CloseHandle( aTIesrSI->threadid );
   aTIesrSI->threadid = NULL;
#endif

   /* Reset sync objects since they are no longer needed for the thread */
   TIesrSIL_resetsync( aTIesrSI );


   /* If the user wanted audio capture, close the capture file */
   if( aTIesrSI->audiofp )
   {
      fclose( aTIesrSI->audiofp );
      aTIesrSI->audiofp = NULL;
   }

#if defined (TIESRSI_DEBUG_LOG)
   if( aTIesrSI->threadfp )
   {
      fclose( aTIesrSI->threadfp );
      aTIesrSI->threadfp = NULL;
   }
#endif

   /* Recognizer has successfully stopped running */
   aTIesrSI->state = TIesrSIStateOpen;

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIStop success\n" );
   fflush( aTIesrSI->logfp );
#endif
   return TIesrSIErrNone;
}

/*----------------------------------------------------------------
TIesrSI_nbcount

Get the number of hypothesis recognized.  This assumes that a valid recognition
result has completed.
------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_nbcount( TIesrSI_t aTIesrSI, short* aNumHyp) 
{
   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }

   *aNumHyp = (aTIesrSI->engine).GetNumNbests(aTIesrSI->srchmemory );

   return TIesrSIErrNone;
}

/*----------------------------------------------------------------
TIesrSI_wdcount

Get the number of words recognized.  This assumes that a valid recognition
result has completed.
------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_wdcount( TIesrSI_t aTIesrSI, unsigned short* aNumWords ,
                                short iHypIdx)
{
   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }

   *aNumWords = (aTIesrSI->engine).GetAnswerCount( aTIesrSI->srchmemory, iHypIdx);

   return TIesrSIErrNone;
}

/*----------------------------------------------------------------
TIesrSI_word

Get a pointer to a word that was recognized during the last recognition.
This assumes a valid recognition result has completed.
------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_word( TIesrSI_t aTIesrSI, unsigned short aWordNum, 
                             const char** aWord , short iHypIdx)
{
   short numHyp;
   unsigned short numWords;

   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }


   numHyp = (aTIesrSI->engine).GetNumNbests(aTIesrSI->srchmemory );
   if( iHypIdx < 0 || iHypIdx >= numHyp )
   {
      *aWord = NULL;
      return TIesrSIErrFail;
   }

   numWords = (aTIesrSI->engine).GetAnswerCount( aTIesrSI->srchmemory , iHypIdx);
   if( aWordNum >= numWords )
   {
      *aWord = NULL;
      return TIesrSIErrFail;
   }


   *aWord = (aTIesrSI->engine).GetAnswerWord( aWordNum, aTIesrSI->srchmemory , iHypIdx);

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_status

This function is normally not used.  It can provide additional
information in case one of the TIesrSI_* functions returns an error
status.  It returns a status integer values for the audio channel, the
recognizer engine, and the jac adaptation processing that may provide
help in diagnosis.  These values are set, even by the recognizer
thread, but are not protected by any synchronization object.  They are
best obtained when the recognizer is not running.
-------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_status( TIesrSI_t aTIesrSI, int* aRecoStatus, 
                               int* aAudioStatus, int* aJacStatus )
{
   *aRecoStatus = (int)aTIesrSI->recostatus;
   *aAudioStatus = (int)aTIesrSI->audiostatus;
   *aJacStatus = (int)aTIesrSI->jacstatus;

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_score

This function provides the likelihood score information for the result of the
last recognition operation.  This function is normally not necessary for
a voice-enabled application.  However, the likelihood score can indicate whether
the input speech matched the models well or not.  Larger values of score indicate
better likelihood.
------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_score( TIesrSI_t aTIesrSI, long* aScore )
{
   *aScore = (aTIesrSI->engine).GetTotalScore( aTIesrSI->srchmemory );

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_frmcount

This function provides the number of frames that the last speech recogntion
result consumed, excluding the portion of the input signal not determined to
be speech.  This function is normally not necessary for a voice-enabled
application.  However, it may be helpful for diagnosing some problems.
----------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_frmcount( TIesrSI_t aTIesrSI, short* aNumFrames )
{
   *aNumFrames = (aTIesrSI->engine).GetFrameCount( aTIesrSI->srchmemory );

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_volume

This function returns the volume corresponding to the last speech
recognition result.  It is a crude measure that just indicates
volume was too low, high, or ok, by returning 0,1, or 2
respectively.  The crude measure is presently based on the absolute
maximum sample value in the speech utterance signal.
----------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_volume( TIesrSI_t aTIesrSI, short* aVolume )
{
   *aVolume = (aTIesrSI->engine).GetVolumeStatus( aTIesrSI->srchmemory );

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_confidence

This function returns the confidence corresponding to the last
speech recognition result.  This is a measure of likelihood that the
recognition was correct.
----------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_confidence(  TIesrSI_t aTIesrSI, short* aConfidence )
{
   *aConfidence = (aTIesrSI->engine).GetConfidenceScore( aTIesrSI->srchmemory );

   return TIesrSIErrNone;
}


/*----------------------------------------------------------------
TIesrSI_close

This function closes the recognizer and deallocates any objects that were
allocated when opened.  It also closes the JAC processing, storing a channel
estimate.
------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_close( TIesrSI_t aTIesrSI )
{
   TIesrFA_Error_t faError;
   TIesrEngineJACStatusType jacStatus;

   if( aTIesrSI->state != TIesrSIStateOpen )
   {
      return TIesrSIErrState;
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIClose\n" );
   fflush( aTIesrSI->logfp );
#endif

   /* Save JAC  estimate and close the recognizer. */
   jacStatus = (aTIesrSI->engine).JAC_save(aTIesrSI->jacfile, aTIesrSI->srchmemory );

   aTIesrSI->jacstatus = jacStatus;

   (aTIesrSI->engine).CloseASR( aTIesrSI->srchmemory );

   /* Destroy the audio channel resources */
   faError = TIesrFA_destroy( &aTIesrSI->audiodevice );

   aTIesrSI->audiostatus = faError;


   /* Free resources allocated during open */
   TIesrSIL_freeopen( aTIesrSI );

   /* State is now closed, regardless of failures */
   aTIesrSI->state = TIesrSIStateClosed;

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIClose end %d\n", faError );
   fflush( aTIesrSI->logfp );
#endif

   if( faError != TIesrFAErrNone && jacStatus != eTIesrEngineJACSuccess )
      return TIesrSIErrBoth;
   else if(  faError != TIesrFAErrNone )
      return TIesrSIErrAudio;
   else if( jacStatus != eTIesrEngineJACSuccess )
      return TIesrSIErrJAC;
   else
   {
      return TIesrSIErrNone;
   }
}


/*----------------------------------------------------------------
TIesrSI_destroy

This function destroys the TIesrSI object specified in the argument.  This
completely destroys the instance of the recognizer.  This completely removes
the TIesrSI object that was created with TIesrSI_create, and frees all
resources that were allocated internally.  This must be called as the last
part of using a recognizer instance to ensure that all allocated resources
have been deallocated and no leakage occurs.  After this function is called
the pointer is no longer valid, and should be set to NULL.
-------------------------------------------------------------------*/
TIESRSI_API
TIesrSI_Error_t TIesrSI_destroy( TIesrSI_t aTIesrSI )
{

   if( aTIesrSI->state != TIesrSIStateClosed )
   {
      return TIesrSIErrState;
   }

#ifdef TIESRSI_DEBUG_LOG
   fprintf( aTIesrSI->logfp, "SIDestroy\n" );
   fflush( aTIesrSI->logfp );
#endif

   /* Close the engine instance */
   TIesrEngineClose( &(aTIesrSI->engine) );

#ifdef TIESRSI_DEBUG_LOG
   if( aTIesrSI->logfp )
      fclose( aTIesrSI->logfp );
    
   if( aTIesrSI->threadfp )
      fclose( aTIesrSI->threadfp );
#endif

   /* deallocate the TIesrSI instance */
   free( aTIesrSI );

   return TIesrSIErrNone;
}
