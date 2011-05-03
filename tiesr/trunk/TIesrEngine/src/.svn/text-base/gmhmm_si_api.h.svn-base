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

#include "tiesr_config.h"

#include "status.h"
#include "winlen.h"
#include "gmhmm_type.h"

/*---------------------------------------------------------------------
 OpenASR
 
 Initialize the ASR. Load speech HMM and sentence network,
 initializes parameters, and allocates memory structures.

 Arguments:
 path: Directory containing HMM models and grammar network for recognition
 total_words: Number of shorts in memoryPool
 memoryPool: Pointer to memory, the beginning of which is the gmhmm_type struct.
-----------------------------*/
TIesrEngineStatusType OpenASR(char *path, int total_words, gmhmm_type *memoryPool);


/*-------------------------------------------------------------------
 CloseASR

 Deallocate memory assigned to models and processing blocks and close the ASR.

 Arguments:
 memoryPool: Pointer to memory pool starting with the gmhmm_type struct.
 ------------------------------*/
void CloseASR(gmhmm_type * memoryPool );
 

/*---------------------------------------------------------------------------
 CallSearchEngine
 
 Perform recognition search for one frame.

 Arguments:
 sig_frm: A frame of speech data
 reco: Pointer to the ASR structure holding recognition state information
 aFramesQueued: Number of frames available for searching, used to adjust
 search pruning.

-------------------------------------*/
TIesrEngineStatusType CallSearchEngine(short sig_frm[], gmhmm_type * reco,
        unsigned int aFramesQueued );


/*-----------------------------------------------------------------------
 CloseSearchEngine

 Back track the search space to determine the recognition result and the
 associated best sequence word indices, if a recognition result exists. The
 search may not include a grammar network stop symbol with a non-zero
 probability, in which case there is no recognition result.

 Arguments:
 status: Status of recognizer search
 reco: Pointer to the ASR structure holding recognition state information

------------------------------------*/
TIesrEngineStatusType CloseSearchEngine(TIesrEngineStatusType status,
        gmhmm_type * reco );



/*---------------------------------------------------------------------------
 GetSearchMemorySize

 Get the search space memory utilized for this utterance.

 Arguments:
 reco: Pointer to the ASR structure holding recognition state information

-----------------------------------*/
unsigned short GetSearchMemorySize(gmhmm_type *reco);


/*----------------------------------------------------------------------------
 GetMaxFrames

 Get the maximum number of frames of speech to process with the search
 engine. This does not include initial non-speech frames.

 Arguments:
 reco: Pointer to the ASR structure holding recognition state information
 ----------------------------------*/
unsigned short GetMaxFrames(gmhmm_type *gv );
