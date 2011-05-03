/*=======================================================================
 
 *
 * TIesrFA_ALSA.h
 *
 * Header for implementation of TIesrFA for Linux using ALSA audio.
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

 This header is designed to be used with TIesrFA_ALSA.c, which implements
 the TIesrFA API for Linux platforms supporting the ALSA library API.

======================================================================*/

#ifndef _TIesrFA_ALSA_H
#define	_TIesrFA_ALSA_H

/* Needed for thread operation? */
#define _REENTRANT

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

/* The ALSA library API */
#include <alsa/asoundlib.h>

/* Constants that are part of the API */
#define THREAD_PRIORITY 20

#define FALSE 0
#define TRUE 1


#ifdef	__cplusplus
extern "C"
{
#endif

    /* State of the instance of FILE data collection */
    typedef enum TIesrFA_ALSA_States
    {
        TIesrFA_ALSAStateUninit,
        TIesrFA_ALSAStateInit,
        TIesrFA_ALSAStateOpen,
        TIesrFA_ALSAStateRunning
    } TIesrFA_ALSA_State_t;

    /* Errors that can be returned */
    typedef enum TIesrFA_ALSA_Errors
    {
        TIesrFA_ALSAErrNone,
        TIesrFA_ALSAErrFail,
        TIesrFA_ALSAErrNoMemory,
        TIesrFA_ALSAErrOverflow
    } TIesrFA_ALSA_Error_t;

    /* TIesrFA_ALSA_t defines the implementation specific implementation data used */

    typedef struct TIesrFA_ALSA_Struct
    {
        /* State of the channel */
        TIesrFA_ALSA_State_t state;

        /* File-mode control.Boolean flag indicating if audio channel is a file,
         file pointer, and buffer pointer to hold file data when read */
        int file_mode;
        FILE *file_pointer;
        unsigned char* frame_data;


        /* The ALSA PCM channel */
        snd_pcm_t *alsa_handle;


        /* TIesr frame circular buffer and parameters for managing the TIesr frame
         circular buffer. */
        unsigned char *circ_buffer;
        unsigned int *frame_bytes;
        unsigned int input_buffer_frame;
        unsigned int output_buffer_frame;
        unsigned int sample_size;
        unsigned int frame_size;


        /* Input read buffer, for reading samples from ALSA */
        unsigned char *read_buffer;
        unsigned int  read_samples;


        /* circular buffer and thread synchronization */
        unsigned int buffer_frames_full;
        pthread_t thread_id;
        sem_t start_semaphore;
        pthread_mutex_t buffer_mutex;
        sem_t buffer_semaphore;
        pthread_mutex_t read_mutex;
        int read_data;
        int buffer_overflow;

    } TIesrFA_ALSA_t;



    /* static functions - hidden from user */
    static TIesrFA_ALSA_Error_t TIesrFA_ALSA_initbuf( TIesrFA_t * const aTIesrFAInstance );

    static TIesrFA_ALSA_Error_t TIesrFA_ALSA_initsync( TIesrFA_ALSA_t * const aFILEData );

    static TIesrFA_ALSA_Error_t TIesrFA_ALSA_open( TIesrFA_t * const aTIesrFAIntance );

    static TIesrFA_ALSA_Error_t TIesrFA_ALSA_fillframes( TIesrFA_t * const aTIesrInstance, int aSamplesRead );

    static TIesrFA_ALSA_Error_t TIesrFA_ALSA_resetsync( TIesrFA_ALSA_t * const aFILEData );

    /* The thread function for loading frames of audio */
    static void* TIesrFA_ALSA_thread( void* aArg );


#ifdef	__cplusplus
}
#endif

#endif	/* _TIesrFA_ALSA_H */
