/*=======================================================================

 *
 * confidence_user.h
 *
 * Header for confidence functionality interface.
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
 
======================================================================*/

#ifndef CONFIDENCE_USER_H
#define CONFIDENCE_USER_H

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

/*--------------------------------------------------------*/

#include "tiesrcommonmacros.h"
#include "status.h"
#include "gmhmm_type.h"
#include "tiesr_config.h"
#include "search_user.h"

/* should not include for users */
/* #include "confidence.h" */ 

/* Defaults for confidence parameters. Should be in confidence.h and there
 should be a function cm_set_default. */

/* the number of N-bests that shows that the utterance has low confidence */
#define CM_NBR_NBEST_LOW_CONF 20

#ifdef USE_CONF_DRV_ADP
/* the minimum value of confidence score for adaptation */
#define CM_ADP_THRE -200
#endif

/* Include the confidence structures */

typedef struct{
  short mean; /* Q11 */
  short inv ; /* Q9 */
  short gconst; /* Q6 */
}CM_hypo;

#define CM_SIZE 1300
typedef struct{
  short  base_mem[CM_SIZE];
  ushort  mem_count ;

  ushort *answers; //[CM_MAX_WORD]; /* recognized words */
  ushort *uStp; //[CM_MAX_WORD]; /* word-ending frame index */
  ushort nWords; /* number of words */

  short nS; /* number of segments */

  short *cm_scr; /* the confidence measure
		    [0] : raw log-likelihood distance of the best path versus the
		          accumulated score of the maximum likelihood score along speech frames,
		    [1] : non-stationary SNR, refer to AT&T paper
		    [2] : likelihood ratio of the two hypothesis log(P(X|H_0)) - log(P(X|H_1))
		    [3] : logistic regression score, P(H_0|X) ,
		    the final confidence score is cm_scr[2]*cm_scr[3], which is the product
		        of the scores = p(H_0|X)*[ log(P(X|H_0)) - log(P(X|H_1)) ] */

  Boolean bNeedUpdate; /* TRUE if the anti score needs to be updated */
  short *cm_anti_scr; //[CM_MAX_FRM];

  long cm_fg_scr;
  long cm_bk_scr; /* accumulated max score of speech frames */

  short th_nbest; /* the threshold of number of N-bests to declare low
		     confidence of an uttearnce */

  short nSpeech; /* number of speech frames */
  short nNoise;  /* number of noise frames */
  long speech_power; /* Q6 */
  long noise_power;  /* Q6 */
  long noise_power_sqr; /* Q12 */
  short NSNR; /* non-stationary SNR */
  CM_hypo H0;
  CM_hypo H1;

  short *lgBeta; //[CM_NBR_BETA];
  long *acc_dif2_beta; //[CM_NBR_BETA];

  Boolean bCalced; /* calculated confidence score */

#ifdef USE_CONF_DRV_ADP
  /* for confidence driven unsupervised adaptation */
  short *sCH; /* channel distortion estimate backup */
  long * lAccum;
  short adp_th; /* the threshold for adaptation */
#ifdef USE_SVA
  short * sLogVarRho;
  long * lVarFisherIM;
#endif

#endif

}ConfType;


/* open space for confidence measure calculation */
TIESRENGINECOREAPI_API TIesrEngineStatusType cm_open( ConfType * , gmhmm_type * );

/* The parameters for confidence-driven adaptation
   @param th_adp, the threshold for unsupervised adaptation, default = CM_ADP_THRE, which is
   -200.
   @param th_nbest, the threshold of number of N-bests to declare low 
		     confidence of an uttearnce 
		     default is 20
*/
TIESRENGINECOREAPI_API void cm_set_param( short th_adp,
		   short th_nbest,
		   gmhmm_type * gv );

/* initialize confidence measure for one utterance */
void cm_init(gmhmm_type * gv);

/*-----------------------------------------------------------------------------
 cm_load

 Load statistical and update parameters for the confidence measure if the
 JAC state file pointer exists. Otherwise initialize the parameters to default
 values.

 Arguments:
 fp: File pointer to open JAC state file or NULL
 gv: Pointer to ASR instance structure

--------------------------------- */
TIESRENGINECOREAPI_API void cm_load(FILE*fp, gmhmm_type *gv);

/* save updated parameter for confidence measure computation */
TIESRENGINECOREAPI_API void cm_save(FILE *fp,
	     gmhmm_type* gv);

/* return confidence score */
short cm_score(gmhmm_type *gv);

/* accumulate statistics */
TIESRENGINECOREAPI_API void cm_acc_seg_ratio_to_antimodel(gmhmm_type *gv,
        ushort hmm_code, 
        short stt, short stp,
        long scaling);

/* close confidence measure space */
TIESRENGINECOREAPI_API void cm_close( gmhmm_type* gv );

/* freeze background scores */
TIESRENGINECOREAPI_API void cm_freeze(ConfType* pconf);

/* get the address to the anti score at frame $frm_cnt */
short * cm_antiscr( ConfType* pconf, ushort frm_cnt );

/* compute confidence score and update confidence measure */
TIESRENGINECOREAPI_API void cm_update_status(gmhmm_type *gv);


/* --------------------- used for confidence-driven adaptation ------------------*/

/* backup channel and SBC parameters */
TIESRENGINECOREAPI_API void cm_backup_adp( gmhmm_type * gv);

/* restore channel and SBC parameters */
TIESRENGINECOREAPI_API void cm_restore_adp(gmhmm_type * gv);

/* ------------------------------------------------------------------------------*/

#endif
