/*=======================================================================
 hlr_status.h

 *
 * hlr_status.h
 *
 * Header for HLR status values of the TIesr recognizer.
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

 This header file defines the HLR status return values that will
 be returned by the API.  It also defines the maximum number of
 adaptation utterances that can be collected.

======================================================================*/

#ifndef _HLR_STATUS_H
#define _HLR_STATUS_H


/*--------------------------------*/
/* 
   The maximum number of HLR utterances that an app can collect.
   This number must not be exceeded 
*/

#define HLR_MAX_UTTERANCES 5     /* maximum number of utterances that will be stored for adaptation */


/*--------------------------------*/
/*
** HLR error codes:
*/

typedef enum {
       eTIesrEngineHLRSuccess = 100,            
       eTIesrEngineHLRFail,
/* load/save problem */
       eTIesrEngineHLRTxmSave = 200,     /* fail to save transformation to file */
       eTIesrEngineHLRTxmLoad,           /* fail to load transformation to file */
       eTIesrEngineHLRLoadNbrTerminal,   /* fail to load the number of terminal symbols */
       eTIesrEngineHLRLoadTree,          /* fail to load tree */
       eTIesrEngineHLRPhoneTable,        /* fail to load phone table */
       eTIesrEngineHLRMuFail,            /* fail to write mu file */
/* memory limitation */
       eTIesrEngineHLRMaxNbrUtter = 300, /* max. nbr of utterances limit */
       eTIesrEngineHLRTxmMemory,         /* out of memory for transformation */
       eTIesrEngineHLRMemoryAlloc,       /* fail to mem_alloc */
       eTIesrEngineHLRMemoryBase,        /* base_map size limit reached */
       eTIesrEngineHLRNbrTmnls,          /* too many ternimals  */
       eTIesrEngineHLRAlignmentMemory,   /* alignment memory out */
       eTIesrEngineHLRMaxMeanVectors,    /* max nbr of mean vectors (for estmation) not enough */
       eTIesrEngineHLRMemoryBlock,       /* search memory not large enough to cover transformation */
       eTIesrEngineHLRGlobalMemory,      /* specified memory limit not large enough */
/* others */
       eTIesrEngineHLRDimensionMismatch = 400, /* dimension (model) mismatch */
       eTIesrEngineHLRFailed_Xfm,        /* HLR: failed xfm means */
       eTIesrEngineHLRUtterShort,        /* too short enroll utterance */
       eTIesrEngineHLRTreeTransformFail ,/* fail in tree_transform */
       eTIesrEngineHLRMergeTransfFail ,  /* fail in merge transform */
       eTIesrEngineHLRSuperviseAlignUtt, /* fail in supervison */
       eTIesrEngineHLRBacktraceAligned,  /* fail in alignment */
       eTIesrEngineHLRGetStats,          /* fail to get stats */
} TIesrEngineHLRStatusType;


#endif /* _HLR_STATUS_H */
