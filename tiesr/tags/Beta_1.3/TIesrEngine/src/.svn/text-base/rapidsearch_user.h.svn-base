/*=======================================================================

 *
 * rapidsearch_user.h
 *
 * Header for interface for rapid likelihood calculation.
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
 
 This header exposes the function interface for rapid calculation of
 likelihoods using Gaussian clustering.

======================================================================*/

#ifndef _RAPID_SEARCH_USER_H
#define _RAPID_SEARCH_USER_H


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

/*-----------------------------------------------------------------*/
#include "tiesr_config.h"
#include "search_user.h"
#include "dist_user.h"
#include "gmhmm.h"
#include "sbc_user.h"
#include "load_user.h"
#include "pmc_f_user.h"
#include "rapidsearch.h"

/* --------------- cluster-dependent JAC ------------------------------- */
TIESRENGINECOREAPI_API void rj_noise_compensation(gmhmm_type *cd_hmms);
TIESRENGINECOREAPI_API TIesrEngineJACStatusType rj_compensate(gmhmm_type * gv);

/* @param pFlexModelPath The path saving models for TIesrFlex
   @param perc_core_clusters_q15 precentage of core clusters, value in Q15,
   @param perc_inter_clusters_q15 percentage of intermediate clusters, value in Q15 
   @param gv pointer to decoder
   @param pMem pointer to avaliable memory space 
   return TRUE if success, else return FALSE
*/

TIESRENGINECOREAPI_API TIesrEngineStatusType rj_open(ushort u_perc_cor,
			      ushort u_perc_inte, 
			      gmhmm_type *gv );


TIESRENGINECOREAPI_API void rj_close(gmhmm_type * gv);

TIESRENGINECOREAPI_API short rj_clear_obs_scr(gmhmm_type * gv, short *mfcc_feat);


TIESRENGINECOREAPI_API TIesrEngineStatusType rj_load(gmhmm_type * gv, FILE* pf);
TIESRENGINECOREAPI_API TIesrEngineStatusType rj_save(gmhmm_type * gv, FILE* pf);

short rj_gauss_obs_score_f(short *feature, int pdf_idx, gmhmm_type*gv, long * l_cnter, short * iargmix);

void rj_init(gmhmm_type *gv);

/* @param perc_core_clusters_q15 precentage of core clusters, value in Q15,
   @param perc_inter_clusters_q15 percentage of intermediate clusters, value in Q15 
   return TRUE if sucess, FALSE if can not meet the requirement
*/
Boolean rj_set_param( ushort perc_core_clusters_q15,
		   ushort perc_inter_clusters_q15, 
		   gmhmm_type * gv );

/* setup parameters for online reference modeling, VAD and EOS detection
   @param num_frms_gbg: number of begining frames to construct gabage model 
   @param noise_level : the noise level threshold to choose an LLR threshold
   @param th_llr_low_noise: the LLR threshold in low noise level
   @param th_llr_high_noise: the LLR threshold in high noise level 
   @param ctn_th : number of frames that continuously below a threshold
   @param llr_perc_th : the threshold is computed as $llr_perc_th * max_llr, where max_llr
   is the score of the best matched cluster 
   @param start_from : don't do EOS decision making untill $start_from of frames have
   passed the peak of LLR 
   @param nbr_frm_after_peak, the threshold which declares end of speech if more than 
   then number of frames have been processsed after the peak of LLR has been reached 
   ever since. 
   return TRUE if sucess
*/

TIESRENGINECOREAPI_API Boolean rj_set_ORM(short num_frms_gbg,
		   short noise_level,
		   short th_llr_low_noise, short th_llr_high_noise,
		   short cnt_th, ushort llr_perc_th, short start_from, 
		   short nbr_frm_after_peak, gmhmm_type * gv);


/* The function conducts two operations: 
   1) if the current frame number is smaller than a threshold,
   it records the best matched Gaussian cluster and put it to construct a reference model
   2) else, it updates score of the online reference model 
   Also, it computes the log-likelihood ratio of the best matched cluster w.r.t.
   the reference model
*/
TIESRENGINECOREAPI_API void rj_update_noise(gmhmm_type * gv, short * mfcc_feat, short imax);

/* if SAD is not confirmed, buffering the input speech and return NULL,
   if SAD is confirmed, then pop up buffered speech and synchronise to the current frame
*/
TIESRENGINECOREAPI_API short* rj_vad_proc(gmhmm_type * gv, short * mfcc_feat);

/* ---------------------------------------------------------------------- */


TIESRENGINECOREAPI_API void mlp_open(gmhmm_type* gv);
TIESRENGINECOREAPI_API void mlp_close(gmhmm_type * gv);

void mlp_init(gmhmm_type *gv);
void mlp_reset(gmhmm_type* gv);
void mlp_update(gmhmm_type* gv, short hmm_idx, long score);

/* calculate pruning threshold 
   @param gv pointer to decoder
   @param th_default the pruning threshold by default beam-width pruning 
   @param beam_width beam width for pruning
   return pruning threshold
 */
short mlp_threshold(gmhmm_type *gv , long th_default , short beam_width);

#ifdef USE_AVL_TREE

void avl_MakeEmpty( TreeSpaceType* );
Position avl_Find( ElementType X, AvlTree T, TreeSpaceType * );
Position avl_FindMin( AvlTree T , TreeSpaceType* pTree);
Position avl_FindMax( AvlTree T , TreeSpaceType* pTree);
short avl_Insert( ElementType X, ElementType Val, short T_idx , short elem_size_in_short, 
		  TreeSpaceType * pTreeSpace,
		  TIesrEngineStatusType * status);
ElementType avl_Retrieve( Position P );
#endif

#endif
