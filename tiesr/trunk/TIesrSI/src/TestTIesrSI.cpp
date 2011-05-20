/*=======================================================================

 *
 * TestTIesrSI.cpp
 *
 * Program to test TIesrSI API.
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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined( LINUX)
#include <errno.h>

#if defined(DEBUG)
#include <mcheck.h>
#endif

#endif

// Include the app macros, constants and structures
#include "TestTIesrSI.h"

void AddToDialerBuffer( char word );
int SendToDialer( char* buffer );

/*--------------------------------*/
int main( int argc, char** argv )
{

   /* recognition object and error */
   Reco_t reco;
   RecoError_t recoError;

   // The recognition semaphores for TIesrSI Callbacks
#if defined (LINUX)
   sem_t start_semaphore;
   sem_t done_semaphore;
   int result;
#elif defined ( WIN32 )
#endif

#if defined (DEBUG)
   mtrace( );
#endif

   // The TIesrSI recognizer instance variable.  It will hold a
   // TIesrSI recognizer instance.
   TIesrSI_t tiesrSI;
   TIesrSI_Error_t siError;


   /* speech data input file list variables */

   /* Initialization of reco structure with command line arguments */
   if( argc != 6 && argc != 7 )
   {
      printf( "Invalid argument count\n" );
      return 1;
   }

   reco.memorySize = atoi( argv[1] );
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
   sem_init( reco.start_semaphore, 0, 0 );

   reco.done_semaphore = &done_semaphore;
   sem_init( reco.done_semaphore, 0, 0 );

#elif defined ( WIN32 )
   reco.start_event = CreateEvent( NULL, FALSE, FALSE, NULL );
   reco.done_event = CreateEvent( NULL, FALSE, FALSE, NULL );
#endif


   /* Create a TIesrSI recognizer instance */
   printf( "Creating TIesrSI recognizer\n" );
   siError = TIesrSI_create( &tiesrSI );
   if( siError != TIesrSIErrNone )
   {
      printf( "TIesrSI create error %d\n", siError );
      return 1;
   }
   reco.tiesrSI = tiesrSI;


   /* Open the recognizer instance */
   printf( "Opening TIesrSI recognizer\n" );
   recoError = OpenTIesr( &reco );
   if( recoError != RecoErrNone )
   {
      printf( "Could not open TIesr\n" );
      TIesrSI_destroy( tiesrSI );
      exit( recoError );
   }

int i;
for (i=0;i<10;i++)
{
printf( "Loop%d\n",i );


   /* Start TIesr recognition.  TIesrSI will start recognition in a
   new thread. */
   printf( "Starting TIesrSI recognizer\n" );
   recoError = StartTIesr( &reco );
   if( recoError != RecoErrNone )
   {
      printf( "Could not start TIesr\n" );
      CloseTIesr( &reco );
      TIesrSI_destroy( tiesrSI );
      exit( recoError );
   }

   /* Wait for recognizer to be ready to receive speech.  The TIesrSI
    recognizer will call the SpeakCallback function to signal this
    semaphore when ready. Normally you would do other processing during this
    time and the callback would signal an event when the recognizer is ready. */
#if defined ( LINUX )
   do
   {
      result = sem_wait( reco.start_semaphore );
   }
   while( result && errno == EINTR );
#elif defined ( WIN32 )
   WaitForSingleObject( reco.start_event, INFINITE );
#endif
   if( reco.speakResult != TIesrSIErrNone )
   {
      printf( "Recognizer not ready - spoke too early?\n" );
      StopTIesr( &reco );
      CloseTIesr( &reco );
      TIesrSI_destroy( tiesrSI );
      exit( reco.speakResult );
   }

   /* Prompt user for speech */
   printf( "Speak now...\n" );


   /* Wait for recognition to complete.  TIesrSI will call DoneCallback to
    set the semaphore when recognition is complete.  Normally you would do
    processing at this time, and the callback would signal an event
    when complete. */
#if defined ( LINUX )
   do
   {
      result = sem_wait( reco.done_semaphore );
   }
   while( result && errno == EINTR );
#elif defined ( WIN32 )
   WaitForSingleObject( reco.done_event, INFINITE );
#endif





   /* Stop the recognizer (this also formalizes recognition results
   // within TIesrSI ) */
   printf( "Stopping TIesrSI recognizer...\n" );
   recoError = StopTIesr( &reco );
	 printf( "Stopped TIesrSI recognizer\n" );
   if( recoError != RecoErrNone )
   {
      printf( "Recognizer stop failure\n" );
      CloseTIesr( &reco );
      TIesrSI_destroy( tiesrSI );
      exit( recoError );
   }


   // Check recognition result, and if a recognition result is available
   // output the result.
   if( reco.doneResult != TIesrSIErrNone )
   {
      printf( "Recognition failed with error %d\n", reco.doneResult );

      int recoStatus, audioStatus, jacStatus;
      TIesrSI_status( reco.tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco: %d  Audio: %d JAC: %d\n", recoStatus, audioStatus, jacStatus );

   }
   else
   {
			printf( "Calling OutputResult\n" );
      OutputResult( reco.tiesrSI );
   }
}


   /* Close the recognizer on this file */
   printf( "Closing TIesrSI recognizer\n" );
   recoError = CloseTIesr( &reco );
   if( recoError != RecoErrNone )
   {
      printf( "Failed to close TIesr recognizer\n" );
      exit( recoError );
   }

   /* Destroy the recognizer instance */
   printf( "Destroying TIesrSI recognizer\n" );
   siError = TIesrSI_destroy( reco.tiesrSI );

   /* destroy the semaphore */
#if defined ( LINUX )
   sem_destroy( reco.start_semaphore );
   sem_destroy( reco.done_semaphore );
#elif defined ( WIN32 )
   CloseHandle( reco.start_event );
   CloseHandle( reco.done_event );
#endif

   if( siError != TIesrSIErrNone )
      return siError;

   return TIesrSIErrNone;
}

/*----------------------------------------------------------------*/
void OutputResult( TIesrSI_t aTIesrSI )
{
   TIesrSI_Error_t siError;
   unsigned short numWords;
   unsigned short wd;
   char const *word;
   short confidence;

   /* Number of words in best answer */
   siError = TIesrSI_wdcount( aTIesrSI, &numWords, 0 );

   if( numWords == 0 )
   {
      printf( "No recognition result - no words recognized\n" );
      return;
   }

   /* Output all words of recognized answer except _SIL */
   printf( "Recognized: " );
	 for( wd = 0; wd < numWords; wd++ )
   {
			//printf(":-)\n");
      siError = TIesrSI_word( aTIesrSI, wd, &word, 0 );
      //printf("siError: %d ", siError);
			if( siError != TIesrSIErrNone ){
         printf( "*** " );
			}else if( strcmp( word, "_SIL" ) ){
        printf( "%s ", word );
				
				if( strcmp(word, "zero") == 0 ){
					printf("adding to dialer buffer: 0\n");
					AddToDialerBuffer( '0' );
				}else if( strcmp(word, "one") == 0 ){
					printf("adding to dialer buffer: 1\n");
					AddToDialerBuffer( '1' );
				}else if( strcmp(word, "two") == 0 ){
					printf("adding to dialer buffer: 2\n");
					AddToDialerBuffer( '2' );
				}else if( strcmp(word, "three") == 0 ){
					printf("adding to dialer buffer: 3\n");
					AddToDialerBuffer( '3' );
				}else if( strcmp(word, "four") == 0 ){
					printf("adding to dialer buffer: 4\n");
					AddToDialerBuffer( '4' );
				}else if( strcmp(word, "five") == 0 ){
					printf("adding to dialer buffer: 5\n");
					AddToDialerBuffer( '5' );
				}else if( strcmp(word, "six") == 0 ){
					printf("adding to dialer buffer: 6\n");
					AddToDialerBuffer( '6' );
				}else if( strcmp(word, "seven") == 0 ){
					printf("adding to dialer buffer: 7\n");
					AddToDialerBuffer( '7' );
				}else if( strcmp(word, "eight") == 0 ){
					printf("adding to dialer buffer: 8\n");
					AddToDialerBuffer( '8' );
				}else if( strcmp(word, "nine") == 0 ){
					printf("adding to dialer buffer: 9\n");
					AddToDialerBuffer( '9' );
				}else{
					printf("nothing added to dialer buffer");
				}
					
			}
   }
   printf( "\n" );

   /* Get confidence measure - only valid for "large" grammars now */
   /*
   siError = TIesrSI_confidence( aTIesrSI, &confidence );
   if( siError != TIesrSIErrNone )
       printf( "Could not get confidence measure\n");
   else
       printf( "Confidence: %d\n", confidence );
    */
}

/*----------------------------------------------------------------*
  AddToDialerBuffer
	
	adds recognized strings to buffer to be sent to voice dialer.
	dialer accepts strings of numerics with lengths of 7, 10 or 11.
	
  ----------------------------------------------------------------*/
char* buffer;
int buffer_size = 0;

void AddToDialerBuffer( char number ){
	
	//check to see if input is a numeric
	/*istringstream stream( word );
	double test;
	stream >> test;
	if( !stream ){
		return false;
	}*/
	
	if(buffer_size > 0){
		buffer = (char*)realloc(buffer, (++buffer_size)*sizeof(char));
	}else{
		buffer = (char*)malloc((++buffer_size)*sizeof(char));
	}
	buffer[buffer_size-1] = number;
	buffer[buffer_size] = '\0';
	
	switch(buffer_size) {
		//case 7 :
		//case 11 :
		case 10 :
			printf("buffer full. dialing...\n");
			SendToDialer( buffer );
			//clear buffer
			for (int i = 0; i < buffer_size; i++){
				buffer[i] = '\0';
			}
			buffer_size = 0;
		break;
		
		default :
			printf("buffer size: %d buffer: %s\n", buffer_size, buffer);
		break;
	}
	return;
}

/*----------------------------------------------------------------*
  Send To Dialer
  
	forks the voice dialer process and loads it with numerics read

  ----------------------------------------------------------------*/
int SendToDialer( char* buffer ){
	printf( "Dialing: %s\n", buffer );
	
	int i = 1;
	int result = 0;
	pid_t child_pid;
	
	char* address = "http://maxifig.reshall.rose-hulman.edu:8080/";
	char* arg = (char*)malloc(56*sizeof(char));
	strcpy(arg, address);
	strcat(arg, buffer);
	
	char* wget = "wget";
	char** args = (char**)malloc(56 * sizeof(char*));
	args[0] = wget;
	args[1] = arg;
	printf( "forking to: %s %s\n", args[0], args[1]);
	
	if ((child_pid = fork()) != 0) {
		//Parent process
		//wait for child to complete before continuing
		printf("parent is waiting...\n");
		waitpid(child_pid, &result, 0);
		printf("parent has continued...\n");
		
	}else{
		//Child process
		//fork external dialer or DIE	
		
		i = execv(args[0], args);
		
		//the external command was not found in the pwd or as an absolute path
		//now check the path variable for these programs
		char * path_var = (char*)malloc( sizeof(char) * strlen( getenv("PATH") ) );
		path_var = getenv("PATH");
		
		
		char * slash_cmd = (char*)malloc( (1 + strlen( args[0] ) ) * sizeof(char));
		slash_cmd[0] = '/';
		strcat( slash_cmd, args[0] );

		char * next_path = (char*)malloc( strlen(path_var)*sizeof(char) );
		strcpy( next_path, strtok(path_var, ":") );
		i = execv( strcat( next_path, slash_cmd ), args );
		
		//first path in path variable no good... keep searching
		while( next_path != NULL ){
			i = execv( strcat( next_path, slash_cmd ), args );
			strcpy( next_path, strtok(NULL, ":") );
		}
		
		//code should never get here
			printf( "error in exec: %d\n", i);
		kill(getpid(), SIGKILL);
		
	}
	
	//parent is finished waiting
	printf( "dialing complete.\n" );
	return result;
}

/*----------------------------------------------------------------
  SpeakCallback
 
  This is the callback function sent to TIesrSI that it will call
  when it is ready for speech. This is a simple function that just
  stores the status returned in the callback.
  ---------------------------------------------------------------*/
void SpeakCallback( void* aArg, TIesrSI_Error_t aError )
{
   Reco_t* reco = (Reco_t*) aArg;

   // Just store the status of the callback
   reco->speakResult = aError;
#if defined ( LINUX )
   sem_post( reco->start_semaphore );
#elif defined ( WIN32 )
   SetEvent( reco->start_event );
#endif
}

/*----------------------------------------------------------------
 DoneCallback

 This is the callback function that TIesrSI will call when it has
 completed recognition, either because of an error condition, or
 because the recognizer has completed.
 -----------------------------------------------------------------*/
void DoneCallback( void* aArg, TIesrSI_Error_t aError )
{
   Reco_t* reco = (Reco_t*) aArg;

   // Save recognition status of the callback
   reco->doneResult = aError;

   // Notify the main thread that the recognition has completed
#if defined ( LINUX )
   sem_post( reco->done_semaphore );
#elif defined ( WIN32 )
   SetEvent( reco->done_event );
#endif
}

/*----------------------------------------------------------------
 OpenTIesr

 This app function is a thin wrapper over the TIesrSI open function
 call that opens the TIesrSI recognizer to recognize a given file
 with a created grammar network and model set.  It also sets any
 custom recognizer parameters that may be tested.
 ------------------------------------------------------------------*/
RecoError_t OpenTIesr( Reco_t* reco )
{
   TIesrSI_Error_t siError;
   TIesrSI_Params_t siParams;
   int recoStatus;
   int audioStatus;
   int jacStatus;

   if( reco->tiesrIsOpen )
   {
      return RecoErrOpen;
   }


   // Open TIesrSI connected to an input speech data file
   siError = TIesrSI_open( reco->tiesrSI,
           reco->audioDevice,
           reco->grammarDir,
           reco->jacFile,
           reco->sbcFile,
           reco->memorySize,
           SpeakCallback,
           DoneCallback,
           (void*) reco );

   if( siError != TIesrSIErrNone )
   {
      printf( "Failed to open TIesr\n" );
      siError = TIesrSI_status( reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus );
      return RecoErrFail;
   }
   reco->tiesrIsOpen = TRUE;

   // Update the parameters to those desired.  Parameters will be specified
   // by the STL lab
   siError = TIesrSI_getparams( reco->tiesrSI, &siParams );
   if( siError != TIesrSIErrNone )
   {
      printf( "Failed to get TIesr parameters\n" );
      siError = TIesrSI_status( reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus );
   }

   siParams.sampleRate = SAMPLE_RATE;
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

   siError = TIesrSI_setparams( reco->tiesrSI, &siParams );
   if( siError != TIesrSIErrNone )
   {
      printf( "Failed to set TIesr parameters\n" );
      siError = TIesrSI_status( reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus );
   }

   return RecoErrNone;
}

/*----------------------------------------------------------------
  StartTIesr
 
  This function is a thin wrapper over the TIesr start function call
  to start the TIesrSI recognizer in a new thread. It just ensures
  that the recognizer state is maintained.
  -----------------------------------------------------------------*/
RecoError_t StartTIesr( Reco_t * reco )
{
   TIesrSI_Error_t siError;
   int recoStatus;
   int audioStatus;
   int jacStatus;

   if( !reco->tiesrIsOpen )
   {
      return RecoErrClosed;
   }


   if( reco->state == RECOGNIZING )
   {
      return RecoErrRecognizing;
   }


   // Start the recognizer. Successful completion of
   // this function call means a new TIesrSI thread is running.
   siError = TIesrSI_start( reco->tiesrSI, 0, reco->captureFile );
   if( siError == TIesrSIErrNone )
   {
      reco->state = RECOGNIZING;
      return RecoErrNone;
   }
   else
   {
      printf( "Failed to start TIesr\n" );
      siError = TIesrSI_status( reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus );
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
RecoError_t StopTIesr( Reco_t * reco )
{
   TIesrSI_Error_t siError;
   int recoStatus;
   int audioStatus;
   int jacStatus;

	 printf( "StopTIesr entered...\n");
	 
   if( !reco->tiesrIsOpen )
   {
      return RecoErrClosed;
   }


   if( reco->state == IDLE )
   {
      return RecoErrIdle;
   }


   siError = TIesrSI_stop( reco->tiesrSI );
   if( siError == TIesrSIErrNone )
   {
      reco->state = IDLE;
      return RecoErrNone;
   }
   else
   {
      printf( "Failed to stop TIesr\n" );
      siError = TIesrSI_status( reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus );
      return RecoErrFail;
   }
}

/*----------------------------------------------------------------
 CloseTIesr

 This function is a thin wrapper over the TIesrSI close function
 call.  It maintains the recognizer state.
 ----------------------------------------------------------------*/
RecoError_t CloseTIesr( Reco_t * reco )
{
   TIesrSI_Error_t siError;
   int recoStatus;
   int audioStatus;
   int jacStatus;

   if( !reco->tiesrIsOpen )
   {
      return RecoErrClosed;
   }

   if( reco->state == RECOGNIZING )
   {
      return RecoErrRecognizing;
   }

   siError = TIesrSI_close( reco->tiesrSI );
   if( siError == TIesrSIErrNone )
   {
      reco->tiesrIsOpen = FALSE;
      return RecoErrNone;
   }
   else
   {
      printf( "Failed to close TIesr\n" );
      siError = TIesrSI_status( reco->tiesrSI, &recoStatus, &audioStatus, &jacStatus );
      printf( "Reco/audio status: %d %d %d\n", recoStatus, audioStatus, jacStatus );
      return RecoErrFail;
   }
}




