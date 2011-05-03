/*=======================================================================

 *
 * status.h
 *
 * Header defining TIesr status values.
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

 This header defines the typedefs and enums that make up the status
 that is reported by the TIesr Engine APIs.

======================================================================*/

#ifndef  _STATUS_H
#define  _STATUS_H

/*
** returned value from recognizer's search engine         
*/

typedef enum {
  /*
  ** common for SI and SD
  */
       eTIesrEngineSuccess = 100,            
       eTIesrEngineEvalCellOverflow,
       eTIesrEngineBeamCellOverflow,       
       eTIesrEngineTimeCellOverflow,       

       eTIesrEngineSearchSpace = 200,     /* max_beam_cell set to zero */
       eTIesrEngineSearchMemoryLimit,     /* search memory specified too large for engine (limit for shorts) */
       eTIesrEngineSearchMemorySize,      /* search memory specified too small for engine */
       eTIesrEngineVQHMMMemorySize,      
       /* memory for VQ-HMM is too small, need to adjust VQHMM_SIZE in rapidsearch.h */
       eTIesrEngineVQHMMGarbage,          
       /* requested number of begining frames are too large  */ 
       eTIesrEngineVQHMMSave, 
       /* save VQHMM failed */
       eTIesrEngineVQHMMLoad, 
       /* load VQHMM failed */

       /* TIesr JAC compensation failed */
       eTIesrEngineCompFail,

       eTIesrEngineFeatureMemoryOut = 300,/* feature speech vector count out of MAX_FRM */
       eTIesrEngineSegmentMemoryOut,      /* max nbr of segments overflow */
       eTIesrEngineMuSpaceMemoryOut,      /* search memory specified smaller than mem_block space */
       eTIesrEngineFeatureFrameOut,       /* mfcc memory specified too small */
       eTIesrEngineAnswerMemoryOut,       /* MAX_WORD_UTT too small */
       eTIesrEngineTimeOut,               /* recognition time out */
       eTIesrEngineNBestMem,              /* Not enough space to hold N-best information */

       eTIesrEngineModelLoadFail = 400,   /* fail to load recognition models */
       eTIesrEngineAlignmentFail,
       eTIesrEngineMemorySizeFail,

       eTIesrEngineFailedJAC = 425,

       
/*
**  specific to SDSR:
*/
       eTIesrEngineFeatureDimensionFail = 450, /* gtm model size mismatch */
       eTIesrEngineSegmentSizeCheckFail,       /* SEG cap_in_check failure */
       eTIesrEngineModelFileFail,              /* model file open fail (nbr mfcc file) */
       eTIesrEngineSaveFail,                   /* fail to save model on disk */
       eTIesrEngineEnrollLoadFail,             /* problem in loading utterances in enrollment */
       eTIesrEngineBackgoundCreationFail,      /* failure to calculate background clustered model */     

       eTIesrEngineStateMemoryOut = 500,       /* max size for states reached */
       eTIesrEngineSearchMemoryOut,            /* max size for search space reached */

       eTIesrEngineModelMemorySize = 600,      /* gtm memory size */
       eTIesrEngineFeatureMemorySize,          /* observation memory */
       eTIesrEngineNetworkMemorySize,          /* total network memory */
       eTIesrEngineModelSize,                  /* model size in SD reco */
       eTIesrEngineBackTrackMemorySize,        /* back-tracking in SD ENRO */
       eTIesrEngineCMMemorySize, /* memory for confidence measure is too small */
       eTIesrEngineVALTreeMemorySize, /* memory for VAL BST is too small */
       eTIesrEngineSym2PosMapSize,   /* memory for mapping symbol to start state index is too small */

       eTIesrEngineShortUtterance = 700,       /* utterance (excluding silence) contains too few frames */

       eTIesrTTSLoadHMM2Phone = 800,           /* HMM2phone mapping can not be loaded */
       eTIesrTTSNoSilencePhone = 801,          /* no silence phone is found in the recognition network */
       eTIesrTTSLoadPhone2Voicing = 802,       /* can not load phone 2 voicing */
       eTIesrTTSLoadPhoneDuration = 803,       /* can not load phone durationo */
       eTIesrTTSPhoneMismatch = 804,           /* mismatch of phone indices */
       eTIesrTTSPhoneMissWord = 805,           /* can not locate a proper word */
       eTIesrTTSPhoneDuration = 806,           /* no phone duration information */
       eTIesrTTSPhoneWord = 807,               /* can not find word information */
       eTIesrTTSSaveTTSMean = 808,             /* can not save TTS MFCC mean vectors */
       eTIesrTTSSaveVoicing = 809,             /* can not save TTS voicing */
       eTIesrTTSSaveDuration = 810,            /* can not save duration */
       eTIesrTTSTTS = 811,                     /* TTS data structure does not exist */
       eTIesrTTSPhoneList = 812,               /* phone list data is not in TTS data structure */
       eTIesrTTSNumPhones = 813,               /* mismatch of number of phones in phone list and 
						 pPhone2Voicing */
       eTIesrTTSNumWords = 814,                /* mismatch of number of words in nNumWords and 
						  the autual none NULL list in pWords */
       eTIesrTTSOutputFile = 815,              /* can not open output binary file to save synthesized voice */
       eTIesrTTSInputMFCC = 816,               /* can not read MFCC mean vectors */
       eTIesrTTSInputDuration = 817,           /* can not read duration information */
       eTIesrTTSInputVoicing = 818,            /* can not read voicing information */
       
       eTIesrTTSNotFound = 819,                /* word sequence is not found in the recognition network */
       eTIesrTTSOutputMeanSize = 820,          /* output mean vector size is too large, check whether the space for the mean expansion is sufficient */
       eTIesrTTSOutput = 821              /* can not allocate space for output voice signals */
} TIesrEngineStatusType;

/*
** status for JAC
*/
typedef enum {
       eTIesrEngineJACSuccess,            
       eTIesrEngineJACMemoryFail,         /* lack of memory */
       eTIesrEngineJACLoadFail,
       eTIesrEngineJACReset,              /* reset JAC and other parameters because JAC_load is failed */
       eTIesrEngineJACSaveFail,
       eTIesrEngineJACSBCInitFail,
       eTIesrEngineJACSBCLoadFail,
       eTIesrEngineJACSVAFail,
       eTIesrEngineJACFail,
       eTIesrEngineJACNoAlignment,
       eTIesrEngineJACRJLoadFail
} TIesrEngineJACStatusType;


/*
** volume indication: 
*/

enum {TIesrEngineVolumeLow, TIesrEngineVolumeHigh, TIesrEngineVolumeOK };


#endif /* _STATUS_H */
