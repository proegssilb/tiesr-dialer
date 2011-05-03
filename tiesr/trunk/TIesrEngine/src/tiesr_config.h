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

/*--------------------------------------------------------------------
 Process data as if it is live data.  Live mode feature and JAC processing
 is desired. This should always be used. Not defining this is deprecated.
 ---------------------------------------------------------------------*/
#define LIVEMODE

/*---------------------------------------------------------------------
 In file mode, FILEMODEWAIT determines how many frames are used
   to compute the JAC noise estimate. This is deprecated.
 ---------------------------------------------------------------------*/
#ifndef LIVEMODE
#define FILEMODEWAIT 10
#endif

/*---------------------------------------------------------------------
 Use of live audio devices or file-mode processing.  Also allows
 printing errors if not using audio. This should always be set.
 Not setting this is deprecated.
 ---------------------------------------------------------------------*/
#define USE_AUDIO

#ifndef USE_AUDIO 
#define PRT_ERR(any_) any_
#else
#define PRT_ERR(any_)  
#endif


/*------------------------------------------------------------
 If PRT allows "command" to occur, then printing of HLR 
 intermediate result information is allowed
 -------------------------------------------------------------*/
#define PRT(command) /* command */



/*-----------------------------------------------------------------
 Size of model vectors.
 
 Model vectors may either be 16-bit or byte. If vectors are byte,
 then static and delta parameters are interleaved.
 ------------------------------------------------------------------*/

/* TIesr Engine will store and use byte mean vectors if defined. */
#define BIT8MEAN

/* TIesr Engine will store and use byte inverse variance vectors if defined */
// #define BIT8VAR

/* TIesr Engine will utilize byte feature vectors for doing adaptation. */
#define BIT8FEAT

/* Compact (using 8bits) representation of mean and variance is in the
   model binary files that the TIesr Engine will load.  This
   configuration parameter is now defunct.  It is replaced with a
   config.bin file that specifies the format of the mean and inverse
   variance parameters in the input mu.bin and var.bin files. */
//#define COMPACT



/*-------------------------------------------------------------------
Utilize SVA for variance adaptation of models. Comment out to disable.
 -------------------------------------------------------------------*/

#define USE_SVA

/*--------------------------------------------------------------------
 Defining the following macro enables weighted SVA. Implies USE_SVA.
 Comment out to disable.
 --------------------------------------------------------------------*/
#define USE_WSVA

#if defined( USE_WSVA )
#define USE_SVA
#endif


/*------------------------------------------------------------------------
  Improved SVA. Presently the SVA START_DIMENSION is 1, but further
 improvements in ISVA may result in changing it back to 0. This research
 is not conclusive yet. Comment out to disable.
 ------------------------------------------------------------------------*/
// #define ISVA 


/*------------------------------------------------------------------------
  Use Gaussian cache to reduce calculation of likelihoods.
 ------------------------------------------------------------------------*/
#define USE_GAUSSIAN_CACHE

/*------------------------------------------------------------------------
 Improved JAC calculations for channel estimate. Comment out to disable.
 ------------------------------------------------------------------------ */
#define USE_IJAC


/*-------------------------------------------------------------------------
 Stochastic Bias Compensation for noisy speech recognition. SBC calculates
 additional model mean biases to be applied based on hierarchical phone tree.
 Comment out to disable.
 -------------------------------------------------------------------------*/
#define USE_SBC

/* Smooth the SBC estimates. Required for SBC. */
#ifdef USE_SBC
#define SBC_PRIOR
#endif



/*-------------------------------------------------------------------
 Kept for historical reference in code. Now defunct.
 -------------------------------------------------------------------*/
/*#define REC */


/*---------------------------------------------------------------------
 Use fast DCT. A fast DCT is only available for a few specific feature
 sizes. Comment out do disable.
 ----------------------------------------------------------------------*/
//#define USE_FAST_DCT

/*---------------------------------------------------------------------
 Dynamic Pruning. Several dynamic pruning heuristics are applied.
 The hmm end pruning is always enabled. Comment out to disable.

--------------------------------------------------------------------- */
#define DYNAMIC_PRUNE

/* Turn on each part of dynamic pruning individually by its flag */
#define DYNAMIC_PRUNE_RAMP
#ifdef DYNAMIC_PRUNE_RAMP
#define DYNAMIC_PRUNE
#endif

#define DYNAMIC_PRUNE_BEAM
#ifdef DYNAMIC_PRUNE_BEAM
#define DYNAMIC_PRUNE
#endif

#define DYNAMIC_PRUNE_SUBLINEAR
#ifdef DYNAMIC_PRUNE_SUBLINEAR
#define DYNAMIC_PRUNE
#endif

#define DYNAMIC_PRUNE_DEPTH
#ifdef DYNAMIC_PRUNE_DEPTH
#define DYNAMIC_PRUNE
#endif


/* -----------------------------------------------------------------
 Spectral Subtraction Support. Berouti-style noise supression applied to
 incoming frames of data prior to converting to MFCC. Comment out to disable.
 ------------------------------------------------------------------ */
#define USE_SNR_SS 


/*-------------------------------------------------------
 Offline vector-quantiziation of HMM mean vectors available
 in data files. Comment-out to disable.

 Requires VQ cluster information data in the files ROM2cls.bin
 and cls_centr.bin.  ROM2cls.bin holds mean index to cluster mapping,
 and cls_center.bin holds centroid vectors.
 ----------------------------------------------------------*/
#define OFFLINE_CLS


/*--------------------------------------------------------------
 Rapid JAC via offline vector-quantiziation of HMM mean vectors.
 Reduces computation of bias data. Requires VQ cluster information.
 Comment-out to disable.
  ------------------------------------------------------*/
#define RAPID_JAC
#ifdef RAPID_JAC
#define OFFLINE_CLS
#define USE_SVA
#endif

/* Rapid JAC decoder uses 16-bit operations. Packing/unpacking is not used */
 #ifdef RAPID_JAC
#define USE_16BITMEAN_DECOD
#undef BIT8MEAN
#endif


/*---------------------------------------------------------------------
 VQ-based Gaussian selection. Allows rapid evaluation of likelihoods
 by using only VQ cluster or global likelihoods where possible. Requires
 VQ cluster information. Comment-out to disable.
   --------------------------------------------------------------------*/
#define USE_GAUSS_SELECT
#ifdef USE_GAUSS_SELECT /* && defined( RAPID_JAC ) */
#define OFFLINE_CLS
#endif

/*---------------------------------------------------------------------
 Enables prunning with phone-level score. The phone-level score is the maximum
 triphone score for a given center phone. Pruning threshold is selected
 relative to a good phone-level score, which in the current setup,
 is the 50% quantile of all phone-level scores. 
 -------------------------------------------------------------------------*/
//#define MODEL_LEVEL_PRUNE
#if defined(MODEL_LEVEL_PRUNE)
#undef DYNAMIC_PRUNE
#endif


/*--------------------------------------------------------------------------
 VQ-based On-line Reference Modeling Methods. Requires VQ information.
 Used for two purposes:
 1) GMM-based VAD 
 2) Rescoring of silence model to deal with very non-stationary noise
    such as competing speech.
   ---------------------------------------------------------------------------*/

/* On-line reference modeling (ORM). Dynamically defines a subset of
the clustered model distributions to detect non-speech frames
Comment out to disable */

#define USE_ONLINE_REF
#ifdef USE_ONLINE_REF
#define OFFLINE_CLS
#define USE_SNR_SS
#endif

/* Method for voice activity detection and robust speech recognition.
 End-of-speech detection using the ORM method. Comment-out to disable. */

#define USE_ORM_VAD
#ifdef USE_ORM_VAD
#define USE_ONLINE_REF
#define OFFLINE_CLS
#define USE_SNR_SS
#endif

/* Update cluster lists of ORM using background statistics of the current
 utterance and the previous utterances. Comment-out to disable. */

#define USE_ORM_PU
#ifdef USE_ORM_PU
#define USE_ONLINE_REF
#define OFFLINE_CLS
#define USE_SNR_SS
#define USE_ORM_VAD
#endif



//#define DEV_CLS2PHONE


/*--------------------------------------------------------
   Enable confidence measure output
  -------------------------------------------------------*/
#define USE_CONFIDENCE


/* Confidence-driven unsupervised adaptation.  Update of adaptation
 parameters is done only if confidence score (in gmhmm_type->cm_score)
 is above a certain threshold. The threshold is by default set at
 CM_ADP_THRE (-200). Comment out to disable. */
 
#define USE_CONF_DRV_ADP
#ifdef USE_CONF_DRV_ADP
#define USE_CONFIDENCE 
#endif

/*-----------------------------------------------------------------------
 Real time guard monitors how many frames are queued for recognition. If the
 number of frames back-logged gets large, then the pruning parameter will be
 adjusted to exclude more hypotheses in an attempt to maintain real-time.
 Comment out to disable.
 --------------------------------------------------------------------------*/
#define REALTIMEGUARD

/*----------------------------------------------------------------------
 Word-level Backtrace.  The word-level backtrace is used in the first pass.
 The normal HMM-level back trace is used in the second pass to align
 utterances to phone levels. Research on multi-pass recognition is ongoing,
 so currently this should be defined.  If commented out, HMM based BT
 will be used.
 ------------------------------------------------------------------------*/
#define WORDBACKTRACE

#endif
