/*=======================================================================
 TIesrFA_ALSA.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This file contains an implementation of a TIesrFA API interface
 that is for use on a Linux OS platform with ALSA support.

======================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <time.h>
#include <sched.h>
#include <limits.h>

#include "TIesrFA_User.h"

#include "TIesrFA_ALSA.h"

/*----------------------------------------------------------------
 TIesrFA_init

 Implmentation of the TIesrFA_init function.  Initializes the implementation
 object.
  --------------------------------*/
TIesrFA_Error_t TIesrFA_init( TIesrFA_t * const aTIesrFAInstance,
        const unsigned int sample_rate,
        const TIesrFA_Encoding_t encoding,
        const unsigned int samples_per_frame,
        const unsigned int num_circular_buffer_frames,
        const unsigned int num_audio_buffer_frames,
        const unsigned long audio_read_rate,
        const char* channel_name,
        const int priority )
{
    int error;
    TIesrFA_ALSA_t* ALSAData;

    /* The implementation data is initially not defined */
    aTIesrFAInstance->impl_data = NULL;

    /* Should check for valid parameters here */
    aTIesrFAInstance->sample_rate = sample_rate;

    if( encoding != TIesrFALinear )
        return TIesrFAErrFail;
    aTIesrFAInstance->encoding = encoding;

    aTIesrFAInstance->samples_per_frame = samples_per_frame;

    aTIesrFAInstance->num_circular_buffer_frames = num_circular_buffer_frames;

    /* Number of audio interrupt durations in ALSA ring buffer.  This is an
     indirect way of sizing the ALSA ring buffer.
     */
    aTIesrFAInstance->num_audio_buffer_frames = num_audio_buffer_frames;

    /* Used to calculate ALSA interrupt duration.  This is converted into
     the number of samples that should be collected prior to the ALSA PCM
     interrupting TIesrFA to report data available. */
    aTIesrFAInstance->audio_read_rate = audio_read_rate;

    /* For ALSA, this is the PCM name to open */
    aTIesrFAInstance->channel_name = strdup( channel_name );

    aTIesrFAInstance->priority = priority;

    if( aTIesrFAInstance->channel_name == NULL )
    {
        return TIesrFAErrFail;
    }

    /* Create the implementation specific structure, save it and initialize it */
    ALSAData = (TIesrFA_ALSA_t*) malloc( sizeof( TIesrFA_ALSA_t ) );
    if( ALSAData == NULL )
    {
        return TIesrFAErrFail;
    }
    aTIesrFAInstance->impl_data = (void *) ALSAData;

    ALSAData->state = TIesrFA_ALSAStateUninit;


    /* File mode input parameter initialization */
    ALSAData->file_pointer = NULL;
    ALSAData->frame_data = NULL;


    /* Buffers and data processing initialization */
    ALSAData->circ_buffer = NULL;
    ALSAData->frame_bytes = NULL;
    ALSAData->read_buffer = NULL;



    /* initialize the circular and read buffers */
    error = TIesrFA_ALSA_initbuf( aTIesrFAInstance );
    if( error != TIesrFA_ALSAErrNone )
    {
        free( aTIesrFAInstance->channel_name );
        free( ALSAData );
        aTIesrFAInstance->impl_data = NULL;
        return TIesrFAErrFail;
    }


    /* check if channel_name is a file name, which indicates that
       audio data will be read from a file, rather than collected
       from a live hardware audio channel */
    ALSAData->file_pointer = fopen( channel_name, "rb" );
    if( ALSAData->file_pointer == NULL )
    {
        ALSAData->file_mode = FALSE;
    }
    else
    {
        ALSAData->file_mode = TRUE;
        fclose( ALSAData->file_pointer );
        ALSAData->file_pointer = NULL;
    }


    /* Initialization completed successfully */
    ALSAData->state = TIesrFA_ALSAStateInit;
    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_ALSA_initbuf

 Initialize the circular buffer frames and input buffer, allocating space to
 hold the information.
 --------------------------------*/
TIesrFA_ALSA_Error_t TIesrFA_ALSA_initbuf( TIesrFA_t * const aTIesrFAInstance )
{
    unsigned int sampleBytes;
    unsigned int bufSize;

    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;


    /* Determine number of bytes per sample */
    switch( aTIesrFAInstance->encoding )
    {
        case TIesrFALinear:
            sampleBytes = 2;
            break;

        case TIesrFAUlaw:
        case TIesrFAAlaw:
            sampleBytes = 1;
            break;

        default:
            return TIesrFA_ALSAErrFail;
    }


    /* Allocate the circular buffer of TIesr frames.  The frame size in TIesr
     is not the same as a "frame" in ALSA. TIesr frames are made up of samples
     of mono audio data. Hence an ALSA "frame" is a TIesr "sample". */
    ALSAData->sample_size = sampleBytes;
    ALSAData->frame_size = aTIesrFAInstance->samples_per_frame * sampleBytes;

    bufSize = ALSAData->frame_size * aTIesrFAInstance->num_circular_buffer_frames;
    ALSAData->circ_buffer = (unsigned char*) malloc( bufSize * sizeof(unsigned char) );
    if( ALSAData->circ_buffer == NULL )
    {
        return TIesrFA_ALSAErrFail;
    }

    /* An array to hold how many bytes have been loaded into a circular buffer frame.
     This allows for an audio interface that returns a non-constant number of bytes */
    ALSAData->frame_bytes =
            (unsigned int *) malloc( aTIesrFAInstance->num_circular_buffer_frames * sizeof(unsigned int) );
    if( ALSAData->frame_bytes == NULL )
    {
        free( ALSAData->circ_buffer );
        ALSAData->circ_buffer = NULL;
        return TIesrFA_ALSAErrFail;
    }

    /* The buffer to hold samples read from ALSA, a power of 2*/
    bufSize = aTIesrFAInstance->sample_rate * aTIesrFAInstance->audio_read_rate / 1000000;
    ALSAData->read_samples = bufSize;
    /*
        ALSAData->read_samples = 1;
        while( bufSize > ALSAData->read_samples )
        {
            ALSAData->read_samples <<= 1;
        }
     */
    ALSAData->read_buffer = (unsigned char*) malloc( ALSAData->read_samples * sampleBytes );
    if( ALSAData->read_buffer == NULL )
    {
        free( ALSAData->circ_buffer );
        ALSAData->circ_buffer = NULL;

        free( ALSAData->frame_bytes );
        ALSAData->frame_bytes = NULL;
        return TIesrFA_ALSAErrFail;
    }


    return TIesrFA_ALSAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_open

 Implementation of the function that opens a channel and prepares for
 reading samples of audio data.  Normally this will open the audio
 channel.  However, if the channel name is a file name, input will
 come from a file.
 ----------------------------------------------------------------*/
TIesrFA_Error_t TIesrFA_open( TIesrFA_t * const aTIesrFAInstance )
{
    TIesrFA_ALSA_Error_t error;
    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;

    if( ALSAData->state != TIesrFA_ALSAStateInit )
    {
        return TIesrFAErrState;
    }

    /* If audio data is from a file, then open the file */
    if( ALSAData->file_mode )
    {
        ALSAData->file_pointer = fopen( aTIesrFAInstance->channel_name, "rb" );
        if( ALSAData->file_pointer == NULL )
        {
            return TIesrFAErrFail;
        }

        /* Make space to hold the frame of data from the file */
        ALSAData->frame_data =
                (unsigned char*) malloc( ALSAData->frame_size * sizeof(unsigned char) );

        if( ALSAData->frame_data == NULL )
        {
            fclose( ALSAData->file_pointer );
            return TIesrFAErrFail;
        }
    }

        /* If audio data from an audio channel, then open the channel */
    else
    {
        /* Open an actual ALSA PCM channel. */
        error = TIesrFA_ALSA_open( aTIesrFAInstance );
        if( error != TIesrFA_ALSAErrNone )
        {
            return TIesrFAErrFail;
        }
    }


    /* Opened the audio source successfully.  Ready to read audio samples */
    ALSAData->state = TIesrFA_ALSAStateOpen;

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_ALSA_open

 Try to open an ALSA audio PCM channel.
  ----------------------------------------------------------------*/
TIesrFA_ALSA_Error_t TIesrFA_ALSA_open( TIesrFA_t * const aTIesrFAInstance )
{
    int rtnStatus;
    int openSuccess = 0;
    snd_pcm_uframes_t numSamples;
    snd_pcm_hw_params_t *hw_params = 0;
    snd_pcm_sw_params_t *sw_params = 0;


    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;

    /* Try to open a handle to the ALSA pcm in blocking mode. */
    rtnStatus = snd_pcm_open( &ALSAData->alsa_handle,
            aTIesrFAInstance->channel_name,
            SND_PCM_STREAM_CAPTURE,
            0 );
    if( rtnStatus < 0 )
        goto openExit;

    /* Set up the information for recording from the audio channelport.  This
     will include data specified by the user, such as data in
     aTIesrFAInstance->encoding, etc.   If setup fails, then the audio channel
     should be shut down and failure reported. */

    /* Setup hardware parameters of the ALSA pcm */
    rtnStatus = snd_pcm_hw_params_malloc( &hw_params );
    if( rtnStatus < 0 )
        goto openExit;

    rtnStatus = snd_pcm_hw_params_any( ALSAData->alsa_handle, hw_params );
    if( rtnStatus < 0 )
        goto openExit;

    /* There will only be a single channel */
    rtnStatus = snd_pcm_hw_params_set_channels( ALSAData->alsa_handle,
            hw_params, 1 );
    if( rtnStatus < 0 )
        goto openExit;

    /* Even though only one channel, must specify type of read. */
    rtnStatus = snd_pcm_hw_params_set_access( ALSAData->alsa_handle,
            hw_params,
            SND_PCM_ACCESS_RW_INTERLEAVED );
    if( rtnStatus < 0 )
        goto openExit;


    /* Format type - only 16 bit linear for now */
    rtnStatus = snd_pcm_hw_params_set_format( ALSAData->alsa_handle,
            hw_params,
            SND_PCM_FORMAT_S16 );
    if( rtnStatus < 0 )
        goto openExit;

    /* Set sample rate, it must be exactly supported */
    rtnStatus = snd_pcm_hw_params_set_rate( ALSAData->alsa_handle,
            hw_params,
            aTIesrFAInstance->sample_rate, 0 );
    if( rtnStatus < 0 )
        goto openExit;


    /* Size of ALSA PCM ring buffer is set to approximately the desired 
     number of multiples of read_samples in the buffer */
    numSamples = aTIesrFAInstance->num_audio_buffer_frames *
            ALSAData->read_samples;
    rtnStatus = snd_pcm_hw_params_set_buffer_size_near( ALSAData->alsa_handle,
            hw_params, &numSamples );
    if( rtnStatus < 0 )
        goto openExit;


    /* Set the hardware parameters in the PCM*/
    rtnStatus = snd_pcm_hw_params( ALSAData->alsa_handle, hw_params );
    if( rtnStatus < 0 )
        goto openExit;



    /* Set software parameters to interrupt at the end of an audio buffer 
     number of samples and to start-up manually */
    rtnStatus = snd_pcm_sw_params_malloc( &sw_params );
    if( rtnStatus < 0 )
        goto openExit;

    rtnStatus = snd_pcm_sw_params_current( ALSAData->alsa_handle, sw_params );
    if( rtnStatus < 0 )
        goto openExit;

    /* Number of samples needed in PCM buffer prior to interrupt */
    rtnStatus = snd_pcm_sw_params_set_avail_min( ALSAData->alsa_handle,
            sw_params, ALSAData->read_samples );
    if( rtnStatus < 0 )
        goto openExit;

    /* Set start threshold so startup is done manually */
    rtnStatus = snd_pcm_sw_params_set_start_threshold( ALSAData->alsa_handle,
            sw_params, ULONG_MAX );
    if( rtnStatus < 0 )
        goto openExit;

    /* Channel opened successfully */
    openSuccess = 1;


openExit:

    if( hw_params != NULL )
        snd_pcm_hw_params_free( hw_params );

    if( sw_params != NULL )
        snd_pcm_sw_params_free( sw_params );


    if( openSuccess )
        return TIesrFA_ALSAErrNone;

    return TIesrFA_ALSAErrFail;
}

/*----------------------------------------------------------------
 TIesrFA_start

 This function starts data collection on an opened channel.  If an
 audio channel is opened, then data collection starts in a new thread.
 If audio data is from a file, then this function just indicates the
 collection started successfully if the file pointer is valid.
  ----------------------------------------------------------------*/
TIesrFA_Error_t TIesrFA_start( TIesrFA_t * const aTIesrFAInstance )
{
    int frame;
    int result;
    int threadresult;
    TIesrFA_ALSA_Error_t error;

    /* Thread synchronization startup variables. */
    pthread_attr_t attr;
    struct sched_param sparam;

    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;


    /* The audio source must have opened successfully to be started */
    if( ALSAData->state != TIesrFA_ALSAStateOpen )
    {
        return TIesrFAErrState;
    }

    /* If audio data is from a file, just return if file is opened */
    if( ALSAData->file_mode )
    {
        if( ALSAData->file_pointer == NULL )
        {
            return TIesrFAErrFail;
        }
        else
        {
            ALSAData->state = TIesrFA_ALSAStateRunning;
            return TIesrFAErrNone;
        }
    }


    /* Initialize the thread synchronization parameters */
    error = TIesrFA_ALSA_initsync( ALSAData );
    if( error != TIesrFA_ALSAErrNone )
    {
        return TIesrFAErrFail;
    }


    /* Prepare all of the variables for data collection */
    ALSAData->input_buffer_frame = 0;
    ALSAData->output_buffer_frame = 0;
    ALSAData->buffer_frames_full = 0;
    ALSAData->read_data = TRUE;
    ALSAData->buffer_overflow = FALSE;

    /* Initialize all buffers to holding no bytes of data */
    for( frame = 0; frame < aTIesrFAInstance->num_circular_buffer_frames; frame++ )
    {
        ALSAData->frame_bytes[frame] = 0;
    }


    /* Set thread attributes and priority */
    result = pthread_attr_init( &attr );
    result = pthread_attr_getschedparam( &attr, &sparam );
    sparam.sched_priority = aTIesrFAInstance->priority;
    result = pthread_attr_setschedparam( &attr, &sparam );


    /* Start the audio channel read data thread */
    threadresult = pthread_create( &( ALSAData->thread_id ), &attr,
            TIesrFA_ALSA_thread, (void *) aTIesrFAInstance );

    result = pthread_attr_destroy( &attr );

    if( threadresult != 0 )
    {
        ALSAData->read_data = FALSE;
        return TIesrFAErrThread;
    }


    /* Wait for the thread to indicate it has started */
    sem_wait( &( ALSAData->start_semaphore ) );
    /* printf( "Thread started\n" );*/

    /* Check if thread started PCM ok.  If not, it will return zero in
     read_buffer flag, and the thread will be terminated.  Wait for thread
     to complete.*/
    if( !ALSAData->read_data )
    {
        /* Wait for thread to complete */
        void * status;
        pthread_join( ALSAData->thread_id, &status );

        return TIesrFAErrThread;
    }

    ALSAData->state = TIesrFA_ALSAStateRunning;

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_ALSA_initsync

 Initialize the synchronization objects for the circular buffer.
 ----------------------------------------------------------------*/
TIesrFA_ALSA_Error_t TIesrFA_ALSA_initsync( TIesrFA_ALSA_t * const aALSAData )
{
    int status;

    status = pthread_mutex_init( &( aALSAData->buffer_mutex ), NULL );
    if( status != 0 )
    {
        return TIesrFA_ALSAErrFail;
    }


    status = sem_init( &( aALSAData->buffer_semaphore ), 0, 0 );
    if( status != 0 )
    {
        return TIesrFA_ALSAErrFail;
    }

    status = sem_init( &( aALSAData->start_semaphore ), 0, 0 );
    if( status != 0 )
    {
        return TIesrFA_ALSAErrFail;
    }

    return TIesrFA_ALSAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_getframe

 The user is requesting a frame of data from the audio channel.
 Check if any frames of data are available.  If blocking is
 requested, and no frame data is available, block for next frame.
 Otherwise if blocking is not requested and no frame data is
 available, just return error information that a frame of data is not
 available yet.

 If audio is coming from a file, then just read the data from the
 file.
 ----------------------------------------------------------------*/
TIesrFA_Error_t TIesrFA_getframe( TIesrFA_t * const aTIesrFAInstance,
        unsigned char* const aFrame,
        const int aBlock,
        unsigned int* aFramesQueued )
{
    int waitresult;
    unsigned char* bufFrame;
    int bufferOverflow;
    unsigned int numBytesRead;

    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;


    /* The audio source must be reading audio */
    if( ALSAData->state != TIesrFA_ALSAStateRunning )
    {
        return TIesrFAErrState;
    }


    /* If audio is coming from the file, just read the samples.
     If at the end of the file, just continue to output the last frame,
     but notify calling function by setting EOF return status. */
    if( ALSAData->file_mode )
    {
        /* No backlogged frames */
        *aFramesQueued = 0;

        if( !feof( ALSAData->file_pointer ) )
        {
            numBytesRead = fread( aFrame, sizeof(unsigned char),
                    ALSAData->frame_size,
                    ALSAData->file_pointer );

            if( numBytesRead == ALSAData->frame_size )
            {
                /* Save copy of the latest full frame */
                memcpy( ALSAData->frame_data, aFrame, ALSAData->frame_size );
            }
            else
            {
                /* Output the copy of the last full frame */
                memcpy( aFrame, ALSAData->frame_data, ALSAData->frame_size );
                return(TIesrFAErrEOF );

            }
        }
        else
        {
            /* Output a copy of the last full frame if no more data. */
            memcpy( aFrame, ALSAData->frame_data, ALSAData->frame_size );
            return(TIesrFAErrEOF );
        }

        return TIesrFAErrNone;
    }


    /* Not reading from a file, so reading from live channel.
     Check to see if a frame of data is available.  The thread may also
     be implemented to return some read failure status. It is assumed here
     the thread continues working until requested to stop. */
    if( aBlock )
    {
        /* Block waiting for a circular buffer frame to become available */
        waitresult = sem_wait( &( ALSAData->buffer_semaphore ) );
        if( waitresult != 0 )
        {
            return TIesrFAErrFail;
        }
    }
    else
    {
        /* Check for frame available right now, if not notify user */
        waitresult = sem_trywait( &( ALSAData->buffer_semaphore ) );
        if( waitresult != 0 )
        {
            if( errno == EAGAIN )
                return TIesrFAErrNoFrame;
            else
                return TIesrFAErrFail;
        }
    }

    /* A frame is available, so transfer it to the user's memory */
    bufFrame = ALSAData->circ_buffer + ALSAData->output_buffer_frame * ALSAData->frame_size;
    memcpy( aFrame, bufFrame, ALSAData->frame_size );

    /* Update the circular buffer samples and pointer to indicate we
       are finished with the frame.  This is mutex protected, to
       protect the circular buffer frame access by both threads */
    pthread_mutex_lock( &( ALSAData->buffer_mutex ) );

    ALSAData->frame_bytes[ ALSAData->output_buffer_frame ] = 0;

    ALSAData->buffer_frames_full--;

    *aFramesQueued = ALSAData->buffer_frames_full;

    bufferOverflow = ALSAData->buffer_overflow;

    pthread_mutex_unlock( &( ALSAData->buffer_mutex ) );

    /* Go to the next output circular buffer frame, which the read
       thread will fill or has already filled. */
    ALSAData->output_buffer_frame++;
    if( ALSAData->output_buffer_frame >= aTIesrFAInstance->num_circular_buffer_frames )
    {
        ALSAData->output_buffer_frame = 0;
    }


    /* Successfully transferred a frame */
    return( bufferOverflow ) ? TIesrFAErrOverflow : TIesrFAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_ALSA_thread

 This function is the thread function that reads audio sample data
 from the audio channel and puts the data into the circular buffers.
 --------------------------------*/
void* TIesrFA_ALSA_thread( void* aArg )
{
    int rtnval;
    int pcmStarted = FALSE;

    /* The argument to the thread is the TIesrFA instance */
    TIesrFA_t * const aTIesrFAInstance = (TIesrFA_t * const) aArg;

    /* TIesrFA_ALSA specific data */
    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;


    /* Start the capture of audio data by the audio channel.  If it failed to
     start, then set the read_data flag to zero, indicating failure, and 
     the thread will terminate. */
    rtnval = snd_pcm_start( ALSAData->alsa_handle );
    if( rtnval < 0 )
        ALSAData->read_data = FALSE;
    else
        pcmStarted = TRUE;


    /* Notify main thread that TIesrFA thread has attempted to start the PCM.
     Success or failure is set in the read_data value. If failed, then the
     thread will terminate immediately. */
    sem_post( &( ALSAData->start_semaphore ) );

    /* Loop reading sample data until requested to stop */
    while( ALSAData->read_data == TRUE )
    {
        snd_pcm_sframes_t numSamples;

        /* Wait for audio data to become available.  If no data in one second,
         then something bad has happened. */
        rtnval = snd_pcm_wait( ALSAData->alsa_handle, 1000 );
        if( rtnval == 0 )
        {
            /* Timeout indicating some bad failure. */
            continue;
        }

        /* PCM has indicated data is available. Get amount of data available */
        numSamples = snd_pcm_avail_update( ALSAData->alsa_handle );
        if( numSamples < 0 )
        {
            /* Assume buffer overflow condition */
            ALSAData->buffer_overflow = 1;

            /* Try to recover */
            snd_pcm_prepare( ALSAData->alsa_handle );

            continue;
        }


        /* Determine number of samples to read.  Must not exceed buffer size. */
        numSamples = numSamples > ALSAData->read_samples ?
                ALSAData->read_samples : numSamples;

        /* Read the samples from the PCM */
        numSamples = snd_pcm_readi( ALSAData->alsa_handle,
                ALSAData->read_buffer, numSamples );

        /* Transfer samples to circular frame buffers, or handle error on read. */
        if( ALSAData->read_data == TRUE )
        {
            if( numSamples > 0 )
            {
                int numBytesRead = numSamples * ALSAData->sample_size;
                TIesrFA_ALSA_fillframes( aTIesrFAInstance, numBytesRead );
            }

            else if( numSamples == -EPIPE )
            {
                ALSAData->buffer_overflow = 1;
                snd_pcm_prepare( ALSAData->alsa_handle );
                continue;
            }
            else
            {
                /* Some other failure */
                continue;
            }
        }

    }

    /* The flag has been set to stop processing audio data, so terminate the thread */

    if( pcmStarted )
        snd_pcm_drop( ALSAData->alsa_handle );

    return(void*) TIesrFA_ALSAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_ALSA_fillframes

 The audio channel has returned some data.  Put it in one or more
 circular buffer frames, and if circular buffer frames are filled, then
 update the circular buffer frame pointer and semaphore accordingly.
 --------------------------------*/
TIesrFA_ALSA_Error_t TIesrFA_ALSA_fillframes( TIesrFA_t * const aTIesrFAInstance, const int aBytesRead )
{
    unsigned int framesFull;
    unsigned char* bufFrame;
    unsigned int frameBytesRemaining;
    unsigned int frameOffset;

    /* Initialize read buffer tracking variables. */
    int bufBytesRemaining = aBytesRead;
    int bufOffset = 0;

    /* FILE specific data */
    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;

    /* printf( "BytesRead %d\n", aBytesRead ); */

    while( bufBytesRemaining > 0 )
    {
        /* Determine if all buffer frames are full.  If so, there is an overflow condition, so
         abort putting data in frames.  Mutex protect the fetch of the count of number of
         frames full and status setting. */
        pthread_mutex_lock( &( ALSAData->buffer_mutex ) );

        framesFull = ALSAData->buffer_frames_full;

        /* printf( "Frames full: %d\n", framesFull ); */

        if( framesFull == aTIesrFAInstance->num_circular_buffer_frames )
        {
            /* log that a buffer overflow occurred */
            ALSAData->buffer_overflow = TRUE;
            pthread_mutex_unlock( &( ALSAData->buffer_mutex ) );
            return TIesrFA_ALSAErrOverflow;
        }

        pthread_mutex_unlock( &( ALSAData->buffer_mutex ) );


        /* The input buffer frame is not full, so put data into it */
        bufFrame = ALSAData->circ_buffer + ALSAData->input_buffer_frame * ALSAData->frame_size;

        /* This should be ok unprotected by a mutex, since if the frame is not full, the main thread will
         not be clearing the frame_bytes variable for this frame */
        frameBytesRemaining = ALSAData->frame_size -
                ALSAData->frame_bytes[ ALSAData->input_buffer_frame ];

        frameOffset = ALSAData->frame_size - frameBytesRemaining;

        /* Check if there are enough read buffer bytes to completely fill the present input
         circular buffer frame */
        if( bufBytesRemaining >= frameBytesRemaining )
        {
            memcpy( bufFrame + frameOffset, ALSAData->read_buffer + bufOffset, frameBytesRemaining );
            ALSAData->frame_bytes[ ALSAData->input_buffer_frame ] += frameBytesRemaining;
            bufBytesRemaining -= frameBytesRemaining;
            bufOffset += frameBytesRemaining;

            /* printf( "Fill %d with %d\n", ALSAData->input_buffer_frame, frameBytesRemaining ); */

            /* Indicate to the main thread that a new frame is available.  This is mutex protected. */
            pthread_mutex_lock( &( ALSAData->buffer_mutex ) );

            ALSAData->buffer_frames_full++;

            /* Another frame is available for processing indicated by the semaphore */
            sem_post( &( ALSAData->buffer_semaphore ) );

            pthread_mutex_unlock( &( ALSAData->buffer_mutex ) );


            /* Go to the next input buffer frame, since the present one is now full */
            ALSAData->input_buffer_frame++;
            if( ALSAData->input_buffer_frame >= aTIesrFAInstance->num_circular_buffer_frames )
            {
                ALSAData->input_buffer_frame = 0;
            }
        }


            /* There are not enough buffer bytes to fill the present input circular buffer frame.  Put
             all remaining buffer bytes into the circular buffer frame, but it is not full yet so we
             do not need to update any inter-thread buffer status.  This will exit the while loop,
             since no more read_buffer bytes remain to be processed. */
        else
        {
            memcpy( bufFrame + frameOffset, ALSAData->read_buffer + bufOffset, bufBytesRemaining );
            ALSAData->frame_bytes[ ALSAData->input_buffer_frame ] += bufBytesRemaining;

            /* printf( "Add to %d with %d\n", ALSAData->input_buffer_frame, bufBytesRemaining ); */

            bufBytesRemaining = 0;
        }
    }


    /* Done successfully filling the circular buffer from the read buffer */
    return TIesrFA_ALSAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_stop

 This function call stops data collection.  If the data is from 
 a file, then the state is just changed.  Normally for an audio channel,
 the data reading thread is terminated and then the channel data collection
 is stopped. This function does not close the audio channel, which is still
 available for doing another recording by calling TIesrFA_start.
  ----------------------------------------------------------------*/
TIesrFA_Error_t TIesrFA_stop( TIesrFA_t * const aTIesrFAInstance )
{
    void *status;
    TIesrFA_ALSA_Error_t error;


    /* FILE specific data */
    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;

    if( ALSAData->state != TIesrFA_ALSAStateRunning )
    {
        return TIesrFAErrState;
    }


    /* If audio is coming from a file, just rewind it */
    if( ALSAData->file_mode )
    {
        rewind( ALSAData->file_pointer );

        ALSAData->state = TIesrFA_ALSAStateOpen;

        return TIesrFAErrNone;
    }


    /* Set variable to stop reading data and shut down thread */
    ALSAData->read_data = FALSE;

    /* Wait for thread to complete */
    pthread_join( ALSAData->thread_id, &status );

    /* printf( "Thread ended\n"); o*/

    /* Reset sync objects */
    error = TIesrFA_ALSA_resetsync( ALSAData );

    ALSAData->state = TIesrFA_ALSAStateOpen;

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_ALSA_resetsync

 Reset all of the synchronization objects.  They must be reinitialized
 to use them again.
 --------------------------------*/
TIesrFA_ALSA_Error_t TIesrFA_ALSA_resetsync( TIesrFA_ALSA_t * const aALSAData )
{
    int status;

    status = pthread_mutex_destroy( &( aALSAData->buffer_mutex ) );

    status = sem_destroy( &( aALSAData->start_semaphore ) );

    status = sem_destroy( &( aALSAData->buffer_semaphore ) );

    return TIesrFA_ALSAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_close

 Close an audio channel.  This will just close the file in file mode. For
 normal audio channels, this would close the audio channel.
  ----------------------------------------------------------------*/
TIesrFA_Error_t TIesrFA_close( TIesrFA_t * const aTIesrFAInstance )
{

    int rtnStatus;

    /* FILE specific data */
    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;

    if( ALSAData->state != TIesrFA_ALSAStateOpen )
    {
        return TIesrFAErrState;
    }


    /* If data is coming from a file, close the file, and free frame data memory */
    if( ALSAData->file_mode )
    {
        fclose( ALSAData->file_pointer );
        ALSAData->file_pointer = NULL;

        free( ALSAData->frame_data );
        ALSAData->frame_data = NULL;

        ALSAData->state = TIesrFA_ALSAStateInit;
        return TIesrFAErrNone;
    }


    /* Close the audio channel here, and check for failure */
    rtnStatus = snd_pcm_close( ALSAData->alsa_handle );
    if( rtnStatus < 0 )
        return TIesrFAErrFail;


    /* The audio channel closed successfully */
    ALSAData->state = TIesrFA_ALSAStateInit;

    return TIesrFAErrNone;
}

/*----------------------------------------------------------------
 TIesrFA_destroy
  
 This function is supplied to free all memory allocated to an
 instance of the audio channel.  It does not destroy the instance
 object itself.
 --------------------------------*/
TIesrFA_Error_t TIesrFA_destroy( TIesrFA_t * const aTIesrFAInstance )
{
    /* FILE specific data */
    TIesrFA_ALSA_t * const ALSAData = (TIesrFA_ALSA_t * const) aTIesrFAInstance->impl_data;

    if( aTIesrFAInstance->channel_name != NULL )
    {
        free( aTIesrFAInstance->channel_name );
        aTIesrFAInstance->channel_name = NULL;
    }


    /* Check for already in an uninitialized state */
    if( ALSAData == NULL )
    {
        return TIesrFAErrNone;
    }


    /* Check if the FILE channel is not closed */
    if( ALSAData->state != TIesrFA_ALSAStateInit )
    {
        return TIesrFAErrState;
    }


    /* Close file if open.  Should not happen */
    if( ALSAData->file_mode )
    {
        if( ALSAData->file_pointer != NULL )
        {
            fclose( ALSAData->file_pointer );
            ALSAData->file_pointer = NULL;
        }
    }


    /* Free all allocated memory */
    if( ALSAData->circ_buffer != NULL )
    {
        free( ALSAData->circ_buffer );
        ALSAData->circ_buffer = NULL;
    }

    if( ALSAData->read_buffer != NULL )
    {
        free( ALSAData->read_buffer );
        ALSAData->read_buffer = NULL;
    }

    if( ALSAData->frame_bytes != NULL )
    {
        free( ALSAData->frame_bytes );
        ALSAData->frame_bytes = NULL;
    }


    /* Free the FILE impl_data object itself.  This removes any FILE
       implementation state information */
    free( ALSAData );
    aTIesrFAInstance->impl_data = NULL;



    return TIesrFAErrNone;
}
