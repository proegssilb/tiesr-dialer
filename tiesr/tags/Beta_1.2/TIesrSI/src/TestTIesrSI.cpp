/*=======================================================================
 TestTIestSI.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 This file implements a utility that runs TIesrSI recognition
 from the command line.  It prompts for an utterance, recognizes it,
 and outputs the recognized phrase.  All output is to stdout.
 The command line is:

 TestTIesrSI Mem GramDir JacFile HLRFile device [Filename]

 Mem = Working membory size in 16-bit elements
 GramDir = Grammar and model directory where models created by TIesrFlex exist
 JACFile = JAC file location where dynamic noise/channel adaptation data placed
 HLRFile = HLR phonetic tree file location
 device =  ALSA audio PCM device to open. May also be a file name containing
           raw audio as input to be recognized.
 [Filename] = Optional file name containing output recorded audio data.

======================================================================*/

/* C run-time usage */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Include the app macros, constants and structures
#include "TestTIesrSI.h"

/*--------------------------------*/
int main(int argc, char** argv) {

    /* recognition object and error */
    Reco_t reco;
    RecoError_t recoError;

    // The recognition semaphores for TIesrSI Callbacks
#if defined (LINUX )
    sem_t start_semaphore;
    sem_t done_semaphore;
#elif defined ( WIN32 )
#endif

    // The TIesrSI recognizer instance variable.  It will hold a
    // TIesrSI recognizer instance.
    TIesrSI_t tiesrSI;
    TIesrSI_Error_t siError;


    /* speech data input file list variables */

    /* Initialization of reco structure with command line arguments */
    if (argc != 6 && argc != 7 ) {
        printf("Invalid argument count\n");
        return 1;
    }

    reco.memorySize = atoi(argv[1]);
    reco.grammarDir = argv[2];
    reco.jacFile = argv[3];
    reco.sbcFile = argv[4];
    reco.audioDevice = argv[5];
    
    if( argc == 7 )
        reco.captureFile = argv[6];
    else
        reco.captureFile = NULL;



    /* Initialize app recognizer state */
    reco.state = IDLE;
    reco.tiesrIsOpen = FALSE;


#if defined ( LINUX )
    /* Initialize the recognition TIesr callback semaphores */
    reco.start_semaphore = &start_semaphore;
    sem_init(reco.start_semaphore, 0, 0);

    reco.done_semaphore = &done_semaphore;
    sem_init(reco.done_semaphore, 0, 0);

#elif defined ( WIN32 )
    reco.start_event = CreateEvent(NULL, FALSE, FALSE, NULL );
    reco.done_event = CreateEvent( NULL, FALSE, FALSE, NULL );
#endif


    /* Create a TIesrSI recognizer instance */
    printf("Creating TIesrSI recognizer\n");
    siError = TIesrSI_create(&tiesrSI);
    if (siError != TIesrSIErrNone) {
        printf("TIesrSI create error %d\n", siError);
        return 1;
    }
    reco.tiesrSI = tiesrSI;


    /* Open the recognizer instance */
    printf("Opening TIesrSI recognizer\n");
    recoError = OpenTIesr(&reco);
    if (recoError != RecoErrNone) {
        printf("Could not open TIesr\n");
        TIesrSI_destroy(tiesrSI);
        exit(recoError);
    }


    /* Start TIesr recognition.  TIesrSI will start recognition in a
    new thread. */
    printf("Starting TIesrSI recognizer\n");
    recoError = StartTIesr(&reco);
    if (recoError != RecoErrNone) {
        printf("Could not start TIesr\n");
        CloseTIesr(&reco);
        TIesrSI_destroy(tiesrSI);
        exit(recoError);
    }

    /* Wait for recognizer to be ready to receive speech.  The TIesrSI
     recognizer will call the SpeakCallback function to signal this
     semaphore when ready. Normally you would do other processing during this
     time and the callback would signal an event when the recognizer is ready. */
#if defined ( LINUX )
    sem_wait(reco.start_semaphore);
#elif defined ( WIN32 )
    WaitForSingleObject(reco.start_event, INFINITE );
#endif

    if (reco.speakResult != TIesrSIErrNone) {
        printf("Recognizer not ready - spoke too early?\n");
        StopTIesr(&reco);
        CloseTIesr(&reco);
        TIesrSI_destroy(tiesrSI);
        exit(reco.speakResult);
    }

    /* Prompt user for speech */
    printf("Speak now...\n");


    /* Wait for recognition to complete.  TIesrSI will call DoneCallback to
     set the semaphore when recognition is complete.  Normally you would do
     processing at this time, and the callback would signal an event
     when complete. */
#if defined ( LINUX )
    sem_wait(reco.done_semaphore);
#elif defined ( WIN32 )
    WaitForSingleObject(reco.done_event, INFINITE );
#endif



    /* Stop the recognizer (this also formalizes recognition results
    // within TIesrSI ) */
    printf("Stopping TIesrSI recognizer\n");
    recoError = StopTIesr(&reco);
    if (recoError != RecoErrNone) {
        printf("Recognizer stop failure\n");
        CloseTIesr(&reco);
        TIesrSI_destroy(tiesrSI);
        exit(recoError);
    }


    // Check recognition result, and if a recognition result is available
    // output the result.
    if (reco.doneResult != TIesrSIErrNone) {
        printf("Recognition failed with error %d\n", reco.doneResult);

        int recoStatus, audioStatus, jacStatus;
        TIesrSI_status( reco.tiesrSI, &recoStatus, &audioStatus, &jacStatus );
        printf( "Reco: %d  Audio: %d JAC: %d\n", recoStatus, audioStatus, jacStatus );

    } else {
        OutputResult(reco.tiesrSI);
    }

    /* Close the recognizer on this file */
    printf("Closing TIesrSI recognizer\n");
    recoError = CloseTIesr(&reco);
    if (recoError != RecoErrNone) {
        printf("Failed to close TIesr recognizer\n");
        exit(recoError);
    }

    /* Destroy the recognizer instance */
    printf("Destroying TIesrSI recognizer\n");
    siError = TIesrSI_destroy(reco.tiesrSI);

    /* destroy the semaphore */
#if defined ( LINUX )
    sem_destroy(reco.start_semaphore);
    sem_destroy(reco.done_semaphore);
#elif defined ( WIN32 )
    CloseHandle(reco.start_event);
    CloseHandle(reco.done_event);
#endif

    if (siError != TIesrSIErrNone)
          return siError;

    return TIesrSIErrNone;
}

/*----------------------------------------------------------------*/
void OutputResult(TIesrSI_t aTIesrSI) {
    TIesrSI_Error_t siError;
    unsigned short numWords;
    unsigned short wd;
    char const *word;
    short confidence;

    /* Number of words in best answer */
    siError = TIesrSI_wdcount(aTIesrSI, &numWords, 0);

    if (numWords == 0) {
        printf("No recognition result - no words recognized\n");
        return;
    }

    /* Output all words of recognized answer except _SIL */
    printf("Recognized: ");
    for (wd = 0; wd < numWords; wd++) {
        siError = TIesrSI_word(aTIesrSI, wd, &word, 0);
        if (siError != TIesrSIErrNone)
            printf("*** ");
        else if (strcmp(word, "_SIL"))
            printf("%s ", word);
    }
    printf("\n");

    /* Get confidence measure - only valid for "large" grammars now */
    /*
    siError = TIesrSI_confidence( aTIesrSI, &confidence );
    if( siError != TIesrSIErrNone )
        printf( "Could not get confidence measure\n");
    else
        printf( "Confidence: %d\n", confidence );
        */
}

/*----------------------------------------------------------------
  SpeakCallback
 
  This is the callback function sent to TIesrSI that it will call
  when it is ready for speech. This is a simple function that just
  stores the status returned in the callback.
  ---------------------------------------------------------------*/
void SpeakCallback(void* aArg, TIesrSI_Error_t aError) {
    Reco_t* reco = (Reco_t*) aArg;

    // Just store the status of the callback
    reco->speakResult = aError;
#if defined ( LINUX )
    sem_post(reco->start_semaphore);
#elif defined ( WIN32 )
    SetEvent(reco->start_event);
#endif
}

/*----------------------------------------------------------------
 DoneCallback

 This is the callback function that TIesrSI will call when it has
 completed recognition, either because of an error condition, or
 because the recognizer has completed.
 -----------------------------------------------------------------*/
void DoneCallback(void* aArg, TIesrSI_Error_t aError) {
    Reco_t* reco = (Reco_t*) aArg;

    // Save recognition status of the callback
    reco->doneResult = aError;

    // Notify the main thread that the recognition has completed
#if defined ( LINUX )
    sem_post(reco->done_semaphore);
#elif defined ( WIN32 )
    SetEvent(reco->done_event);
#endif
}

/*----------------------------------------------------------------
 OpenTIesr

 This app function is a thin wrapper over the TIesrSI open function
 call that opens the TIesrSI recognizer to recognize a given file
 with a created grammar network and model set.  It also sets any
 custom recognizer parameters that may be tested.
 ------------------------------------------------------------------*/
RecoError_t OpenTIesr(Reco_t* reco) {
    TIesrSI_Error_t siError;
    TIesrSI_Params_t siParams;
    int recoStatus;
    int audioStatus;
    int jacStatus;

    if (reco->tiesrIsOpen) {
        return RecoErrOpen;
    }


    // Open TIesrSI connected to an input speech data file
    siError = TIesrSI_open(reco->tiesrSI,
            reco->audioDevice,
            reco->grammarDir,
            reco->jacFile,
            reco->sbcFile,
            reco->memorySize,
            SpeakCallback,
            DoneCallback,
            (void*) reco);

    if (siError != TIesrSIErrNone) {
        printf("Failed to open TIesr\n");
        siError = TIesrSI_status(reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus);
        printf("Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus);
        return RecoErrFail;
    }
    reco->tiesrIsOpen = TRUE;

    // Update the parameters to those desired.  Parameters will be specified
    // by the STL lab
    siError = TIesrSI_getparams(reco->tiesrSI, &siParams);
    if (siError != TIesrSIErrNone) {
        printf("Failed to get TIesr parameters\n");
        siError = TIesrSI_status(reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus);
        printf("Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus);
    }

    siParams.circularFrames = CIRCULAR_FRAMES;
    siParams.audioReadRate = AUDIO_READ_RATE;
    siParams.audioFrames = AUDIO_FRAMES;
    siParams.jacRate = JAC_RATE;
    siParams.pruneFactor = PRUNE_FACTOR;
    siParams.transiWeight = TRANSI_WEIGHT;
    siParams.sadEndFrames = SAD_END_FRAMES;
    siParams.audioPriority = AUDIO_PRIORITY;

    // Testing only
    //siParams.pruneFactor = -5;
    //siParams.jacRate = 80;
    //siParams.transiWeight = 0;

    //siParams.ormLowNoiseLLRThreshold = 200;
    //siParams.ormHiNoiseLLRThreshold = 100;
    //siParams.cnfAdaptThreshold = -20;
    //siParams.realTimeFrameThreshold = 100;
    //siParams.bNBest = 0;

    siError = TIesrSI_setparams(reco->tiesrSI, &siParams);
    if (siError != TIesrSIErrNone) {
        printf("Failed to set TIesr parameters\n");
        siError = TIesrSI_status(reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus);
        printf("Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus);
    }

    return RecoErrNone;
}

/*----------------------------------------------------------------
  StartTIesr
 
  This function is a thin wrapper over the TIesr start function call
  to start the TIesrSI recognizer in a new thread. It just ensures
  that the recognizer state is maintained.
  -----------------------------------------------------------------*/
RecoError_t StartTIesr(Reco_t * reco) {
    TIesrSI_Error_t siError;
    int recoStatus;
    int audioStatus;
    int jacStatus;

    if (!reco->tiesrIsOpen) {
        return RecoErrClosed;
    }


    if (reco->state == RECOGNIZING) {
        return RecoErrRecognizing;
    }


    // Start the recognizer. Successful completion of
    // this function call means a new TIesrSI thread is running.
    siError = TIesrSI_start(reco->tiesrSI, 0, reco->captureFile );
    if (siError == TIesrSIErrNone) {
        reco->state = RECOGNIZING;
        return RecoErrNone;
    } else {
        printf("Failed to start TIesr\n");
        siError = TIesrSI_status(reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus);
        printf("Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus);
        return RecoErrFail;
    }
}

/*----------------------------------------------------------------
 StopTIesr

 This is a thin wrapper app function over the TIesrSI stop function
 call.  Stopping the recognizer must be done if the recognizer
 started successfully.  This is true even if the recognizer notifies
 the app of a recognition error via the callback status.  This function
 also maintains the app recognizer state.
 ------------------------------------------------------------------*/
RecoError_t StopTIesr(Reco_t * reco) {
    TIesrSI_Error_t siError;
    int recoStatus;
    int audioStatus;
    int jacStatus;

    if (!reco->tiesrIsOpen) {
        return RecoErrClosed;
    }


    if (reco->state == IDLE) {
        return RecoErrIdle;
    }


    siError = TIesrSI_stop(reco->tiesrSI);
    if (siError == TIesrSIErrNone) {
        reco->state = IDLE;
        return RecoErrNone;
    } else {
        printf("Failed to stop TIesr\n");
        siError = TIesrSI_status(reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus);
        printf("Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus);
        return RecoErrFail;
    }
}

/*----------------------------------------------------------------
 CloseTIesr

 This function is a thin wrapper over the TIesrSI close function
 call.  It maintains the recognizer state.
 ----------------------------------------------------------------*/
RecoError_t CloseTIesr(Reco_t * reco) {
    TIesrSI_Error_t siError;
    int recoStatus;
    int audioStatus;
    int jacStatus;

    if (!reco->tiesrIsOpen) {
        return RecoErrClosed;
    }

    if (reco->state == RECOGNIZING) {
        return RecoErrRecognizing;
    }

    siError = TIesrSI_close(reco->tiesrSI);
    if (siError == TIesrSIErrNone) {
        reco->tiesrIsOpen = FALSE;
        return RecoErrNone;
    } else {
        printf("Failed to close TIesr\n");
        siError = TIesrSI_status(reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus);
        printf("Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus);
        return RecoErrFail;
    }
}




