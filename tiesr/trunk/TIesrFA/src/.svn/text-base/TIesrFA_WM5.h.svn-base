/*=======================================================================
 TIesrFA_WM5.h

 *
 * TIesrFA_WM5.h
 *
 * Header for implementation of TIesrFA on Windows.
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

 This header file defines the objects that are internal to this
 particular implmentation of the TIesrFA API, and do not need to be
 accessed by the user.

 This implmementation of the API is for use on Windows Mobile 5.0
 systems, and uses the Windows Waveform Audio API. The API also works
 on other Windows platforms.

======================================================================*/

#ifndef _TIESRFA_WM5_H
#define _TIESRFA_WM5_H

/* These are the WM5 headers required for threading and WM5 waveform audio API */
#include <mmsystem.h>
#include <Mmreg.h>

/* Used C libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Enable this flag to output a debugging log file.
   Normally this flag should not be defined. */
// #define WM5_DEBUG_LOG

#ifdef WM5_DEBUG_LOG
#define LOGFILE "\\Application Data\\TIesr\\FAlog.txt"
#define THREADFILE "\\Application Data\\TIesr\\FAthread.txt"
#endif

/* Constants that are part of the API. */

/* Initialization check parameters */
#define WM5_MIN_SAMPLE_RATE 8000

#define WM5_MIN_SAMPLES_PER_FRAME 40


/* The priority of the thread that transfers data from the WM5 audio driver
to circular buffer frames. */
#define WM5_THREAD_PRIORITY   THREAD_PRIORITY_ABOVE_NORMAL

/* Time to wait on the thread to respond to an action, either starting,
or stopping. For debugging it is better to use an infinite wait.*/
//#define WM5_THREAD_WAIT  10000
#define WM5_THREAD_WAIT  INFINITE

/* Time to wait on the thread to return a frame.  This can indicate to the
application that the thread has for some reason become non-responsive. 
For debugging it is better to use an infinite wait. */
//#define WM5_THREAD_FRAME_WAIT  10000
#define WM5_THREAD_FRAME_WAIT  INFINITE


/* Time to wait for waveInReset to return headers.  Only used in 
catastrophic conditions where the thread fails. */
#define WM5_WAVEINRESET_WAIT 5000


/* State of the instance of WM5 data collection */
typedef enum TIesrFAWM5_States
{
    TIesrFAWM5StateUninit,
    TIesrFAWM5StateInit,
    TIesrFAWM5StateOpen,
    TIesrFAWM5StateRunning
} TIesrFAWM5_State_t;


/* Errors that can be returned */
typedef enum TIesrFAWM5_Errors
{
    TIesrFAWM5ErrNone,
    TIesrFAWM5ErrFail,
    TIesrFAWM5ErrNoMemory,
    TIesrFAWM5ErrOverflow,
    TIesrFAWM5ErrThreadTimeout,
    TIesrFAWM5ErrThreadStart
} TIesrFAWM5_Error_t;


enum TIesrFAWM5_Status
{
    TIesrFAWM5StatusMemory = 0x1,
    TIesrFAWM5StatusPrep = 0x2,
    TIesrFAWM5StatusAddHdr = 0x4,
    TIesrFAWM5StatusCreateThread = 0x8,
    TIesrFAWM5StatusStartThread = 0x10,
    TIesrFAWM5StatusUnprep = 0x020,
    TIesrFAWM5StatusWaitThread = 0x40,
    TIesrFAWM5StatusOverflow = 0x80,
    TIesrFAWM5StatusUnderflow = 0x100,
    TIesrFAWM5StatusReAddHdr = 0x200,
    TIesrFAWM5StatusEndThread = 0x400,
    TIesrFAWM5StatusSync = 0x800,
    TIesrFAWM5StatusDequeue = 0x1000,
    TIesrFAWM5StatusInQueue = 0x2000
};


/* TIesrFAWM5_t defines the implementation specific implementation data used */

typedef struct TIesrFAWM5_Struct
{
    /* State of the channel */
    TIesrFAWM5_State_t state;

    /* Boolean flag indicating if audio channel is a file,
    and file pointer */
    bool fileMode;
    FILE *filePointer;
    unsigned char* frameData;

    /* WM5 Waveform format tag type */
    WORD formatTag;   

    /* Size of a sample and a circular frame buffer in bytes */
    unsigned int sampleSize;
    unsigned int frameSize;

    /* Size of a wave header read buffer in bytes */
    unsigned int readBufSize;
   
 
    /* WM5 read buffer and headers.*/
    unsigned char *readBuffer;
    PWAVEHDR readHeaders;
    
    /* Variables to manage read header/buffer processing */

    /* Number of read headers that are being actively used between
    TIesrFA and the WM5 audio driver. This number should remain constant 
    unless a waveInAddHeader fails. */
    unsigned int readHeadersActive;

    /* Number of read headers in WM5 audio driver queue.  This
    is a counter local to TIesrFA that maintains a count of read headers 
    that TIesrFA knows it has transferred to the WM5 audio driver, but which
    TIesrFA has not yet processed by having them returned by the WM5 audio
    driver. */
    unsigned int readHeadersinAudioQueue;

    /* Circular queue of returned read header indices specifying 
    read headers that have been returned by the WM5 audio driver
    that need to be processed. This queue is synchronized. */
    unsigned int *readHeadersReturnQueue;

    /* Number of read headers returned by WM5 Audio device to the read headers
    return queue and not yet processed.  This count is synchronized between
    TIesrFA and the WM5 audio driver via the driver callback using a critical
    section. */
    unsigned int readHeadersinReturnQueue;
 
    /* Present read header return queue index for input used by the WM5
    audio driver callback function to access the read return queue. */
    unsigned int readHeadersInputIndex;
    
    /* Present read header return queue index for output to circular buffer used
    by TIesrFA to access the read return queue. */
    unsigned int readHeadersOutputIndex;
    
    /* Synchronization object for management of read frame return queue. */
    CRITICAL_SECTION readCriticalSection;
    bool readCSInitialized;

    /* readReturnEvent when signalled indicates that there is at least
    one read header in the read header return queue that has not
    yet been processed.  If it is not signalled, then the read header return
    queue has no entries that need to be processed by TIesrFA. This event is
    synchronized by the critcal section. */ 
    HANDLE readReturnEvent;


    /* Frame output circular buffer */
    unsigned char *circBuffer;

    /* circular frame buffers full, not yet transferred to user. */
    unsigned int circularFramesFull;

    /* Array of number of valid bytes loaded from read frame buffers to each 
    circular buffer frame.  This determines when a circular buffer frame is full
    and hence available for output. */
    unsigned int *bytesInFrame;

    /* Variables to manage the circular frame buffer */
    
    /* Present circular frame index for input of data returned from the 
    WM5 audio driver in a read frame. */
    unsigned int circularFrameInputIndex;
    
    /* Present circular frame index for output to user. */
    unsigned int circularFrameOutputIndex;
 
    /* Synchronization object for managing circular buffer. */
    CRITICAL_SECTION circularCriticalSection;
    bool circCSInitialized;
   
    /* This semaphore contains a count of circular frames available to
    the user.  This semaphore is used to determine when frames are
    available to the user. If no frames are available, then the user may
    choose to block waiting for a frame, or continue processing and try
    again later. */
    HANDLE circularSemaphore;
   

    /* Synchronization objects for starting and stopping data collection thread */
    HANDLE startEvent;
    HANDLE stopEvent;

    /* Array of handles for waiting in the thread.  The thread waits for
    either a read frame wave header to be returned, or for the user to 
    signal that data collection should terminate. The two wait objects are
    therefore stopEvent and readReturnEvent */
    HANDLE waitObjects[2];


    /* WM5 Waveform Input Audio Device handle */
    HWAVEIN wiHandle;

    /* Data collection thread handle */
    HANDLE threadHandle;
    TIesrFAWM5_Error_t threadError;
    
    /* Flag indicating a circular buffer overflow occurred.  That is, 
    there is data available in a read frame buffer but no place to
    put it since the circular frame buffers are all full,
    therefore the read frame buffer data is just discarded. */
    BOOL circularOverflow;
    
    /* Flag indicating that there was a read frame wave header underflow. 
    That is, the WM5 audio device has returned the last wave header queued and
    now has no queued buffers. Note that this naturally happens when the
    wave headers are flushed at the end of data collection by a waveInReset.
    However, if this happens during data collection it indicates that TIesrFA
    could not keep up with the WM5 audio driver returned frames and so real-time
    has been lost. */
    BOOL readUnderflow;

    /* The status flag of this instance. */
    unsigned int status;


    /* Debugging parameters */
    unsigned int totalReadHeadersReturned;
    unsigned int totalCircFramesTransferred;

#ifdef WM5_DEBUG_LOG
    FILE* logFile;
    FILE* threadFile;
#endif

} TIesrFAWM5_t;



/* static functions - hidden from user */

static TIesrFAWM5_Error_t TIesrFAWM5_addallheaders( TIesrFA_t* aTIesrFAInstance );

static TIesrFAWM5_Error_t TIesrFAWM5_destroybuf( TIesrFAWM5_t * const aWM5Data );

static TIesrFAWM5_Error_t TIesrFAWM5_destroysync( TIesrFAWM5_t *aWM5Data );

static TIesrFAWM5_Error_t TIesrFAWM5_initbuf( TIesrFA_t* const aTIesrFAInstance );

static TIesrFAWM5_Error_t TIesrFAWM5_fillframes( TIesrFA_t* const aTIesrFAInstance );

static TIesrFAWM5_Error_t TIesrFAWM5_initbuf( TIesrFA_t* const aTIesrFAInstance );

static TIesrFAWM5_Error_t TIesrFAWM5_initsync( TIesrFAWM5_t* const aWM5Data, 
                                       int aReadFrames, int aCircularFrames );

static TIesrFAWM5_Error_t TIesrFAWM5_open( TIesrFA_t* const aTIesrFAInstance );

static TIesrFAWM5_Error_t TIesrFAWM5_prepallheaders( TIesrFA_t *aTIesrFAInstance );

static TIesrFAWM5_Error_t TIesrFAWM5_prepheader( TIesrFAWM5_t *aWM5Data, 
                                                PWAVEHDR aHeader, 
                                                unsigned int aFrame );

static TIesrFAWM5_Error_t TIesrFAWM5_unprepallheaders( TIesrFA_t *aTIesrFAInstance,
                                               BOOL aQueuedFlag );

/* The thread function for loading frames of audio */
static DWORD TIesrFAWM5_thread( LPVOID aArg );

/* The waveform audio callback function */
static void CALLBACK TIesrFAWM5_waveinproc( HWAVEIN aWIHandle, UINT aMsg,
                                    DWORD aInstance, DWORD aParam1,
                                    DWORD aParam2 );

#endif
