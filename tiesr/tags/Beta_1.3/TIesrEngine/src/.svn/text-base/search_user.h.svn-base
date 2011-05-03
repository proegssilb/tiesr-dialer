/*=======================================================================

 *
 * search_user.h
 *
 * Header for interface for HMM Viterbi search.
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

 This header file defines the interface to search functions of the
 TIESR gmhmm search engine.

======================================================================*/

#ifndef SEARCH_USER_H
#define SEARCH_USER_H


/*--------------------------------------------------------------
The Windows method of exporting functions from a DLL.
---------------------------------------------------------------*/
#if defined (WIN32) || defined (WINCE)

/* If it is not defined already, define a macro that does
Windows format export of DLL functions */
#ifndef TIESRENGINECOREAPI_API

// The following ifdef block is the standard way of creating macros which
// make exporting from a DLL simpler. All files within this DLL are compiled
// with the TIESRENGINECOREAPI_EXPORTS symbol defined on the command line.
// This symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see
// TIESRENGINECOREAPI_API functions as being imported from a DLL,
// whereas this DLL sees symbols defined with this macro as being exported.
#ifdef TIESRENGINECOREAPI_EXPORTS
#define TIESRENGINECOREAPI_API __declspec(dllexport)
#else
#define TIESRENGINECOREAPI_API __declspec(dllimport)
#endif
#endif

#else

#ifndef TIESRENGINECOREAPI_API
#define TIESRENGINECOREAPI_API
#endif

#endif
/*--------------------------------------------------------------------*/


#include "tiesrcommonmacros.h"
#include "status.h"
#include "gmhmm_type.h"

//---------------------PRUNING PARAMETERS

// most pruning constraints are for the first 20 frames
#define  PRUNE_FRAME_TH 20   

// time dependant pruning. Note: 20*300 should be smaller to the overall  pruning max, which is,e.g. 7*64*18=8000, 
//This is number is subtracted from the overall pruning max.
#define PRUNE_TIME_FACTOR  300  

// the penalty when #of active beam increase too fast
#define PRUNE_GROW_PENALTY 2000  

//in case the pruning constraints add up, here is a minimal 
#define  PRUNE_MINIMUM  2560

// This is the maximal ratio between the number of words and the number of beams. If above, penalty will be applied on pruning.
#define PRUNE_BEAM_WORD_RATIO 7 //3
// The delta applied for the above threshold, as the second line to tell there is really too many active beam. MOre penalty will be used.
#define PRUNE_BEAM_WORD_DELTA  250

//the two penalty factors for too much active beam
#define  PRUNE_BEAM_WORD_PNT1 4  //11
#define  PRUNE_BEAM_WORD_PNT2 3   //10

//hmm end prune, with 400 more prune
#define SYMEND_PRUNE_DELTA  400
#define WORDEND_PRUNE_DELTA  0
//-----------------------------------------

//word based BT and dymanic memory management
typedef enum DecMode
{
   WORDBT,
   HMMBT,
   HMMFA
} DecMode;


#define BEAM_CELL_LIMIT 3000


/* gaussian observation likelihood type evaluation */
typedef enum ObsType 
{
   VITERBI,
   FULL,
   COMPONENT
} ObsType;


/*--------------------------------*/
/* SymType */
typedef struct {
  unsigned short hmm_code; /* HMM code, not context-expanded */
  unsigned short n_next;   /* the number of symbols in next[]. if bit15 == 1 then the last element in
                              "next[]" is the word code  */
  unsigned short next[1];  /* the list of following symbols, 
			      cannot use short *, more than 1 */
} SymType;


/*--------------------------------*/
/* HmmType */
typedef struct {

  short          tran;    /* offset pointing into the transition matrix pool,
                             NOT converted into real addr in initialization
                          */
  unsigned short pdf_idx[1]; /* each entry is an offset pointing into
				base_pdf[],
				it is the observation distribution for each
				emission state,
				pdf_idx[ n_state - 1 ]
			     */
} HmmType;


typedef struct _SEARCH_MEM{
  short * p_lld_hmm; /* pointer to the log-likelhood score of the HMM */
  short * p_beta_hmm; /* pointer to the backward score of the HMM */
} SearchMemType; 

/*--------------------------------*/
/* Functional interface to search engine */

TIESRENGINECOREAPI_API void set_search_space(short *search_space, ushort beam_z, ushort sym_z,
		      ushort state_z, ushort time_z, gmhmm_type *gv);


TIESRENGINECOREAPI_API short gauss_obs_score_f(short *feature, int pdf_idx, gmhmm_type *gv);


TIESRENGINECOREAPI_API short search_a_frame(short mfcc_feature[], short not_end,
		     unsigned short  frm_cnt, gmhmm_type *gv);


TIESRENGINECOREAPI_API TIesrEngineStatusType back_trace_beam(short symcode, short frm,
				      ushort *hmm_codes, ushort *stt, 
				      ushort *stp, ushort *nbr_seg, 
				      ushort *hmm_delta, gmhmm_type *gv);

/* return the starting address of alpha variable for frame t */
TIESRENGINECOREAPI_API short * EM_alpha_t (short lld[],  /* log likelihood of the segment, lld[j * T + t] for state j frame t */
		    short alpha[],    /* forward variable, size must be 2 * NBR_STATES(hmm) */
		    ushort t,         /* current frame index */
		    ushort T,         /* total number of frames */
		    HmmType *hmm,     /* GTM structure hmm  */
		    short c_t_beta[], /* normalization factor, size T+1, c_t_beta[t] for frame t output */
		    gmhmm_type *gv);


/* return the total log likelihood over the T frames */
TIESRENGINECOREAPI_API long EM_beta(short lld[],     /* log likelihood of the segment, lld[j * T + t] for state j frame t */
	     short beta[],    /* backward variable, beta[j * T + t] for state j frame t */
	     ushort T,        /* total number of frames */
	     HmmType *hmm,    /* GTM structure hmm  */
	     short c_t_beta[],/* normalization factor, size T+1, c_t_beta[t] for frame t output */
	     gmhmm_type *gv);

/* get the address of beta and log-likelihood score */
void EM_get_lld( short s_seg,
		 const short *p_lld, 
		 const short * p_beta, 
		 short T, 
		 SearchMemType * smem, 
		 gmhmm_type * gv);

/* -------------------- utterance-level forward-backward algorithm ----------------------- */
#ifdef DEV_EM

long EM_beta_word(const short lld[],     /* log likelihood of the segment, lld[j * T + t] for state j frame t */
		  short beta[],
		  /* backward variable, beta[j * T + t] for state j frame t */
		  short sz_bkwd, 
		  ushort T,        /* total number of frames */
		  short c_t_beta[],
		  /* normalization factor, size T+1, c_t_beta[t] for frame t output */
		  short * lo_hi_idx , /* start and end points */
		  gmhmm_type *gv); 

/* Forward alpha updating without HMM boundaries */
long EM_alpha_word(short tfrm, 
		   const short lld[],     
		   /* log likelihood of the segment, lld[j * T + t] for state j frame t */
		   short alpha[],    
		   /* backward variable, beta[j * T + t] for state j frame t */
		   ushort T,        /* total number of frames */
		   ushort sz_frwd,  /* size of alpha */
		   short c_t_beta[],
		   /* normalization factor, size T+1, c_t_beta[t] for frame t output */
		   short * lo_hi_idx, /* start and end points */
		   gmhmm_type *gv);

#endif

#endif
