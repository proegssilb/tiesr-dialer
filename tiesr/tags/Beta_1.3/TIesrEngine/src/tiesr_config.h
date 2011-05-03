/*=======================================================================

 *
 * tiesr_config.h
 *
 * Header to specify TIesr configuration of options.
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

 This header file is used to define global flags that describe the
 configuration used when a version of the TIesr APIs is built.  This
 is a central repository for all flags that affect the compilation of
 the TIesr APIs and thus makes it easy to change the configuration
 without having to hunt down exactly where each configuration flag is
 defined.  This header should be included as the first item in any
 source file.

======================================================================*/

#ifndef TIESR_CONFIG_H
#define TIESR_CONFIG_H

/* Live mode feature and JAC processing is desired.  To imitate
   file mode processing, comment out the line below*/
#define LIVEMODE

/* In file mode, FILEMODEWAIT determines how many frames are used
   to compute the JAC noise estimate. */
#define FILEMODEWAIT 10


/*--------------------------------*/
/* Use of live audio devices or file-mode processing */
/* Also allows printing errors if not using audio */
#define USE_AUDIO

#ifndef USE_AUDIO 
#define PRT_ERR(any_) any_
#else
#define PRT_ERR(any_)  
#endif


/*--------------------------------*/
/* If PRT allows "command" to occur, then printing of HLR 
   intermediate result information is allowed */
#define PRT(command) /* command */



/*--------------------------------*/
/* Size of model vectors */

/* TIesr Engine will store and use byte mean vectors */
#define BIT8MEAN

/* TIesr Engine will store and use byte inverse variance vectors */
// #define BIT8VAR

/* TIesr Engine will utilize 8 bit feature vectors for doing
   adaptation.  At this time this is the only method supported.  Work
   needs to be done on TIesr in order to use short feature vectors,
   since at this time it means that static buffers need to be
   allocated. */
#define BIT8FEAT

/* Compact (using 8bits) representation of mean and variance is in the
   model binary files that the TIesr Engine will load.  This
   configuration parameter is now defunct.  It is replaced with a
   config.bin file that specifies the format of the mean and inverse
   variance parameters in the input mu.bin and var.bin files. */
//#define COMPACT


/*--------------------------------*/
/* Utilize SVA for variance adaptation of models */

#define USE_SVA

/* Qifeng's improved ISVA */
/* Presently the SVA START_DIMENSION is 1, but further improvements in ISVA
   may result in changing it back to 0. This research is not conclusive yet. */
// #define ISVA 

/* Qifeng's Gaussian cache */
#define USE_GAUSSIAN_CACHE

/*--------------------------------*/
/* Defining the following macro enables IJAC for noisy speech
   recognition */

#define USE_IJAC

/*--------------------------------*/
/* Defining the following macro enables weighted SVA */
#define USE_WSVA

/*--------------------------------*/
/* Defining the following macro to enables SBC for noisy speech 
   recognition */

#define USE_SBC

/*--------------------------------*/
/* Smooth the SBC estimate (also eliminates overflow problem) */

#define SBC_PRIOR

/*--------------------------------*/
/* Not sure of the use of REC.  Not used for live TIESR. */

/* #define REC */

/*--------------------------------*/
/* fast DCT */
//#define USE_FAST_DCT

/*--------------------------------*/
/* Dynamic Prunning 
enable the dynamic pruning (the hmm end pruning is always enabled)
*/
#define DYNAMIC_PRUNE

// Turn on each part of dynamic pruning individually by its flag
#define DYNAMIC_PRUNE_RAMP
#define DYNAMIC_PRUNE_BEAM
#define DYNAMIC_PRUNE_SUBLINEAR
#define DYNAMIC_PRUNE_DEPTH


/* ----- Spectral Subtraction Support ------------------
   Berouti-style noise supression (ETSI VAD is used inside)
   ----------------------------------------------------- */
#define USE_SNR_SS 

/*---------- Cluster-dependent JAC --------------------- 
 Rapid JAC via offline vector-quantiziation of HMM mean vectors 
  ------------------------------------------------------*/
#define RAPID_JAC

/*-------------------------------------------------------
    Offline vector-quantiziation of HMM mean vectors 

 -------------------- NOTICE -----------------------------
 |   Need to load ROM2cls.bin and cls_centr.bin          |
 |   ROM2cls.bin : ROM mean index to cluster mapping     |
 |   cls_center.bin : 20-dimenional centroid             |
 --------------------------------------------------------
*/
#define OFFLINE_CLS

/* ---------------------------------------------- 
   Decoder uses 16-bit operation
   Packing/unpacking is not used 
   ---------------------------------------------*/
#define USE_16BITMEAN_DECOD
#if defined(USE_16BITMEAN_DECOD)
#undef BIT8MEAN
#endif

/* ----------------------------------------------
   Enable Gaussian selection. 
   ----------------------------------------------*/
#define USE_GAUSS_SELECT  

/* ------------------------------------------------------- 
   Enables prunning with phone-level score. The phone-level score is the maximum
   triphone score for a given center phone. Prunning threshold is selected
   relative to a good phone-level score, which in the current setup,
   is the 50% quantile of all phone-level scores. 
   -------------------------------------------------------*/
//#define MODEL_LEVEL_PRUNE
#if defined(MODEL_LEVEL_PRUNE)
#undef DYNAMIC_PRUNE
#endif

/* --------------------------------------------------------
   Enable confidence measure output 
   -------------------------------------------------------*/
#define USE_CONFIDENCE

/* --------------------------------------------------------------------------
   The On-line Reference Modeling Method
   used for two purposes:
   1) GMM-based VAD 
   2) Rescoring of silence model to deal with very non-stationary noise
        such as competing speech.
   ---------------------------------------------------------------------------*/
#if defined(OFFLINE_CLS)&&defined(USE_SNR_SS)
#define USE_ONLINE_REF 
// on-line reference modeling (ORM)
#define USE_ORM_VAD
// method for voice activity detection and robust speech recognition
// End-of-speech detection using the ORM method

#define USE_ORM_PU // update cluster lists of ORM using background statistics
// of the current utterance and the previous utterances. 

#endif


//#define DEV_CLS2PHONE

/* --------------- Confidence-driven unsupervised adaptation -----------------
   Update of adaptation parameters is done only if confidence score (in gmhmm_type->cm_score)
   is above a certain threshold. The threshold is by default set at CM_ADP_THRE (-200)
   --------------------------------------------------------------------------- */
#if defined(USE_CONFIDENCE)
#define USE_CONF_DRV_ADP
#endif

#define REALTIMEGUARD

/* Word-level Backtrace 
   the word-level backtrace is used in the first pass. the normal HMM-level back
   trace is used in the second pass to align utterances to phone levels. 
*/
#define WORDBACKTRACE  // if commented out, HMM based BT will be used.


// The setup below will remove all Gaussian selection and clustering processing

//#undef OFFLINE_CLS
//#undef RAPID_JAC
//#undef USE_GAUSS_SELECT
//#undef USE_ORIG_MEAN
//#undef USE_ONLINE_REF
//#undef USE_ORM_VAD
//#undef USE_ORM_PU

#endif
