/*=======================================================================
 TIesrSI_User.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


   This header defines the interface that implements a TIesr Speaker Independent
  speech recognition API.  Application designers may use libraries that
  implement this API to voice enable their applications.  The API presents a
  simple and clean set of C functions that utilize the TIesrEngine API as the
  basis for the speech recognizer, and TIesrFA API as the frame audio data
  collection interface, without the application designer required to know the
  intricate details of these two APIs.  Note that the TIesrSI API runs the
  speech recognizer and the audio collection in two separate threads apart from
  the TIesrSI API.

  Prior to using the TIesrSI API to do speech recognition, the user must
  prepare a grammar and model set that describes the utterances to be
  recognized.  The grammar and models are placed in a known directory.  The
  TIesrFlex API is available for creating the grammar and model set.

======================================================================*/


#ifndef _TIESRSI_USER_H
#define _TIESRSI_USER_H


#if defined (WIN32) || defined (WINCE)

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TIESRSI_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TIESRSI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TIESRSI_EXPORTS
#define TIESRSI_API __declspec(dllexport)
#else
#define TIESRSI_API __declspec(dllimport)
#endif

#else
#define TIESRSI_API 

#endif

/*---------------------------------------------------------------- 
   TIesrSI_Errors is an enumeration that defines the status values
   associated with operations provided by functions of the TIesrSI
   API.  A brief description of the errors is as follows:

   TIesrSIErrNone = No error, operation completed successfully.

   TIesrSIErrFail = The function failed to complete successfully.

   TIesrSIErrNoMemory = The function could not complete, out of memory.

   TIesrSIErrState = Attempt to do something out of sequence, for example,
   trying to open an already open recognizer.

   TIesrSIErrReco = The recognizer reported a failure during processing.

   TIesrSIErrAudio = The audio channel reported a collection failure.

   TIesrSIErrBoth = Both of recognizer and audio reported failures.

   TIesrSIErrSpeechEarly = Speech occurred prior to completion of adaptation
   to the present noise environment.

   TIesrSIErrThread = The recognizer thread could not start successfully.

   TIesrSIErrCapture = The recognizer failed to open file or capture audio data.
------------------------------------------------------------------------*/
typedef enum TIesrSI_Errors
{
   TIesrSIErrNone,
   TIesrSIErrFail,
   TIesrSIErrNoMemory,
   TIesrSIErrState,
   TIesrSIErrReco,
   TIesrSIErrJAC,
   TIesrSIErrAudio,
   TIesrSIErrBoth,
   TIesrSIErrSpeechEarly,
   TIesrSIErrThread,
   TIesrSIErrCapture
} TIesrSI_Error_t;


/*------------------------------------------------------------------------- 
   The TIesrSI_Object defines and contains all data that comprise an instance
   of the TIesrSI recognizer.  A pointer to this object is defined as
   TIesrSI_t.  The application designer will only interact with the recognizer
   through the pointer to the object, and will never operate directly on the
   contents of the TIesrSI_Object.
---------------------------------------------------------------------------*/
typedef struct TIesrSI_Object* TIesrSI_t;


/*-------------------------------------------------------------------------
  TIesrSI provides results by calling two callback functions.  One function is
  called when the recognizer has been started, has performed adaptation to the
  present noise environment, and is ready for the user to start speaking. The
  second function is called when the recognizer has completed an attempt to
  recognize an utterance. Both functions are called from the recognizer thread
  which is running outside of the main application thread.  These functions
  must execute quickly to maintain real time, and should normally just signal
  events that have occurred that should be processed within the main
  application thread in some kind of event loop processing.  A pointer
  argument is specified by the application to be used as an argument of
  the callback function.  TIesrSI also supplies an error value, indicating
  status of the recognizer when the callback function is called.

  NOTE: The callback functions must not call any TIesrSI_* functions, since
  the callback functions are called outside of the main thread context. 

  See TIesrSI_open.
----------------------------------------------------------------------------*/
typedef void (*TIesrSI_Callback_t)( void* aAppArgument, TIesrSI_Error_t aError );


/*---------------------------------------------------------------------------  
    The TIesrSI_Parameters object defines the parameters that may be set for the
    TIesrEngine, once it is opened.  During opening, default values of these
    parameters are set. Once the TIesr recognizer is opened, this object may
    be used to set values that tailor the recognizer to a specific recognition
    task.  These are advanced settings that can be used to improve performance
    of the recognizer and adjust between processing time and processing
    accuracy.  Consult documentation for detailed description before setting each
    of these parameters.
-----------------------------------------------------------------------------*/
typedef struct TIesrSI_Parameters
{
      /* TIesr parameters */
      short pruneFactor;
      short transiWeight;

      short sadDelta;
      short sadMinDb;
      short sadBeginFrames;
      short sadEndFrames;
      short sadNoiseFloor;

      short jacRate;
      short jacLikelihood;
      short jacSegmentSize;
      short jacSmoothCoef;
      short jacDiscountFactor;
      short jacDeweightFactor;
      short jacSVAForgetFactor;
      short sbcForgetFactor;

      unsigned short lowVolume;
      unsigned short highVolume;

      unsigned short gselPctCoreClusters;
      unsigned short gselPctInterClusters;

      short ormRefFrames;
      short ormNoiseLevelThreshold;
      short ormLowNoiseLLRThreshold;
      short ormHiNoiseLLRThreshold;
      short ormEOSLowLLRFrames;
      unsigned short ormEOSPctLLRThreshold;
      short ormEOSDelayFrames;
      short ormEOSFrmsAfterPeak;

      unsigned short ssAlpha;
      short ssBeta;
      
      short cnfAdaptThreshold;
      short cnfNbestThreshold;

      short realTimeFrameThreshold;
      short realTimeGuard;

      /*audio channel parameters */
      int sampleRate;
      int circularFrames;
      int audioReadRate;
      int audioFrames;
      int audioPriority;

      short bNBest; 
  
      short useTwoPass; 

} TIesrSI_Params_t;

    

/*----------------------------------------------------------------
  The functional interface to TIesrSI is given below
----------------------------------------------------------------*/

/*
  Do this in order for the interface to work with C and C++.  The
  TIesrSI API is compiled in C, so if used with C++ the function
  names must not include the signature.
*/

#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------------------------------------------------- 
   TIesrSI_create creates an instance of the recognizer and initializes
   internal parameters necessary for operation of the recognizer.  It
   places a pointer of type TIesrSI_t to the newly created TIesrSI_Object
   in the location specified by aPtrToTIesrSI.
---------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_create( TIesrSI_t* aPtrToTIesrSI );


/*----------------------------------------------------------------------
  TIesrSI_open opens the recognizer and prepares it for recognition by loading
  the grammar used for the recognition. It specifies the name of the audio
  channel to open and it also specifies the location of the JAC channel
  compensation file that will be used for adjusting to varying channel
  conditions.  If this file does not exist, it will be created.

  A filename for the SBC tree definition file is specified, and must exist
  and have the appropriate phone tree definitions that match the acoustic
  model set in use.

  The application also specifies how much memory (in short values) to allow the
  recognizer to allocate for recognition.  This memory is allocated on the
  heap, and will be deallocated when the recognizer is closed.

  Two callback functions are specified, along with a pointer to data to be
  used as an argument in the callback.  The recognizer also returns an
  error value to indicate state of the recognizer when the function is
  called. These functions are both called outside the main application thread
  in the recognizer thread, and so should complete very quickly in order to
  maintain real time processing.  They should not call any TIesrSI_* function.

  The aSpeakCallback function is called when the recognizer is ready for the speaker
  to speak.  The error value may indicate that the speaker is already speaking too
  early, which may reduce recognizer performance.  There is no guarantee that
  this function will ever be called, since there may be a failure in recognition,
  or the application may stop the recognizer before it is ready for speech.

  The second function, aDoneCallback, is called when recognition is complete.
  If specified, it is guaranteed to be called once recognition starts successfully.
  The error value will indicate whether recognition successfully completed or
  completed with some error. 

  If a function pointer is NULL, then the function will not be called.
-------------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_open( TIesrSI_t aTIesrSI,
				      const char* aAudioDevice,
				      const char* aGrammarDir,
				      const char* aJACFile,
				      const char* aSBCFile,
				      const unsigned int aMemorySize,
				      TIesrSI_Callback_t aSpeakCallback,
				      TIesrSI_Callback_t aDoneCallback,
				      void* const aCbData );


/*----------------------------------------------------------------------- 
   Once the recognizer is open, the application can set several parameters
   that affect the operation of the recognizer, as described by the
   TIesrSI_Parameters object.  These parameters all are initialized with default
   values when the recognizer is opened, but may be changed after opening.  
   Parameters can not be changed while the recognizer is performing a recognition
   task. Below are the functions for setting the recognizer parameters.
--------------------------------------------------------------------------*/
        
    /* Fill a TIesrSI_Parameters object with the values presently in use in
    the recognizer */
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_getparams( TIesrSI_t aTIesrSI,  
        TIesrSI_Params_t* aParams );


    /* Set the parameters for the recognizer to the values contained in the
    TIesrSI_Parameters object. */
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_setparams( TIesrSI_t aTIesrSI, 
        const TIesrSI_Params_t* aParams );


/*------------------------------------------------------------------------ 
   Start a TIesrSI recognition.  This will start audio data collection
   (usually in a separate high priority thread in order to maintain real time)
   and will start recognition in a another separate thread.  The application
   can set the priority for the recognition thread, but this should normally
   be set to the same priority as the main thread or lower, so that it runs at
   the same priority or lower than the application. That way it will not
   interfere with real-time interaction with the user.

   A capture file name may be specified.  If the parameter is not NULL, then the
   audio used for recognition will be written to the capture file.

   NOTE: Each successful call of TIesrSI_start must be accompanied by a single
   call to TIesrSI_stop, either before recognition is complete to stop
   recognition early, or after recognition is completed.
-------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_start( TIesrSI_t aTIesrSI, const int aPriority,
        char* aCaptureFile );


/*--------------------------------------------------------------------------
  Stop a TIesrSI recognition.  This function stops a recognition task.  This
  may be called before the recognizer has had time to complete the recognition
  task, or after the recognizer has completed recognition.  

  NOTE: This function must be called once for each successful call to
  TIesrSI_start.  It must be called prior to getting recognition results.

  This function ensures that the audio data collection and recognition threads
  have completed.  Since the recognizer is running in a separate thread,
  recognition may complete normally before this function is completed.
  However, it is guaranteed that when this function returns that the
  recognizer and audio data collection threads have terminated.  It is the
  responsibility of the application designer to correctly handle any callback
  functions that the recognizer might make prior to termination of the
  recognition thread.  Once the recognizer has stopped, it may be started
  again simply by another call to TIesrSI_start.
---------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_stop( TIesrSI_t aTIesrSI );


/*----------------------------------------------------------------
TIesrSI_nbcount

Get the number of hypothesis recognized.  This assumes that a valid recognition
result has completed. 
------------------------------------------------------------------*/
  TIESRSI_API
  TIesrSI_Error_t TIesrSI_nbcount( TIesrSI_t aTIesrSI, short* aNumHyp) ;

/*-----------------------------------------------------------------------
  Get the number of words that were recognized during the last recognition.
  This must be called after a a successful recognition has taken place, as
  indicated by the aDoneCallback function, and after TIesrSI_stop is called.
--------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_wdcount( TIesrSI_t aTIesrSI, 
        unsigned short* aNumWords , short iHypIdx);

/*-------------------------------------------------------------------------
  Get a pointer to a word that was recognized during the last recognition.
  This must be called after a a successful recognition has taken place, as
  indicated by the aDoneCallback function, and after TIesrSI_stop is called.
  Word indices outside of the actual number of words recognized will
  return a NULL pointer.
--------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_word( TIesrSI_t aTIesrSI, unsigned short aWordNum, 
        const char** aWord , short iHypIdx);

/*--------------------------------------------------------------------------- 
   Get the likelihood score information for the result of the last recognition
   operation.  The likelihood score can indicate whether the input speech
   matched the models well or not.
------------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_score( TIesrSI_t aTIesrSI, long* aScore );


/*-------------------------------------------------------------------------- 
   Get the number of audio frames that the recognizer processed to arrive at
   the last recognition result.  This function may be helpful to diagnose
   recognition problems.  It is generally not necessary for enabling
   applications.
----------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_frmcount( TIesrSI_t aTIesrSI, short* aNumFrames );


/*----------------------------------------------------------------
  Get the volume status of the last recognized utterance.  This indicates whether
  the volume was too low, high or ok.  At the present time it is a crude measure
  based on maximum absolute signal value.
----------------------------------------------------------------*/
   TIESRSI_API
   TIesrSI_Error_t TIesrSI_volume( TIesrSI_t aTIesrSI, short* aVolumeStatus );


/*----------------------------------------------------------------
  Get the confidence measure of the last recognized utterance  This is a measure
  that indicates how confident the measure of the last utterance is.
----------------------------------------------------------------*/
   TIESRSI_API
   TIesrSI_Error_t TIesrSI_confidence( TIesrSI_t aTIesrSI, short* aConfidence );


/*--------------------------------------------------------------------------
   Get detailed status information for the result of the last
   recognition operation.  This status consists of three integers. The
   first specifies detailed recognition engine status, the second the
   detailed status of audio collection, and the last the detailed
   status of environment adaptation.  Normally, this function is not
   called, but may be useful during debugging of an application to
   determine the cause of recognition failures.  This function can be called
   after any other TIesrSI API function except TIesrSI_destroy, to get
   detailed status of the result of the TIesrSI API function call.
   ----------------------------------------------------------------------------*/

    TIESRSI_API
        TIesrSI_Error_t TIesrSI_status( TIesrSI_t aTIesrSI, 
        int* aRecoStatus, int* aAudioStatus, int* aJacStatus );


/*---------------------------------------------------------------------------
  Close the TIesrSI recognizer.  This will deallocate any resources that 
  were created when the recognizer was opened.  The recognizer can then
  be opened again, for example, with a new grammar dependent on the particular
  state of the application.
-----------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_close( TIesrSI_t aTIesrSI );


/*--------------------------------------------------------------------------- 
   Destruct the instance of the recognizer.  This completely removes the 
   TIesrSI object that was created with TIesrSI_create, and frees all resources
   that were allocated internally.  This must be called as the last part of
   using a recognizer instance to ensure that all allocated resources have
   been deallocated and no leakage occurs.
-----------------------------------------------------------------------------*/
    TIESRSI_API
        TIesrSI_Error_t TIesrSI_destroy( TIesrSI_t aTIesrSI );


#ifdef __cplusplus
}
#endif


#endif /* _TIESRSI_USER_H */
