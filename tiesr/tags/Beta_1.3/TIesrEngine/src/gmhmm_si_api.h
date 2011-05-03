/*=======================================================================

 *
 * gmhmm_si_api.h
 *
 * Header for interface of the TIesr recognizer SI API.
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


 This header defines the interface specific to the TIesr SI API
 structure TIesrEngineSIRECOType.  This defines main SI functions.
 
======================================================================*/


/*
** functions to build a frame-based (live-mode) recognizer. 
*/

// #include "use_si_dll.h" 

#include "tiesr_config.h"

#include "status.h"
#include "winlen.h"
#include "gmhmm_type.h"

/* initialization of ASR:
** The function loads speech HMM and sentence network. Both of them are
** stored in the directory (folder) specified by the argument.
*/



/* GMHMM_SI_API */
/*TIesrEngineStatusType OpenASR(char [], int, void *);*/
TIesrEngineStatusType OpenASR(char[] , int total_words, gmhmm_type *memoryPool);

/* close ASR: */
/* GMHMM_SI_API */
/*void CloseASR(void *);*/
void CloseASR(gmhmm_type *);
 
/* initialization:
** Initialize functions and variables (e.g. utterance detection, frame counter) 
** for recognizing the next utterance.
** RETURN:  location for audio buffer
*/
/* GMHMM_SI_API */ 
/* Now part of CORE API */
/*TIesrEngineStatusType 
OpenSearchEngine(void *); 
*/
/* search one frame:
** Perform search (recognition) function for one frame, given in the argument.
*/
/* GMHMM_SI_API */
/*TIesrEngineStatusType CallSearchEngine(short [], void *);*/
TIesrEngineStatusType CallSearchEngine(short [], gmhmm_type *, unsigned int);

/* back tracking search space: 
** Find the best sequence of word index.
*/
/* GMHMM_SI_API */
/*TIesrEngineStatusType CloseSearchEngine(TIesrEngineStatusType, void *);*/
TIesrEngineStatusType CloseSearchEngine(TIesrEngineStatusType, gmhmm_type *);

/* speech activity detected: 
*/
/* GMHMM_SI_API */ 
/* Now part of CORE API 
short SpeechDetected(void *);
*/

/* utterance end detected: 
** Indicate (using utterance detection information) if the utterance has ended.
*/
/* GMHMM_SI_API */ 
/* Now part of CORE API
short SpeechEnded(void *);
*/

/*
** the following functions access search related quantities
*/

/* frames fed into search engine, used in search.c */
/* GMHMM_SI_API */ 
/* Now part of CORE API 
short GetFrameCount(void *);  
*/

/* GMHMM_SI_API */ 
/* Now part of CORE API
long GetTotalScore(void *);  
*/

/*  nbr of times the whole model set has been compensated */
/* GMHMM_SI_API */ 
/* Now part of CORE API
unsigned short  GetCycleCount(void *); 
*/

/* index of words in the recognized string, in reversed time order */
/* GMHMM_SI_API */
/* Now part of CORE API 
 unsigned short *GetAnswerIndex(void *);
*/

/* i-th word in the recognized string */
/* GMHMM_SI_API */
/* Now part of CORE API 
const char *GetAnswerWord(unsigned short, void *);
*/

/* number of words in the recognized string        */
/* GMHMM_SI_API */
/* Now part of CORE API 
unsigned short GetAnswerCount(void const *) ; 
*/

/* actual search space 
 */
/*unsigned short GetSearchMemorySize(void *);*/
unsigned short GetSearchMemorySize(gmhmm_type *);


/* Maximum number of MFCC frames */
unsigned short GetMaxFrames(gmhmm_type *gv );


//void GetSearchInfo(void *);

/* indication of the volume setting */
/* GMHMM_SI_API */
/* Now part of CORE API 
short GetVolumeStatus(void *);
*/

/* GMHMM_SI_API */ 
/* Now all of this is part of COREAPI */
/*void SetTIesrPrune(void *data, short);*/
/* GMHMM_SI_API */ 
/*void GetTIesrPrune(void *data, short *);*/
/* GMHMM_SI_API */
/* void SetTIesrTransiWeight(void *data, short); */
/* GMHMM_SI_API */ 
/*void GetTIesrTransiWeight(void *data, short*);*/


/*
** reset SAD control parameters
*/
/*
Th_SPEECH_DELTA = 10   // larger -> more robust
Th_MIN_SPEECH_DB = 50  //  min dB level to be considered as speech 
Th_MIN_BEG_FRM = 5     // larger  -> more robust to  noise pulses 
Th_MIN_END_FRM = 30    // smaller (.e.g. 30) --> quicker latency  time 
Th_NOISE_FLR = 0       // larger -> more robust
*/
/* GMHMM_SI_API */ 
/* Now part of CORE API */
/*void SetTIesrSAD(void *, short, short, short, short, short);*/
/* GMHMM_SI_API */
/*void GetTIesrSAD(void *, short*, short*, short*, short*, short*);*/

/*
** set the number of mean vectors to adapt during each frame 
** PC: 300, DSP 75, Compal 45 
*/

/* GMHMM_SI_API */ 
/* Now in jac_one_user.h */
/*void SetTIesrJacRate(void *, short);*/
/*void SetTIesrJacRate(gmhmm_type *, short);*/
/* GMHMM_SI_API */
/*void GetTIesrJacRate(void *, short*);*/
/*void GetTIesrJacRate(gmhmm_type *, short*);*/

/* check audio volume range (typical: [1200, 25000]) */

/* GMHMM_SI_API */
/* Now part of CORE API */
/* void SetTIesrVolRange(void *, unsigned short, unsigned short);*/
/* GMHMM_SI_API */ 
/*void GetTIesrVolRange(void *, unsigned short*, unsigned short*);*/

