/*=======================================================================

 *
 * TestTIesrFA.cpp
 *
 * Program to test TIesrFA API.
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
 
 This program tests the TIesrFA API by recording audio input. The command
 line for the program is:

 TestTIesrFA sampleRate frameSamples numFrames numCircFrames numAudioFrames \
 readRate device filename

 sampleRate = sample rate in Hz

 frameSamples = number of samples in one audio frame

 numFrames = number of audio frames to record

 numCircFrames = number of audio frames in the circular input buffer

 numAudioFrames = number of buffer frames used by audio device. Buffer frames
 and audio frames are not necessarily the same size.
 
 readRate = Value depends on the TIesrFA API implementation. For TIesrFA_ALSA
 it is the number of microseconds between successive reads of the audio buffers,
 typically 40000. For TIesrFA_WM5 (Windows) it is the multiple of audio frame
 size for each device buffer frame, typically 2.

 device = audio device name, which depends on the OS and TIesrFA implementation.
 For TIesrFA_ALSA it is set to plughw:0,0 and for TIesrFA_WM5 it is set to
 any non-file name string such as "WinAudio"

 filename = output file name

======================================================================*/

#if defined( WIN32 )
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "TIesrFA_User.h"

#if defined(LINUX)
#define THREAD_PRIORITY 20
#elif defined( WIN32 )
#define THREAD_PRIORITY THREAD_PRIORITY_ABOVE_NORMAL
#endif

int main( int argc, char** argv )
{
   int sampleRate;
   int frameSamples;
   int numFrames;
   int numCircularFrames;
   int numAudioFrames;
   int readRate;
   char *device;
   char *filename;
   unsigned int framesQueued;

   TIesrFA_t aTIesrFAInstance;
   TIesrFA_Error_t error;
   int frameCount;
   unsigned char* sampleBuf;
   FILE *fp;

   sampleRate = atoi( argv[1] );
   frameSamples = atoi( argv[2] );
   numFrames = atoi( argv[3] );
   numCircularFrames = atoi( argv[4] );
   numAudioFrames = atoi( argv[5] );
   readRate = atoi( argv[6] );
   device = argv[7];
   filename = argv[8];

   fp = fopen( filename, "wb");
   if( fp == NULL )
   {
      printf( "File open fail\n" );
      return -1;
   }

   sampleBuf = (unsigned char*) malloc( frameSamples*2*sizeof(unsigned char) );

   error = TIesrFA_init( &aTIesrFAInstance, sampleRate, TIesrFALinear, frameSamples, 
			 numCircularFrames, numAudioFrames, 
			 readRate, device, THREAD_PRIORITY );
   if( error != TIesrFAErrNone )
   {
      printf( "Init fail %d\n", error );
      return 1;
   }

   printf( "Instance initialized\n" );

   error = TIesrFA_open( &aTIesrFAInstance );
   if( error != TIesrFAErrNone )
   {
      printf( "Open fail %d\n", error );
      return 2;
   }

   printf( "Instance opened\n" );


   error = TIesrFA_start( &aTIesrFAInstance );
   if( error != TIesrFAErrNone )
   {
      printf( "Start fail %d\n", error );
      return 3;
   }

   printf( "Speak now...\n" );
   for( frameCount=0; frameCount<numFrames; frameCount++ )
   {
      error = TIesrFA_getframe( &aTIesrFAInstance, sampleBuf, 1, &framesQueued );

      if( error != TIesrFAErrNone )
      {
	 printf( "Get fail %d\n", error );
	 return 4;
      }

      fwrite( sampleBuf, sizeof( unsigned char), frameSamples*2, fp );

   }

   printf( "Done Recording\n" );
   fclose(fp);

   error = TIesrFA_stop( &aTIesrFAInstance );
   if( error != TIesrFAErrNone )
   {
      printf( "Stop fail %d\n", error );
      return 5;
   }
   printf( "Instance stopped\n" );


   error = TIesrFA_close( &aTIesrFAInstance );
   if( error != TIesrFAErrNone )
   {
      printf( "Close fail %d\n", error );
      return 6;
   }
   printf( "Instance closed\n" );


   error = TIesrFA_destroy( &aTIesrFAInstance );
   if( error != TIesrFAErrNone )
   {
      printf( "Destroy fail %d\n", error );
      return 7;
   }

   printf( "Instance destroyed\n" );
   return 0;
}
