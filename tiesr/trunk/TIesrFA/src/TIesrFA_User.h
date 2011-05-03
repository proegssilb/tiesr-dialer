/*=======================================================================
 
 *
 * TIesrFA_User.h
 *
 * Header defining the TIesrFA API interface.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License 2.1 as
 * published by the Free Software Foundation and with respect to the additonal 
 * permission below.
 *
 * This program is distributed “as is” WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * As an additional permission to the GNU Lesser General Public License version
 * 2.1, the object code form of a "work that uses the Library" may incorporate
 * material from a header file that is part of the Library.  You may distribute
 * such object code under terms of your choice, provided that:
 *   (i)   the header files of the Library have not been modified; and 
 *   (ii)  the incorporated material is limited to numerical parameters, data
 *         structure layouts, accessors, macros, inline functions and
 *         templates; and
 *   (iii) you comply with the terms of Section 6 of the GNU Lesser General
 *         Public License version 2.1.
 *
 * Moreover, you may apply this exception to a modified version of the Library,
 * provided that such modification does not involve copying material from the
 * Library into the modified Library's header files unless such material is
 * limited to (i) numerical parameters; (ii) data structure layouts;
 * (iii) accessors; and (iv) small macros, templates and inline functions of
 * five lines or less in length.
 *
 * Furthermore, you are not required to apply this additional permission to a
 * modified version of the Library.
 *

 This is the header file for using the TIesrFA frame audio API.  The TIesrFA
 API provides the means of obtaining frames of audio data for use by the
 TIesr recognizer.  This header file defines the interface with the API.  The
 API provides a means of creating an instance of a single monaural audio channel.
 Note that an audio channel may be a live hardware audio channel, or a
 file containing binary audio data.

 This interface provides the basis for designing the TIesrFA API for a large
 number of OS platforms.

======================================================================*/

#ifndef _TIESRFA_USER_H
#define _TIESRFA_USER_H

/* Windows OS DLL function export method */
#if defined( WIN32 ) || defined( WINCE )

/*------------------------------------------------------------------
 The following ifdef block is the standard way of creating macros which make exporting
 from a DLL simpler. All files within this DLL are compiled with the TIESRFA_EXPORTS
 symbol defined on the command line. This symbol should not be defined on any project
 that uses this DLL. This way any other project whose source files include this file see
 TIESRFA_API functions as being imported from a DLL, whereas this DLL sees symbols
 defined with this macro as being exported.
 ---------------------------------------------------------------------*/
#ifdef TIESRFA_EXPORTS
#define TIESRFA_API __declspec(dllexport)
#else
#define TIESRFA_API __declspec(dllimport)
#endif

#else
#define TIESRFA_API 
#endif 

#if defined( __cplusplus )
extern "C"
{
#endif
/*----------------------------------------------------------------------
 TIesrFA_error defines all returned status reporting of the API.  These
 may be used in any way a particular implementation desires.  However,
 general definitions are as follows:

 TIesrFAErrNone = No error from function's request

 TIesrFAErrFail = The request failed, and the instance is in a condition
 as if the function call had not been made.

 TIesrFAErrArg = Invalid argument

 TIesrFAErrState = State error. For example, trying to open an already
 running  audio data channel collection.

 TIesrFAErrThread = Thread start-up error, for those cases in which the
 data collection occurs in a separate thread.  Use of a thread is normally
 the case, so that data can be collected while the recognizer is processing.

 TIesrFAErrNoFrame = A non-blocking request for a frame was made and no
 frame is available.

 TIesrFAErrOverflow = A frame of data was returned.  However, it has been detected
 that an overflow condition occurred at some frame, and some data is lost at some
 frame.
 ---------------------------------------------------------------------------------*/
typedef enum TIesrFA_Errors
{
  TIesrFAErrNone,
  TIesrFAErrFail,
  TIesrFAErrArg,
  TIesrFAErrState,
  TIesrFAErrThread,
  TIesrFAErrNoFrame,
  TIesrFAErrOverflow,
  TIesrFAErrEOF
} TIesrFA_Error_t;


/*--------------------------------------------------------------
 TIesrFA_encoding defines the type of data returned.  At this time
 TIesr is expecting TIesrFALinear only as 16 bit PCM samples.
 --------------------------------------------------------------*/
typedef enum TIesrFA_Encodings
{
  TIesrFALinear,
  TIesrFAUlaw,
  TIesrFAAlaw
} TIesrFA_Encoding_t;

/*---------------------------------------------------------------------------
 TIesrFA_t defines the structure that encapsulates an instance of an audio
 channel. The user must create an object of this structure in order to
 obtain a connection to an audio channel.  The definition of this structure
 is given here so that it can be allocated on the stack or in heap.  The
 user must never directly modify any of the parameters in the structure, but
 should only rely on the interface functions to do so.  The structure consists
 of a few basic parameters that most audio sampling implementations will define,
 along with the capability for each hardware device to expand the definitions.
 ---------------------------------------------------------------------------*/
typedef struct TIesrFA_Struct
{
      /* The A/D sampling rate in samples per second */
      unsigned int sample_rate;
      
      /* The type of encoding desired */
      TIesrFA_Encoding_t encoding;

      /* The number of samples per frame returned to user */
      unsigned int samples_per_frame;

      /* The number of circular buffer frames that will be used to
       store data as a buffer to maintain real time */
      unsigned int num_circular_buffer_frames;

      /* The number of audio input buffer frames.  Some audio APIs may
       require that one or more input buffers be created.  This
       parameter defines the size of the buffer in terms of number
       of frames of samples.  This parameter may not be used by all
       audio APIs, in which case it can be used as an additional
       API parameter. */
      unsigned int num_audio_buffer_frames;

      /* The rate to attempt reading audio data.  Some audio APIs may
       require that the audio input be read at some periodic rate.
       This parameter defines the rate in microseconds at which the
       reading should take place.  This parameter may not be used
       with all audio APIs, in which case it can be used as an additional
       API parameter.  */
      unsigned long audio_read_rate;

      /* A character string that identifies which audio channel to
       open.  Each implementation and platform will handle this
       string differently.  If the channel_name parameter is the
       name of a file, then the file will be opened and used as a
       source of audio data. */
      char* channel_name;
     
      /* An integer which defines the priority at which the audio
       channel will run, if in a separate thread.  Each
       implementation and platform will handle this integer
       according to the particular OS and device */
      int priority;

      /* A pointer to implementation dependent data.  This data will
       depend on the particular interface with the hardware audio
       driver supplied on a particular platform and OS. */
      void* impl_data;

} TIesrFA_t;


/*-------------------------------------------------------------------
 TIesrFA_init initializes the TIesrFA_t object for an instance of a
 TIesrFA audio channel.  It must be called prior to using the
 TIesrFA_t object for audio collection. It provides the opportunity
 for an implementation to initialize itself prior to opening an
 audio channel connection.  This will usually include establishing
 OS-dependent parameters within the impl_data object in the
 TIesrFA_t structure.  It also allows for parameter checking to
 ensure that the type of channel parameters requested can be
 supported.  The user passes a pointer to the object in this
 function call.  Thus the TIesrFA_t object may be allocated on the
 stack or in heap.  However, it must not go out of scope until all
 data collection is finished and all memory allocated within the
 object is destroyed by a call to TIesrFA_destroy.
-----------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_init( TIesrFA_t* const  aTIesrFAInstance,
			      const unsigned int sample_rate, 
			      const TIesrFA_Encoding_t encoding,
			      const unsigned int samples_per_frame,
			      const unsigned int num_circular_buffer_frames,
			      const unsigned int num_audio_buffer_frames,
			      const unsigned long audio_read_rate,
			      const char* channel_name,
			      const int priority );


/*--------------------------------------------------------------------
 TIesrFA_open opens an audio channel.  Opening an audio channel does
 not start collection of audio data.  It establishes a connection
 with an audio device. No frames of audio data can be received until
 an instance has been initialized and opened.
----------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_open( TIesrFA_t* const aTIesrFAInstance );


/*------------------------------------------------------------------
 TIesrFA_start starts data collection.  Upon return the audio
 collection should be running. Audio collection may be started in a
 separate thread.
--------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_start( TIesrFA_t* const aTIesrFAInstance );


/*-------------------------------------------------------------------
 TIesrFA_getframe requests a frame of data from a running audio
 channel.  This function examines the circular frame buffer to see
 if a frame of data is available. If a frame is available, it
 transfers the data to the aFrame location specified in the function
 call.  If a frame of data is not available, then the aBlock boolean
 variable will determine if the function blocks waiting for a frame,
 or if the function returns indicating no frame is yet
 available. The parameter aFramesQueued returns the number of frames
 in the circular buffer remaining to be processed, not including the
 frame returned.
 ----------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_getframe( TIesrFA_t* const aTIesrFAInstance, 
				  unsigned char * const aFrame, 
				  const int aBlock,
				  unsigned int *aFramesQueued );


/*-------------------------------------------------------------------
 TIesrFA_stop stops data collection.  Upon return the audio
 collection has stopped, and if a separate thread has been used for
 collection, that thread of execution has terminated.  Audio
 collection may be started again by another call to TIesrFA_start.
 Audio channel resources remain connected.
 ---------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_stop( TIesrFA_t* const aTIesrFAInstance );


/*---------------------------------------------------------------------
 TIesrFA_close closes the connection with the audio channel. In
 order to start collection again, the user must call TIesrFA_open to
 re-establish an audio channel connection.
 ----------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_close( TIesrFA_t* const aTIesrFAInstance );


/*----------------------------------------------------------------------
 TIesrFA_destroy allows the particular implementation to destroy any
 objects it may have created upon initialization.  This function must
 be called after initialization has successfully completed in order
 to ensure that no memory leaks occur.  It can not be called
 successfully when the audio channel is opened.  Calling this
 function does not destroy the TIesrFA_t object itself.
 ----------------------------------------------------------------------*/
TIESRFA_API
TIesrFA_Error_t TIesrFA_destroy( TIesrFA_t* const aTIesrFAInstance );

/*----------------------------------------------------------------------
 TIesrFA_status returns a single unsigned integer of information
 regarding the operation of the TIesrFA API.  The contents of this
 variable will be dependent upon the particular implementation of the API,
 and must be documented by the API implementer. This function will only
 return valid information after TIesrFA_init is successfully called,
 and prior to calling TIesrFA_destroy.  This function is optional and not
 required for operation of the TIesr recognizer.  It provides a mechanism
 to report status of the data collection for diagnostic purposes.
 -----------------------------------------------------------------------*/
TIESRFA_API
unsigned int TIesrFA_status( TIesrFA_t* const aTIesrFAInstance );

#if defined( __cplusplus )
}
#endif
#endif
