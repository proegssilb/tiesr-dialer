/*=======================================================================
 TIesrFA_WM5.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This file contains an implementation of a TIesrFA API interface
 that uses the Windows Audio interface.

======================================================================*/

/* Windows header */
#include <windows.h>


/* TIesrFA headers */
#include "TIesrFA_User.h"
#include "TIesrFA_WM5.h"

/*-----------------------------------------------------------------
DLL Entry point for TIesrFA
--------------------------------------------------------------*/
BOOL APIENTRY DllMain(HANDLE hModule,
        DWORD ul_reason_for_call,
        LPVOID lpReserved
        ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

/*----------------------------------------------------------------
TIesrFA_init

Implmentation of the TIesrFA_init function.  Initializes the implementation
object.  Allocates memory to hold buffer data and run the data collection.
------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_init(TIesrFA_t * const aTIesrFAInstance,
        const unsigned int sample_rate,
        const TIesrFA_Encoding_t encoding,
        const unsigned int samples_per_frame,
        const unsigned int num_circular_buffer_frames,
        const unsigned int num_audio_buffer_frames,
        const unsigned long audio_read_rate,
        const char* channel_name,
        const int priority) {
    TIesrFAWM5_Error_t wm5Error;
    TIesrFAWM5_t* WM5Data;

    /* The implementation data is initially not defined */
    aTIesrFAInstance->impl_data = NULL;

    /* Should check for valid parameters here */
    aTIesrFAInstance->sample_rate = sample_rate;
    aTIesrFAInstance->encoding = encoding;
    aTIesrFAInstance->samples_per_frame = samples_per_frame;
    aTIesrFAInstance->num_circular_buffer_frames = num_circular_buffer_frames;
    aTIesrFAInstance->num_audio_buffer_frames = num_audio_buffer_frames;
    aTIesrFAInstance->audio_read_rate = audio_read_rate;

#ifdef CYGWIN
    // cygwin uses strdup
    aTIesrFAInstance->channel_name = strdup(channel_name);
#else
    // Windows uses _strdup
    aTIesrFAInstance->channel_name = _strdup( channel_name );
#endif


    aTIesrFAInstance->priority = priority;

    if (aTIesrFAInstance->channel_name == NULL) {
        return TIesrFAErrArg;
    }

    if (aTIesrFAInstance->audio_read_rate == 0 ||
            aTIesrFAInstance->num_circular_buffer_frames < 2 ||
            aTIesrFAInstance->num_audio_buffer_frames < 2 ||
            aTIesrFAInstance->sample_rate != WM5_MIN_SAMPLE_RATE ||
            aTIesrFAInstance->samples_per_frame < WM5_MIN_SAMPLES_PER_FRAME) {
        return TIesrFAErrArg;
    }

    /* Create the implementation specific structure, save it and initialize it */
    WM5Data = (TIesrFAWM5_t*) malloc(sizeof ( TIesrFAWM5_t));
    if (WM5Data == NULL) {
        return TIesrFAErrFail;
    }
    aTIesrFAInstance->impl_data = (void *) WM5Data;


#ifdef WM5_DEBUG_LOG
    WM5Data->threadFile = NULL;

    WM5Data->logFile = fopen(LOGFILE, "w");
    if (WM5Data->logFile == NULL) {
        free(WM5Data);
        aTIesrFAInstance->impl_data = NULL;
        return TIesrFAErrFail;
    }

    fprintf(WM5Data->logFile, "FAinit\n");
    fflush(WM5Data->logFile);
#endif


    /* Initial state and status */
    WM5Data->state = TIesrFAWM5StateUninit;
    WM5Data->status = 0;


    /* File mode input parameter initialization */
    WM5Data->filePointer = NULL;
    WM5Data->frameData = NULL;


    /* WM5 waveform audio input channel handle */
    WM5Data->wiHandle = NULL;

    /* WM5 audio channel read frame buffer, headers, processing queue, and
    synchronization objects */
    WM5Data->readHeaders = NULL;
    WM5Data->readBuffer = NULL;
    WM5Data->readHeadersReturnQueue = NULL;
    WM5Data->readReturnEvent = NULL;
    WM5Data->readCSInitialized = FALSE;

    /* Output frame circular buffer and frame buffer byte counts */
    WM5Data->circBuffer = NULL;
    WM5Data->bytesInFrame = NULL;

    /* Output frame circular buffer synchronization objects */
    WM5Data->circularSemaphore = NULL;
    WM5Data->circCSInitialized = FALSE;

    /* Initialize collection thread and synchronization objects */
    WM5Data->threadHandle = NULL;
    WM5Data->startEvent = NULL;
    WM5Data->stopEvent = NULL;

    /* Wait objects for the thread are stopEvent and readSemaphore */
    WM5Data->waitObjects[0] = NULL;
    WM5Data->waitObjects[1] = NULL;


    /* initialize the read and circular buffers and associated structures */
    wm5Error = TIesrFAWM5_initbuf(aTIesrFAInstance);
    if (wm5Error != TIesrFAWM5ErrNone) {
        free(aTIesrFAInstance->channel_name);
        free(WM5Data);
        aTIesrFAInstance->impl_data = NULL;
        return TIesrFAErrFail;
    }


    /* check if channel_name is a file name, which indicates that
    audio data will be read from a file, rather than collected
    from a live hardware audio channel */
    WM5Data->filePointer = fopen(channel_name, "rb");
    if (WM5Data->filePointer == NULL) {
        WM5Data->fileMode = FALSE;
    } else {
        WM5Data->fileMode = TRUE;
        fclose(WM5Data->filePointer);
        WM5Data->filePointer = NULL;
    }


    /* Initialization completed successfully */
    WM5Data->state = TIesrFAWM5StateInit;

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAInit success\n");
    fflush(WM5Data->logFile);
#endif

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
TIesrFAWM5_initbuf

Initialize the read buffer, the WM5 read buffer frame headers,
the read buffer frame queue, the circular buffer and circular buffer frame byte counters.
------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_initbuf(TIesrFA_t * const aTIesrFAInstance) {
    unsigned int sampleBytes;
    unsigned int bufSize;
    TIesrFAWM5_Error_t wm5Error;

    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;


    /* Determine number of bytes per sample */
    switch (aTIesrFAInstance->encoding) {
        case TIesrFALinear:
            sampleBytes = 2;
            WM5Data->formatTag = WAVE_FORMAT_PCM;
            break;

#ifndef CYGWIN
        case TIesrFAUlaw:
            WM5Data->formatTag = WAVE_FORMAT_MULAW;
            sampleBytes = 1;
            break;

        case TIesrFAAlaw:
            WM5Data->formatTag = WAVE_FORMAT_ALAW;
            sampleBytes = 1;
            break;
#endif

        default:
            return TIesrFAWM5ErrFail;
    }

    /* Size of a sample and size of a frame in bytes */
    WM5Data->sampleSize = sampleBytes;
    WM5Data->frameSize = aTIesrFAInstance->samples_per_frame * sampleBytes;

    /* Size of a waveform audio header read buffer. */
    WM5Data->readBufSize = WM5Data->frameSize * aTIesrFAInstance->audio_read_rate;


    /* Allocate the WM5 wave headers */
    WM5Data->readHeaders = (PWAVEHDR) malloc(aTIesrFAInstance->num_audio_buffer_frames *
            sizeof ( WAVEHDR));
    if (WM5Data->readHeaders == NULL)
        goto FailedInit;

    /* Allocate the read frame buffer which holds frames of input data */
    bufSize = WM5Data->readBufSize * aTIesrFAInstance->num_audio_buffer_frames;
    WM5Data->readBuffer = (unsigned char*) malloc(bufSize * sizeof (unsigned char));
    if (WM5Data->readBuffer == NULL)
        goto FailedInit;

    /* Allocate the read frame queue which tells which read headers have been 
    filled and returned by the WM5 audio device and are ready to have data transferred 
    to the circular buffer. */
    WM5Data->readHeadersReturnQueue =
            (unsigned int*) malloc(aTIesrFAInstance->num_audio_buffer_frames *
            sizeof ( unsigned int));
    if (WM5Data->readHeadersReturnQueue == NULL)
        goto FailedInit;



    /* Allocate the circular frame buffer, which is the buffer that holds frames of
    data that can be transferred to the application. */
    bufSize = WM5Data->frameSize * aTIesrFAInstance->num_circular_buffer_frames;
    WM5Data->circBuffer = (unsigned char*) malloc(bufSize * sizeof (unsigned char));
    if (WM5Data->circBuffer == NULL)
        goto FailedInit;

    /* Allocate array of counters containing number of active bytes in each circular
    buffer frame.  This way an audio channel can return partially full read frame buffers
    and this will be handled correctly. Data will be put in a circular buffer frame 
    until full, then that frame will be released to the application when requested. */
    WM5Data->bytesInFrame =
            (unsigned int *) malloc(aTIesrFAInstance->num_circular_buffer_frames * sizeof (unsigned int));
    if (WM5Data->bytesInFrame == NULL)
        goto FailedInit;

    return TIesrFAWM5ErrNone;


FailedInit:

    wm5Error = TIesrFAWM5_destroybuf(WM5Data);

    return TIesrFAWM5ErrFail;
}

/*----------------------------------------------------------------------
    TIesrFAWM5_destroybuf

    This function deallocates any of the buffers allocated by 
    TIesrFAWM5_initbuf.  This makes it easy to clean up the TIesrFAWM5_t
    structure of allocated memory.
    --------------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_destroybuf(TIesrFAWM5_t * const aWM5Data) {
    if (aWM5Data->readHeaders != NULL) {
        free(aWM5Data->readHeaders);
        aWM5Data->readHeaders = NULL;
    }

    if (aWM5Data->readBuffer != NULL) {
        free(aWM5Data->readBuffer);
        aWM5Data->readBuffer = NULL;
    }

    if (aWM5Data->readHeadersReturnQueue != NULL) {
        free(aWM5Data->readHeadersReturnQueue);
        aWM5Data->readHeadersReturnQueue = NULL;
    }

    if (aWM5Data->circBuffer != NULL) {
        free(aWM5Data->circBuffer);
        aWM5Data->circBuffer = NULL;
    }

    if (aWM5Data->bytesInFrame != NULL) {
        free(aWM5Data->bytesInFrame);
        aWM5Data->bytesInFrame = NULL;
    }

    return TIesrFAWM5ErrNone;
}

/*----------------------------------------------------------------
TIesrFA_open

Implementation of the function that opens a channel and prepares for
reading samples of audio data.
----------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_open(TIesrFA_t * const aTIesrFAInstance) {
    TIesrFAWM5_Error_t wm5Error;

    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    if (WM5Data->state != TIesrFAWM5StateInit) {
        return TIesrFAErrState;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAOpen\n");
    fflush(WM5Data->logFile);
#endif

    /* Initialize the status to no errors when attempting to open */
    WM5Data->status = 0;

    /* If audio data is from a file, then open the file */
    if (WM5Data->fileMode) {
        WM5Data->filePointer = fopen(aTIesrFAInstance->channel_name, "rb");
        if (WM5Data->filePointer == NULL) {
            return TIesrFAErrFail;
        }
        WM5Data->frameData =
                (unsigned char*) malloc(WM5Data->frameSize * sizeof (unsigned char));

        if (WM5Data->frameData == NULL) {
            fclose(WM5Data->filePointer);
            WM5Data->filePointer = NULL;
            WM5Data->status |= TIesrFAWM5StatusMemory;
            return TIesrFAErrFail;
        }
    }        /* If audio data from WM5 audio, then open the channel */
    else {
        /* Open a WM5 audio channel */
        wm5Error = TIesrFAWM5_open(aTIesrFAInstance);
        if (wm5Error != TIesrFAWM5ErrNone) {
            return TIesrFAErrFail;
        }
    }


    /* Opened a channel successfully.  Ready to read audio samples */
    WM5Data->state = TIesrFAWM5StateOpen;

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAOpen success\n");
    fflush(WM5Data->logFile);
#endif

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
TIesrFAWM5_open

Try to open a WM5 audio port for reading audio data.
----------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_open(TIesrFA_t * const aTIesrFAInstance) {
    HWAVEIN wiHandle;
    WAVEFORMATEX wvFormat;
    MMRESULT mmResult;

    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    /* Define the WM5 format structure to find an audio channel */
    wvFormat.wFormatTag = WM5Data->formatTag;
    wvFormat.nChannels = 1;
    wvFormat.nSamplesPerSec = aTIesrFAInstance->sample_rate;
    wvFormat.nBlockAlign = WM5Data->sampleSize;
    wvFormat.nAvgBytesPerSec = aTIesrFAInstance->sample_rate * WM5Data->sampleSize;
    wvFormat.wBitsPerSample = WM5Data->sampleSize * 8;
    wvFormat.cbSize = 0;

    /* Try to open a WM5 channel with these characteristics */
    mmResult = waveInOpen(&wiHandle, WAVE_MAPPER, &wvFormat,
            (DWORD_PTR) TIesrFAWM5_waveinproc, (DWORD_PTR) aTIesrFAInstance,
            CALLBACK_FUNCTION);

    if (mmResult != MMSYSERR_NOERROR) {
        return TIesrFAWM5ErrFail;
    }

    /* WM5 audio device opened successfully */
    WM5Data->wiHandle = wiHandle;

    return TIesrFAWM5ErrNone;
}

/*----------------------------------------------------------------
TIesrFA_start

This function starts data collection on an opened channel.  The
data collection starts in a new high priority thread in order to 
maintain real time processing. After the thread is running, or
fails to start this function will return.
----------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_start(TIesrFA_t * const aTIesrFAInstance) {
    unsigned int frame;
    TIesrFAWM5_Error_t wm5Error;
    MMRESULT mmResult;
    DWORD threadStatus;


    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;


    /* The WM5 channel must be open */
    if (WM5Data->state != TIesrFAWM5StateOpen) {
        return TIesrFAErrState;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart\n");
    fflush(WM5Data->logFile);
#endif

    /* Reset all status except memory errors prior to startup attempt */
    WM5Data->status &= TIesrFAWM5StatusMemory;


    /* If audio data is from a file, just return if file is opened */
    if (WM5Data->fileMode) {
        if (WM5Data->filePointer == NULL) {
            return TIesrFAErrFail;
        } else {
            WM5Data->state = TIesrFAWM5StateRunning;

#ifdef WM5_DEBUG_LOG
            fprintf(WM5Data->logFile, "FAStart success\n");
            fflush(WM5Data->logFile);
#endif

            return TIesrFAErrNone;
        }
    }

#ifdef WM5_DEBUG_LOG
    WM5Data->threadFile = fopen(THREADFILE, "w");
    if (WM5Data->threadFile == NULL)
        return TIesrFAErrFail;
#endif


    /* DEBUGGING */
    WM5Data->totalReadHeadersReturned = 0;
    WM5Data->totalCircFramesTransferred = 0;

    /* Initialize flags indicating no buffer real-time status failures. */
    WM5Data->circularOverflow = FALSE;
    WM5Data->readUnderflow = FALSE;

    /* Initialize number of valid bytes in each circular buffer frame */
    for (frame = 0; frame < aTIesrFAInstance->num_circular_buffer_frames; frame++) {
        WM5Data->bytesInFrame[frame] = 0;
    }

    /* Initialize variables for managing the WM5 read buffer frames and headers. 
    These will be synchronized. */
    WM5Data->readHeadersinReturnQueue = 0;
    WM5Data->readHeadersActive = 0;
    WM5Data->readHeadersInputIndex = 0;
    WM5Data->readHeadersOutputIndex = 0;
    WM5Data->readHeadersinAudioQueue = 0;

    /* Initialize variables for managing the circular frame buffer */
    WM5Data->circularFrameInputIndex = 0;
    WM5Data->circularFrameOutputIndex = 0;
    WM5Data->circularFramesFull = 0;


#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart initing sync\n");
    fflush(WM5Data->logFile);
#endif
    /* Initialize the thread synchronization objects */
    wm5Error = TIesrFAWM5_initsync(WM5Data,
            aTIesrFAInstance->num_audio_buffer_frames,
            aTIesrFAInstance->num_circular_buffer_frames);
    if (wm5Error != TIesrFAWM5ErrNone) {
        return TIesrFAErrFail;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart prep headers\n");
    fflush(WM5Data->logFile);
#endif
    /* Prepare all WM5 wave headers for read buffer frames  */
    wm5Error = TIesrFAWM5_prepallheaders(aTIesrFAInstance);
    if (wm5Error != TIesrFAWM5ErrNone) {
        return TIesrFAErrFail;
    }


#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart add headers\n");
    fflush(WM5Data->logFile);
#endif
    /* Add all wave headers to the WM5 waveform audio driver queue.  It
    now takes control of the read frame buffers.  It will return them 
    when they are full, or when requested by a waveInReset. If this function
    fails, all headers are unprepared. */
    wm5Error = TIesrFAWM5_addallheaders(aTIesrFAInstance);
    if (wm5Error != TIesrFAWM5ErrNone) {
        return TIesrFAErrFail;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart create thread\n");
    fflush(WM5Data->logFile);
#endif

    /* Create a thread that will start the audio device running and handle 
    input in a separate high priority thread. The thread is created in a
    suspended state. */
    WM5Data->threadHandle = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE) TIesrFAWM5_thread,
            (LPVOID) aTIesrFAInstance, CREATE_SUSPENDED, NULL);
    if (WM5Data->threadHandle == NULL) {
        WM5Data->status |= TIesrFAWM5StatusCreateThread;

        /* If the thread failed to be created then request return of
        all wave headers, and clean up.  Since the thread has not
        disturbed the header counters, this can be done synchronously. */
        waveInReset(WM5Data->wiHandle);
        TIesrFAWM5_unprepallheaders(aTIesrFAInstance, TRUE);

        return TIesrFAErrFail;
    }
    SetThreadPriority(WM5Data->threadHandle, aTIesrFAInstance->priority);

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart run thread\n");
    fflush(WM5Data->logFile);
#endif

    /* Start running the thread */
    ResumeThread(WM5Data->threadHandle);


    /* Wait for thread to report WM5 audio device start up, 
    and check if it started ok */
    threadStatus = WaitForSingleObject(WM5Data->startEvent, WM5_THREAD_WAIT);
    if (threadStatus == WAIT_TIMEOUT) {
        WM5Data->status |= TIesrFAWM5StatusStartThread;

        /* The thread never responded that it started. Something is very wrong. 
        This is a drastic step, to terminate the thread itself and free all resources. */
        TerminateThread(WM5Data->threadHandle, TIesrFAWM5ErrFail);

        /* Try to recover from the failed thread start-up */
        CloseHandle(WM5Data->threadHandle);
        WM5Data->threadHandle = NULL;

        /* Request that the WM5 audio device return queued wave headers */
        mmResult = waveInReset(WM5Data->wiHandle);

        /* Unprepare the headers.  We can do this synchronously because
        the WM5 audio driver started ok, and the thread has not disturbed
        the buffer counters. */
        wm5Error = TIesrFAWM5_unprepallheaders(aTIesrFAInstance, TRUE);

        /* Clean up sync objects */
        wm5Error = TIesrFAWM5_destroysync(WM5Data);

        return TIesrFAErrFail;
    } else if (WM5Data->threadError != TIesrFAWM5ErrNone) {
        /* Thread had a failure on startup, indicating that
        the WM5 audio device driver did not start correctly. Wait
        for the thread to complete and remove it. */
        WaitForSingleObject(WM5Data->threadHandle, INFINITE);
        CloseHandle(WM5Data->threadHandle);
        WM5Data->threadHandle = NULL;

        /* Request that the WM5 audio device driver return queued
        wave headers */
        mmResult = waveInReset(WM5Data->wiHandle);

        /* Since the WM5 audio device driver did not start ok, it is
        best to assume that it may have problems returning headers,
        so just unprep them after waiting a while. */
        Sleep(WM5_WAVEINRESET_WAIT);
        wm5Error = TIesrFAWM5_unprepallheaders(aTIesrFAInstance, FALSE);

        /* Clean up sync objects */
        wm5Error = TIesrFAWM5_destroysync(WM5Data);

        return TIesrFAErrFail;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStart success\n");
    fflush(WM5Data->logFile);
#endif

    /* Thread started ok */
    WM5Data->state = TIesrFAWM5StateRunning;
    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
TIesrFAWM5_initsync

Initialize the synchronization objects for the circular buffer.

--------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_initsync(TIesrFAWM5_t * const aWM5Data,
        int aReadFrames, int aCircularFrames) {
    /* Read frame buffer interface synchronization critical section. 
    Synchronizes between audio driver callback and collection thread. */

    try {
        InitializeCriticalSection(&aWM5Data->readCriticalSection);
    } catch (...) {
        goto InitSyncFail;
    }
    aWM5Data->readCSInitialized = TRUE;


    /* Event signaled when WM5 driver returns a read frame wave header. 
    Stays signalled until there are no unprocessed read frames/headers in the 
    read frame return queue. */
    aWM5Data->readReturnEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (aWM5Data->readReturnEvent == NULL)
        goto InitSyncFail;

    /* Event signaled when the thread process has initialized. */
    aWM5Data->startEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (aWM5Data->startEvent == NULL)
        goto InitSyncFail;

    /* Event signaled when the applications wants to stop data collection */
    aWM5Data->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (aWM5Data->stopEvent == NULL)
        goto InitSyncFail;

    /* Critical section to interface with circular frame buffer.  
    Synchronizes between collection thread and user request for a
    frame of data. */
    try {
        InitializeCriticalSection(&aWM5Data->circularCriticalSection);
    } catch (...) {
        goto InitSyncFail;
    }
    aWM5Data->circCSInitialized = TRUE;


    /* Semaphore incremented when a frame of data placed in circular buffer. */
    aWM5Data->circularSemaphore = CreateSemaphore(NULL, 0, aCircularFrames, NULL);
    if (aWM5Data->circularSemaphore == NULL)
        goto InitSyncFail;

    /* Object array that is used for waiting in thread */
    aWM5Data->waitObjects[0] = aWM5Data->stopEvent;
    aWM5Data->waitObjects[1] = aWM5Data->readReturnEvent;

    return TIesrFAWM5ErrNone;

InitSyncFail:
    aWM5Data->status |= TIesrFAWM5StatusSync;

    TIesrFAWM5_destroysync(aWM5Data);

    return TIesrFAWM5ErrFail;
}

/*--------------------------------------------------------------------
TIesrFAWM5_destroysync

This function removes the sync objects when TIesrFAWM5 is done using them.
----------------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_destroysync(TIesrFAWM5_t *aWM5Data) {

    /* Check if critical section created ok before delete */
    if (aWM5Data->readCSInitialized) {
        DeleteCriticalSection(&aWM5Data->readCriticalSection);
        aWM5Data->readCSInitialized = FALSE;
    }

    if (aWM5Data->readReturnEvent != NULL) {
        CloseHandle(aWM5Data->readReturnEvent);
        aWM5Data->readReturnEvent = NULL;
    }

    /* Check if critical section created ok before delete */
    if (aWM5Data->circCSInitialized) {
        DeleteCriticalSection(&aWM5Data->circularCriticalSection);
        aWM5Data->circCSInitialized = FALSE;
    }

    if (aWM5Data->circularSemaphore != NULL) {
        CloseHandle(aWM5Data->circularSemaphore);
        aWM5Data->circularSemaphore = NULL;
    }

    if (aWM5Data->startEvent != NULL) {
        CloseHandle(aWM5Data->startEvent);
        aWM5Data->startEvent = NULL;
    }

    if (aWM5Data->stopEvent != NULL) {
        CloseHandle(aWM5Data->stopEvent);
        aWM5Data->stopEvent = NULL;
    }

    aWM5Data->waitObjects[0] = NULL;
    aWM5Data->waitObjects[1] = NULL;

    return TIesrFAWM5ErrNone;
}

/*-------------------------------------------------------------------
TIesrFAWM5_prepallheaders

This function prepares all of the WM5 waveform audio read buffer frame headers
for all frames for use by the WM5 waveform audio input driver.
----------------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_prepallheaders(TIesrFA_t *aTIesrFAInstance) {
    TIesrFAWM5_Error_t wm5Error;
    unsigned int frame;
    unsigned int unframe;
    PWAVEHDR wHeaders;
    MMRESULT mmResult;

    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    /* Pointer to the array of WAVEHDR headers */
    wHeaders = WM5Data->readHeaders;

    /* Clear all flags, so they can be used to determine which read frame buffer
    headers have been prepared */
    for (frame = 0; frame < aTIesrFAInstance->num_audio_buffer_frames; frame++) {
        wHeaders[frame].dwFlags = 0;
    }

    /* Prepare each header.  The user data for each header is its frame index. */
    for (frame = 0; frame < aTIesrFAInstance->num_audio_buffer_frames; frame++) {
        wm5Error = TIesrFAWM5_prepheader(WM5Data, &wHeaders[frame], frame);

        /* If any prepare fails, unprepare all prepared headers and return error */
        if (wm5Error != TIesrFAWM5ErrNone) {
            for (unframe = 0; unframe < frame; unframe++) {
                mmResult = waveInUnprepareHeader(WM5Data->wiHandle,
                        &wHeaders[unframe], sizeof (WAVEHDR));

                if (mmResult != MMSYSERR_NOERROR)
                    WM5Data->status |= TIesrFAWM5StatusUnprep;
            }
            return TIesrFAWM5ErrFail;
        }
    }

    return TIesrFAWM5ErrNone;
}

/*--------------------------------------------------------------------
TIesrFAWM5_prepheader

This function prepares the WM5 header for the read buffer frame indexed by
aFrame for use by the WM5 audio input driver.
------------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_prepheader(TIesrFAWM5_t *aWM5Data,
        PWAVEHDR aHeader,
        unsigned int aFrame) {
    MMRESULT mmResult;
    unsigned char* bufferLocation;

    bufferLocation = aWM5Data->readBuffer +
            aFrame * aWM5Data->readBufSize;

    /* Prepare the header for the present frame buffer.  dwUser holds the 
    frame index number associated with the frame buffer of this header. */
    aHeader->lpData = (LPSTR) bufferLocation;
    aHeader->dwBufferLength = aWM5Data->readBufSize;
    aHeader->dwUser = aFrame;
    aHeader->dwFlags = 0;

    mmResult = waveInPrepareHeader(aWM5Data->wiHandle, aHeader, sizeof (WAVEHDR));
    if (mmResult != MMSYSERR_NOERROR) {
        aWM5Data->status |= TIesrFAWM5StatusPrep;

        return TIesrFAWM5ErrFail;
    }
    return TIesrFAWM5ErrNone;
}

/*---------------------------------------------------------------
TIesrFAWM5_addallheaders

This function adds all of the read frame buffers as specified 
by their WAVEHDR objects to the WM5 waveform audio device driver queue.  
If any of them can not be added, then all headers are unprepared
and failure is returned.  This happens prior to starting of the 
audio collection thread.  Note that this function must be called after 
waveInOpen opens the WM5 audio device driver, but before the driver
is started by waveInStart.
-----------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_addallheaders(TIesrFA_t* aTIesrFAInstance) {
    PWAVEHDR wHeaders;
    bool headersQueued;
    unsigned int frame;
    MMRESULT mmResult;
    TIesrFAWM5_Error_t wm5Error;

    TIesrFAWM5_t* WM5Data = (TIesrFAWM5_t*) aTIesrFAInstance->impl_data;

    /* Track error */
    wm5Error = TIesrFAWM5ErrNone;

    wHeaders = WM5Data->readHeaders;

    for (frame = 0; frame < aTIesrFAInstance->num_audio_buffer_frames; frame++) {
        /* Add a buffer to the WM5 audio driver which has not started yet. */
        mmResult = waveInAddBuffer(WM5Data->wiHandle, &wHeaders[frame], sizeof (WAVEHDR));

        if (mmResult == MMSYSERR_NOERROR) {
            WM5Data->readHeadersActive++;
            WM5Data->readHeadersinAudioQueue++;
        } else {
            /* waveInReset should return buffers even if audio driver
            not started yet? */
            WM5Data->status |= TIesrFAWM5StatusAddHdr;
            headersQueued = (WM5Data->readHeadersinAudioQueue > 0);
            waveInReset(WM5Data->wiHandle);
            wm5Error = TIesrFAWM5_unprepallheaders(aTIesrFAInstance, headersQueued);

            return (wm5Error == TIesrFAWM5ErrNone) ? TIesrFAWM5ErrFail : wm5Error;
        }
    }

    return TIesrFAWM5ErrNone;
}

/*-------------------------------------------------------------------
TIesrFAWM5_unprepallheaders

This function unprepares all of the read buffer frame WAVEHDR headers.
It is assumed when this function is called that all headers have been
or are in the process of being released by the WM5 waveform audio driver,
for example by calling waveInReset.  This function is looping, and 
continues to loop until no headers remain that are not unprepared.
So, it is critical that no header remain in the driver queue.  The 
optional boolean aQueuedFlag specifies that at least some wave headers
have been queued to the audio buffer.  If this is the case, then this 
function loops until all of the headers have been returned prior to 
unpreparing the headers. 
---------------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_unprepallheaders(TIesrFA_t *aTIesrFAInstance,
        BOOL aQueuedFlag) {
    unsigned int frame;
    PWAVEHDR wHeaders;
    TIesrFAWM5_Error_t wm5Error;
    MMRESULT mmResult;

    TIesrFAWM5_t *WM5Data = (TIesrFAWM5_t *) aTIesrFAInstance->impl_data;

    wHeaders = WM5Data->readHeaders;

    /* Track error to return */
    wm5Error = TIesrFAWM5ErrNone;

    /* If headers are queued, loop until they have all been returned.
    TIesr has kept a counter in readHeadersinAudioQueue of the read
    frames/headers that have been sent to the WM5 audio queue but not
    processed from a return yet.
    Include a method to exit the loop to just give up if it appears 
    that the WM5 audio driver never releases the headers.
    This would be some form of catastrophic failure. */
    if (aQueuedFlag) {
        while (WM5Data->readHeadersinAudioQueue > 0) {
            mmResult = WaitForSingleObject(WM5Data->readReturnEvent, WM5_THREAD_WAIT);

            /* The WM5 Driver did not respond by sending back a remaining 
            queued frame/header */
            if (mmResult == WAIT_TIMEOUT) {
                WM5Data->status |= TIesrFAWM5StatusDequeue;
                break;
            }

            /* Count the read frames/headers that are being returned by
            consuming them immediately. */
            EnterCriticalSection(&WM5Data->readCriticalSection);

            WM5Data->readHeadersinReturnQueue--;
            if (WM5Data->readHeadersinReturnQueue == 0)
                ResetEvent(WM5Data->readReturnEvent);

            LeaveCriticalSection(&WM5Data->readCriticalSection);

            WM5Data->readHeadersinAudioQueue--;
        }
    }


    /* Try to unprepare all of the headers */
    for (frame = 0; frame < aTIesrFAInstance->num_audio_buffer_frames; frame++) {
        /* Check if header was returned from WM5 driver queue. That is,
        the driver is done with the header.  This should normally be the
        case. */
        if (wHeaders[frame].dwFlags & WHDR_DONE) {
            /* Unprepare the wave header */
            mmResult = waveInUnprepareHeader(WM5Data->wiHandle,
                    &wHeaders[frame], sizeof (WAVEHDR));

            if (mmResult != MMSYSERR_NOERROR) {
                WM5Data->status |= TIesrFAWM5StatusUnprep;
                wm5Error = TIesrFAWM5ErrFail;
            }
        }            /* This is some form of critical failure, since the audio driver
            did not release the wave header. */
        else {
            WM5Data->status |= TIesrFAWM5StatusInQueue;
            wm5Error = TIesrFAWM5ErrFail;
        }
    }

    return wm5Error;
}

/*----------------------------------------------------------------
TIesrFA_getframe

The user is requesting a frame of data from the audio channel.
Check if any frames of data are available.  If blocking is
requested, and no frame data is available, block for next frame.
Otherwise if blocking is not requested and no frame data is
available, just return error information that a frame of data is not
available yet.
----------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_getframe(TIesrFA_t * const aTIesrFAInstance,
        unsigned char* const aFrame,
        const int aBlock,
        unsigned int *aFramesQueued) {
    DWORD waitResult;
    unsigned char* circFrame;
    int bufferOverflow;
    unsigned int numBytesRead;

    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;


    /* The channel must be reading audio */
    if (WM5Data->state != TIesrFAWM5StateRunning) {
        return TIesrFAErrState;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAGetFrame\n");
    fflush(WM5Data->logFile);
#endif

    /* If audio is coming from the file, just read the samples.
    If at the end of the file, just continue to output the last frame */
    if (WM5Data->fileMode) {
        *aFramesQueued = 0;
        if (!feof(WM5Data->filePointer)) {
            numBytesRead = fread(aFrame, sizeof (unsigned char),
                    WM5Data->frameSize,
                    WM5Data->filePointer);

            if (numBytesRead == WM5Data->frameSize) {
                /* Save copy of the latest full frame */
                memcpy(WM5Data->frameData, aFrame, WM5Data->frameSize);
            } else {
                /* Output the copy of the last full frame */
                memcpy(aFrame, WM5Data->frameData, WM5Data->frameSize);
                return (TIesrFAErrEOF);

            }
        } else {
            /* Output a copy of the last full frame */
            memcpy(aFrame, WM5Data->frameData, WM5Data->frameSize);
            return (TIesrFAErrEOF);
        }

        return TIesrFAErrNone;
    }



    /* Not reading from a file, so reading from live WM5 audio channel.
    Check to see if a frame of data is available */
    if (aBlock) {
        /* Block waiting for a frame */
        waitResult =
                WaitForSingleObject(WM5Data->circularSemaphore, WM5_THREAD_FRAME_WAIT);
        if (waitResult == WAIT_TIMEOUT) {
            WM5Data->status |= TIesrFAWM5StatusWaitThread;
            return TIesrFAErrThread;
        }
    } else {
        /* Check for frame available right now, if not notify user */
        waitResult = WaitForSingleObject(WM5Data->circularSemaphore, 0);
        if (waitResult == WAIT_TIMEOUT)
            return TIesrFAErrNoFrame;
    }

    /* A frame is available, so transfer it to the user's memory */
    circFrame = WM5Data->circBuffer +
            WM5Data->circularFrameOutputIndex * WM5Data->frameSize;

    memcpy(aFrame, circFrame, WM5Data->frameSize);

    /* Update the circular buffer samples and pointer to indicate we
    are finished with the frame.  This is mutex protected, to
    protect the circular buffer frame access by both threads */
    EnterCriticalSection(&WM5Data->circularCriticalSection);

    WM5Data->bytesInFrame[ WM5Data->circularFrameOutputIndex ] = 0;
    WM5Data->circularFramesFull--;
    *aFramesQueued = WM5Data->circularFramesFull;

    bufferOverflow = WM5Data->circularOverflow || WM5Data->readUnderflow;


#ifdef MYDEBUG
    printf("Get: circ frame %d, circ full: %d\n",
            WM5Data->circularFrameOutputIndex, WM5Data->circularFramesFull);
#endif

    LeaveCriticalSection(&WM5Data->circularCriticalSection);

    /* Go to the next output circular buffer frame, which the read
    thread will fill or has already filled. */
    WM5Data->circularFrameOutputIndex++;
    if (WM5Data->circularFrameOutputIndex >= aTIesrFAInstance->num_circular_buffer_frames) {
        WM5Data->circularFrameOutputIndex = 0;
    }
#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAGetframe end %d\n", bufferOverflow);
    fflush(WM5Data->logFile);
#endif

    /* Successfully transferred a frame */
    if (bufferOverflow)
        return TIesrFAErrOverflow;
    else
        return TIesrFAErrNone;
}

/*----------------------------------------------------------------
TIesrFAWM5_thread

This function is the thread function that reads audio sample data
from the WM5 device and puts the data into the circular buffers.
--------------------------------*/
static
DWORD TIesrFAWM5_thread(LPVOID aArg) {
    TIesrFAWM5_Error_t wm5Error;
    MMRESULT mmResult;
    DWORD waitObject;

    TIesrFA_t * const aTIesrFAInstance = (TIesrFA_t * const) aArg;

    /* WM5 specific data */
    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->threadFile, "FAThread\n");
    fflush(WM5Data->threadFile);
#endif

    /* Initially flag failure to start the audio collection */
    WM5Data->threadError = TIesrFAWM5ErrFail;

    /* Try to start the WM5 waveform audio device driver */
    mmResult = waveInStart(WM5Data->wiHandle);
    if (mmResult != MMSYSERR_NOERROR) {
        /* On failure signal the start function that thread failed to start
        the WM5 audio device driver, then terminate the thread by exiting. */
        SetEvent(WM5Data->startEvent);
        return TIesrFAWM5ErrFail;
    }

    /* Signal the start function that audio is now being collected. */
    WM5Data->threadError = TIesrFAWM5ErrNone;
    SetEvent(WM5Data->startEvent);

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->threadFile, "FAThread loop start\n");
    fflush(WM5Data->threadFile);
#endif

    /* Loop reading sampled data */
    while (TRUE) {
        /* Wait either for the user to request that data collection be stopped or
        for a read frame to become available. */
        waitObject = WaitForMultipleObjects(2, WM5Data->waitObjects, FALSE, INFINITE);

        /* Check to see if the user signaled to finish data collection.
        If so, then just exit the thread. */
        if (waitObject == WAIT_OBJECT_0) {
            break;
        }            /* Check to see if the read return event indicates a WM5 wave header
            is filled and available. */
        else if (waitObject == WAIT_OBJECT_0 + 1) {

            /* If the read return event is signaled, then fill frames in the
            circular frame buffer from the read frame buffer in the read frame
            queue.  This function will also synchronously reset the read 
            return event if no more read frames are in the read frame buffer */
            wm5Error = TIesrFAWM5_fillframes(aTIesrFAInstance);
        }            /* Some other condition occurred in the wait.  This is some kind of
            failure. */
        else {
            // TODO: Handle wait failure.
        }
    }

    /* The flag has been set to stop processing WM5 audio data, so we
    just terminate the thread */

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->threadFile, "FAThread end\n");
    fflush(WM5Data->threadFile);
#endif

    return TIesrFAWM5ErrNone;
}

/*----------------------------------------------------------------
TIesrFAWM5_waveinproc

This is the WM5 waveform audio driver callback procedure that is called
each time the driver releases a wave header.  This function queues the header
onto the read return queue for processing by the thread, which will 
copy the data into the circular buffer if space permits.  This function
also checks to ensure that at least one wave header is still queued.  If
not, then data is lost.
---------------------------------------------------------------------*/
static
void CALLBACK TIesrFAWM5_waveinproc(HWAVEIN aWIHandle, UINT aMsg,
        DWORD aInstance, DWORD aParam1,
        DWORD aParam2) {
    unsigned int frame;
    PWAVEHDR pWaveHeader;

    TIesrFA_t * const aTIesrFAInstance = (TIesrFA_t * const) aInstance;

    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    /* Only process data callback messages */
    if (aMsg != WIM_DATA)
        return;

    /* A read frame wave header has been returned.  Find which 
    header frame index has been returned. */
    pWaveHeader = (PWAVEHDR) aParam1;
    frame = (unsigned int) pWaveHeader->dwUser;

    /* Updating of the read return queue and counters must be synchronized. */
    EnterCriticalSection(&WM5Data->readCriticalSection);

    /* Put the wave header frame index in the read frame return queue. */
    WM5Data->readHeadersReturnQueue[ WM5Data->readHeadersInputIndex ] = frame;
    WM5Data->readHeadersinReturnQueue++;

    /* Signal that another read frame is ready for processing. */
    SetEvent(WM5Data->readReturnEvent);

    /* DEBUGGING */
    WM5Data->totalReadHeadersReturned++;

    /* Check to see if the WM5 audio driver has returned all read
    return frames/headers, and is therefore starved for queued 
    wave headers.  Normally readHeadersActive should remain a constant
    during processing, but it can decrease if a waveInAddHeader fails
    during data collection.  readHeadersActive is not synchronized since 
    waveInAddHeader can not be synchronized with this callback function,
    so the underflow may not be set in the very rare condition that
    a waveInAddHeader fails and the readHeadersActive changes exactly 
    when this callback function is at this code. */
    if (WM5Data->readHeadersinReturnQueue >=
            WM5Data->readHeadersActive) {
        WM5Data->readUnderflow = TRUE;
        WM5Data->status |= TIesrFAWM5StatusUnderflow;
    }

#ifdef MYDEBUG
    printf("Drv: hdr %d put in read queue at %d,  %d hdrs in queue\n",
            frame, WM5Data->readHeadersInputIndex, WM5Data->readHeadersinReturnQueue);
#endif

    LeaveCriticalSection(&WM5Data->readCriticalSection);


    /* Update the read frame return queue input index */
    WM5Data->readHeadersInputIndex++;
    if (WM5Data->readHeadersInputIndex >= aTIesrFAInstance->num_audio_buffer_frames) {
        WM5Data->readHeadersInputIndex = 0;
    }

    return;
}

/*----------------------------------------------------------------
TIesrFAWM5_fillframes

This function is called when a read frame wave header data returned by the
WM5 audio driver is available.  This function is responsible for 
transferring the data to one or more circular frame buffers, and for
requeuing the wave header back to the WM5 audio driver queue. The
function puts the data into one or more circular buffer frames, and
if circular buffer frames are filled, then it updates the circular
buffer frame pointer and semaphore accordingly.
-------------------------------------------------------------------------*/
static
TIesrFAWM5_Error_t TIesrFAWM5_fillframes(TIesrFA_t * const aTIesrFAInstance) {
    unsigned int frame;
    PWAVEHDR wHeaders;
    unsigned int framesFull;
    unsigned char* readFrameData;
    unsigned int readBytesRemaining;
    unsigned int circBytesRemaining;
    unsigned int readFrameOffset;
    unsigned int circFrameOffset;
    unsigned char* circFrameData;
    MMRESULT mmResult;



    /* WM5 specific data */
    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    /* Simulate circular buffer filling not able to keep up with real-time */
    //Sleep(50);

    /* Determine next available wave header frame on the read frame return queue. */
    wHeaders = WM5Data->readHeaders;
    frame = WM5Data->readHeadersReturnQueue[ WM5Data->readHeadersOutputIndex ];


    /* Initialize number of bytes in the read frame and frame location */
    readBytesRemaining = wHeaders[frame].dwBytesRecorded;
    readFrameData = WM5Data->readBuffer + frame * WM5Data->readBufSize;
    readFrameOffset = 0;

    /* printf( "BytesRead %d\n", aBytesRead ); */

    while (readBytesRemaining > 0) {
        /* Determine if all circular frames are full.  If so, there is an overflow
        condition, so do not put data from read frame into circular frame buffer.
        Synchronize the fetch of number of circular frame buffers full. */
        EnterCriticalSection(&WM5Data->circularCriticalSection);
        framesFull = WM5Data->circularFramesFull;
        LeaveCriticalSection(&WM5Data->circularCriticalSection);

        /* printf( "Frames full: %d\n", framesFull ); */

        if (framesFull == aTIesrFAInstance->num_circular_buffer_frames) {
            /* Log that a buffer overflow occurred, and exit loop without
            using the data from the read frame buffer since there is no
            place to put it.  Real-time has been lost. */

            WM5Data->status |= TIesrFAWM5StatusOverflow;
            WM5Data->circularOverflow = TRUE;
            break;
        } else {
            /* The circular buffer frame is not full, so put data into it */
            circFrameData = WM5Data->circBuffer +
                    WM5Data->circularFrameInputIndex * WM5Data->frameSize;

            /* This should be ok unprotected by a mutex, since if the frame is not full, 
            the main thread will not be clearing the bytesInFrame variable for this frame */
            circBytesRemaining = WM5Data->frameSize -
                    WM5Data->bytesInFrame[ WM5Data->circularFrameInputIndex ];

            circFrameOffset = WM5Data->frameSize - circBytesRemaining;

            /* Check if there are enough read buffer bytes to completely fill the present
            input circular frame buffer */
            if (readBytesRemaining >= circBytesRemaining) {
                /* completely fill the circular frame */
                memcpy(circFrameData + circFrameOffset, readFrameData + readFrameOffset,
                        circBytesRemaining);

                WM5Data->bytesInFrame[ WM5Data->circularFrameInputIndex ] +=
                        circBytesRemaining;

                /* Adjust location and amount of next data to use read frame buffer */
                readBytesRemaining -= circBytesRemaining;
                readFrameOffset += circBytesRemaining;


                /* Indicate to the main thread that a new frame is available. Protected 
                by synchronization. */
                EnterCriticalSection(&WM5Data->circularCriticalSection);

                WM5Data->circularFramesFull++;

                /* DEBUGGING */
                WM5Data->totalCircFramesTransferred++;

                /* Another circular frame is available for processing indicated
                by the semaphore */
                ReleaseSemaphore(WM5Data->circularSemaphore, 1, NULL);

                LeaveCriticalSection(&WM5Data->circularCriticalSection);


                /* Go to the next circular frame buffer, 
                since the present one is now full */
                WM5Data->circularFrameInputIndex++;
                if (WM5Data->circularFrameInputIndex >=
                        aTIesrFAInstance->num_circular_buffer_frames) {
                    WM5Data->circularFrameInputIndex = 0;
                }
            }
                /* There are not enough read frame buffer bytes to fill the present input
                circular frame buffer.  Put all remaining read frame buffer bytes into
                the circular frame buffer, but it is not full yet so do not need to update
                any inter-thread  status. This will exit the while loop, since no more
                readFrameData bytes remain to be processed. */
            else {
                memcpy(circFrameData + circFrameOffset,
                        readFrameData + readFrameOffset, readBytesRemaining);

                WM5Data->bytesInFrame[ WM5Data->circularFrameInputIndex ] +=
                        readBytesRemaining;

                /* printf( "Add to %d with %d\n", WM5Data->input_buffer_frame, bufBytesRemaining ); */

                readBytesRemaining = 0;
            }
        }
    }

    /* Done with this wave header and its read frame buffer, so TIesrFA
    knows that one read frame buffer has been consumed.  This is a local
    count of frames sent to the audio queue but not yet processed. */
    WM5Data->readHeadersinAudioQueue--;

    /*Update read return queue output index. */
    WM5Data->readHeadersOutputIndex++;
    if (WM5Data->readHeadersOutputIndex >= aTIesrFAInstance->num_audio_buffer_frames)
        WM5Data->readHeadersOutputIndex = 0;

    /* Gain lock on the WM5 audio driver CS so that the count of read frames
    in the queue is synchronized. */
    EnterCriticalSection(&WM5Data->readCriticalSection);

    /*  Check if the waveform audio driver has returned all active headers.  
    If this is the case, then the driver is starved for headers, 
    and real-time is lost. */
    if (WM5Data->readHeadersinReturnQueue == WM5Data->readHeadersActive) {
        WM5Data->readUnderflow = TRUE;
        WM5Data->status |= TIesrFAWM5StatusUnderflow;
    }

    /* Finished with this read frame buffer.  This count is synchronized. */
    WM5Data->readHeadersinReturnQueue--;

    /* If there are no more read frames in the queue, then reset the event, so that
    the thread must wait for the WM5 audio driver to signal when more read headers
    become available. Otherwise, leave the event signaled, since there are more
    read frames in the read frame return queue to process.  It is required that
    this event be synchronized.*/
    if (WM5Data->readHeadersinReturnQueue == 0)
        ResetEvent(WM5Data->readReturnEvent);

#ifdef MYDEBUG
    printf("Fill: hdr %d consumed, %d hdrs in read queue\n",
            frame, WM5Data->readHeadersinReturnQueue);
#endif

    LeaveCriticalSection(&WM5Data->readCriticalSection);


    /* Re-queue the wave header just used back to the WM5 audio device queue.
    This call cannot be made synchronized with the readCriticalSection or
    TIesrFA will hang since the WM5 audio driver callback function may be
    waiting for the CS, and at the same time waveInAddBuffer can not
    complete since the driver is blocked. */
    mmResult = waveInAddBuffer(WM5Data->wiHandle, &wHeaders[frame], sizeof (WAVEHDR));

    if (mmResult == MMSYSERR_NOERROR) {
        /* TIesrFA successfully put back another wave header in the 
        WM5 audio driver queue */
        WM5Data->readHeadersinAudioQueue++;
    } else {
        /* Just report error in status.  If this continues to happen,
        eventually the thread will become non-responsive and the user
        will detect this as a failure to get a frame of data. */
        WM5Data->status |= TIesrFAWM5StatusReAddHdr;
        WM5Data->readHeadersActive--;

        /* If only one or less read frame/headers remains active, 
        then surely real-time will be lost. This would be a fairly
        catostrophic failure, and is not likely to happen. */
        if (WM5Data->readHeadersActive <= 1) {
            WM5Data->readUnderflow = TRUE;
            WM5Data->status |= TIesrFAWM5StatusUnderflow;
        }

    }

#ifdef MYDEBUG
    printf("Fill: finished hdr %d, hdrs to process %d\n",
            frame, WM5Data->readHeadersinAudioQueue);
#endif

    /* Done successfully filling the circular buffer from the read buffer */
    return TIesrFAWM5ErrNone;
}

/*----------------------------------------------------------------
TIesrFA_stop

This function call stops data collection.  This does not close the
audio channel, which is still available for doing another recording
by calling TIesrFA_start. This function does stop the audio data
collection thread.
----------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_stop(TIesrFA_t * const aTIesrFAInstance) {
    TIesrFAWM5_Error_t wm5Error;
    MMRESULT mmResult;
    DWORD threadResult;
    TIesrFA_Error_t faError;


    /* WM5 specific data */
    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    if (WM5Data->state != TIesrFAWM5StateRunning) {
        return TIesrFAErrState;
    }
#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStop\n");
    fflush(WM5Data->logFile);
#endif

    /* If audio is coming from a file, just rewind it */
    if (WM5Data->fileMode) {
        fseek(WM5Data->filePointer, 0, SEEK_SET);
        clearerr(WM5Data->filePointer);

        WM5Data->state = TIesrFAWM5StateOpen;

#ifdef WM5_DEBUG_LOG
        fprintf(WM5Data->logFile, "FAStop success\n");
        fflush(WM5Data->logFile);
#endif
        return TIesrFAErrNone;
    }

    /* Track output error to return */
    faError = TIesrFAErrNone;

    /* Set the stop event to stop reading data and shut down thread */
    /* Should this be synchronized? */
    SetEvent(WM5Data->stopEvent);

    /* Wait for thread to complete */
    threadResult = WaitForSingleObject(WM5Data->threadHandle, WM5_THREAD_WAIT);
    if (threadResult == WAIT_TIMEOUT) {
        /* The thread is not responding, set error status and terminate it. */
        WM5Data->status |= TIesrFAWM5StatusEndThread;
        TerminateThread(WM5Data->threadHandle, TIesrFAWM5ErrThreadTimeout);
        faError = TIesrFAErrThread;
    }

    CloseHandle(WM5Data->threadHandle);
    WM5Data->threadHandle = NULL;

#ifdef WM5_DEBUG_LOG
    if (WM5Data->threadFile) {
        fclose(WM5Data->threadFile);
        WM5Data->threadFile = NULL;
    }

    fprintf(WM5Data->logFile, "FAStop closed thread\n");
    fflush(WM5Data->logFile);
#endif
    /* printf( "Thread ended\n"); o*/

    /* Stop the WM5 audio driver audio collection, and unprepare headers. */
    mmResult = waveInReset(WM5Data->wiHandle);

    /* If the thread failed to exit, then some catastrophic failure occurred
    in the thread, and the thread has been terminated.  Header counters
    may not be good.  Simply wait for the waveInReset to return headers
    and then try to unprep them without synchronization. */
    if (faError == TIesrFAErrThread) {
        Sleep(WM5_WAVEINRESET_WAIT);
        wm5Error = TIesrFAWM5_unprepallheaders(aTIesrFAInstance, FALSE);
    } else {
        wm5Error = TIesrFAWM5_unprepallheaders(aTIesrFAInstance, TRUE);
    }

    if (wm5Error != TIesrFAWM5ErrNone) {
        if (faError == TIesrFAErrNone)
            faError = TIesrFAErrFail;
    }

    /* Reset sync objects */
    wm5Error = TIesrFAWM5_destroysync(WM5Data);
    if (wm5Error != TIesrFAWM5ErrNone) {
        if (faError == TIesrFAErrNone)
            faError = TIesrFAErrFail;
    }

    WM5Data->state = TIesrFAWM5StateOpen;

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAStop end %d\n", faError);
    fflush(WM5Data->logFile);
#endif
    return faError;
}

/*----------------------------------------------------------------
TIesrFA_close

Close an audio channel.  Just closes the audio channel, but does not
deallocate the TIesrFA resources.  The channel can be reopened.
----------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_close(TIesrFA_t * const aTIesrFAInstance) {

    MMRESULT mmResult;

    /* WM5 specific data */
    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    if (WM5Data->state != TIesrFAWM5StateOpen) {
        return TIesrFAErrState;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAClose\n");
    fflush(WM5Data->logFile);
#endif

    /* If data is coming from a file, close the file, and free frame data memory */
    if (WM5Data->fileMode) {
        fclose(WM5Data->filePointer);
        WM5Data->filePointer = NULL;

        free(WM5Data->frameData);
        WM5Data->frameData = NULL;

        WM5Data->state = TIesrFAWM5StateInit;

#ifdef WM5_DEBUG_LOG
        fprintf(WM5Data->logFile, "FAClose success\n");
        fflush(WM5Data->logFile);
#endif

        return TIesrFAErrNone;
    }


    /* Close the WM5 audio port, which stops recording and flushes data */
    mmResult = waveInClose(WM5Data->wiHandle);
    if (mmResult != MMSYSERR_NOERROR) {
        return TIesrFAErrFail;
    }

    WM5Data->wiHandle = NULL;
    WM5Data->state = TIesrFAWM5StateInit;

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FAClose success\n");
    fflush(WM5Data->logFile);
#endif

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
TIesrFA_destroy

This function is supplied to free all memory allocated to an
instance of the audio channel.  It does not destroy the instance
object itself.
--------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_destroy(TIesrFA_t * const aTIesrFAInstance) {
    TIesrFAWM5_Error_t wm5Error;
    TIesrFA_Error_t faError;

    /* WM5 specific data */
    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;


    /* Check for already in an uninitialized state */
    if (WM5Data == NULL) {
        return TIesrFAErrNone;
    }


    /* Check if the WM5 channel is not closed */
    if (WM5Data->state != TIesrFAWM5StateInit) {
        return TIesrFAErrState;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FADestroy\n");
    fflush(WM5Data->logFile);
#endif

    /* Track return error */
    faError = TIesrFAErrNone;


    /* Close file if open.  Should not happen */
    if (WM5Data->fileMode) {
        if (WM5Data->filePointer != NULL) {
            fclose(WM5Data->filePointer);
            WM5Data->filePointer = NULL;
        }

        if (WM5Data->frameData != NULL) {
            free(WM5Data->frameData);
            WM5Data->frameData = NULL;
        }
    }


    /* Destroy syncrhonization objects.  They should already be destroyed by a 
    TIesrFA_stop, or failure to start correctly, but this will make sure. */
    wm5Error = TIesrFAWM5_destroysync(WM5Data);
    if (wm5Error != TIesrFAWM5ErrNone) {
        if (faError == TIesrFAErrNone)
            faError = TIesrFAErrFail;
    }

    /* Free all buffer space */
    wm5Error = TIesrFAWM5_destroybuf(WM5Data);
    if (wm5Error != TIesrFAWM5ErrNone) {
        if (faError == TIesrFAErrNone)
            faError = TIesrFAErrFail;
    }

#ifdef WM5_DEBUG_LOG
    fprintf(WM5Data->logFile, "FADestroy end\n");
    fflush(WM5Data->logFile);

    if (WM5Data->logFile)
        fclose(WM5Data->logFile);

    if (WM5Data->threadFile)
        fclose(WM5Data->threadFile);
#endif

    /* Free the WM5 impl_data object itself.  This removes any WM5
    implementation state information */
    free(WM5Data);
    aTIesrFAInstance->impl_data = NULL;


    /* Free channel (or file) name */
    if (aTIesrFAInstance->channel_name != NULL) {
        free(aTIesrFAInstance->channel_name);
        aTIesrFAInstance->channel_name = NULL;
    }

    return faError;
}

/*---------------------------------------------------------
TIesrFA_status

This function returns a status unsigned int that can be used
during debugging of TIesrFA.  It provides status of failures
during processing.
----------------------------------------------------------*/
TIESRFA_API
unsigned int TIesrFA_status(TIesrFA_t * const aTIesrFAInstance) {
    /* WM5 specific data */
    TIesrFAWM5_t * const WM5Data = (TIesrFAWM5_t * const) aTIesrFAInstance->impl_data;

    if (WM5Data == NULL) {
        return TIesrFAWM5StatusMemory;
    } else {
        return WM5Data->status;
    }
}
