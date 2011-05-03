/*=======================================================================
 confidence.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
 
======================================================================*/

#ifndef _CONFIDENCE_H
#define _CONFIDENCE_H

#include "tiesr_config.h"

#define CM_UPDATE_MEAN /* online update of mean of the hypothesis models */
//#define CM_UPDATE_VAR /* online update of variance of the hypothesis models */

typedef struct{
  short mean; /* Q11 */
  short inv ; /* Q9 */
  short gconst; /* Q6 */
}CM_hypo; 

#define CM_MAX_FRM 500
#define CM_MAX_WORD 10
#define CM_SCRS 5 /* number of computed measures for confidence measure */

#define  CM_NBR_BETA 3

const short CM_NBEST_LOW_CONF = -50;
/* the confidence score for those utterances have too many N-bests */

#define CM_SIZE 900
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

#ifdef USE_CONFIDENCE

/* ----------------------- trained UV model parameters -----------------------
   CM_UV_ACC and CM_UV_REJ are mean, inv, and gconst of the acceptance/rejection models
   for the feature of LLR between the best path and the accumulated maximum likelihood 
   score. LLR is on speech segments. 
   [mean, inv, gconst] with Q11, Q9, and Q6; 
*/
const short CM_UV_ACC[3] = {-4685, 128, 206}; /* trained from OOV, IV in 8-types of Aurora noise,
						 TIMIT noise in 10dB, and WAVES noise. */
//const short CM_UV_ACC[3] = {-4189, 166, 190}; // trained from OOV in WAVES noise 
const short CM_UV_REJ[3] = {-9793, 5, 412}; /* trained from OOV, IV in 8-types of Aurora noise,
					       TIMIT noise in 10dB, and WAVES noise. */
//const short CM_UV_REJ[3] = {-9120, 86, 231};   // trained from OOV in WAVES noise

/* ---------------------- logistic regression ------------------------------
   train UV with logistic regression
   1 / (1 + exp(-W X) 
   where W = [w0, w1, w2], and X = [1, cm[0], cm[1]]
*/
const short CM_LOGIST_PARA[CM_NBR_BETA] = {377, 62, -18};  
/* trained from OOV, IV in 8-types of Aurora noise, TIMIT noise in 10dB, and WAVES noise. */
//const short CM_LOGIST_PARA[CM_NBR_BETA] = {296, 64, -97}; // triained from OOV in WAVES noise

#ifdef USE_CONF_DRV_ADP
const short CM_ADP_THRE = -200; /* the minimum value of confidence score for adaptation */
//const short CM_ADP_THRE = -1000; /* the minimum value of confidence score for adaptation */
#endif

#endif

#endif
