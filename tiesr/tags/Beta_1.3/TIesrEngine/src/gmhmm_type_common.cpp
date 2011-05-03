/*=======================================================================

 *
 * gmhmm_type_common.cpp
 *
 * Common functions of the TIesr recognizer for data access.
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


 This code provides common functions that TIesr APIs may
 use to access the gmhmm_type structure.

======================================================================*/

/* Windows DLL API entry point if Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                                       )
{
      switch (ul_reason_for_call)
      {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
              break;
      }
    return TRUE;
}

#endif


#include "tiesr_config.h"
#include "gmhmm_type_common_user.h"
#include "uttdet_user.h"
#include "jac-estm_user.h"
#ifdef USE_CONFIDENCE
#include "confidence_user.h"
#endif
#include "noise_sub_user.h"
#if defined(OFFLINE_CLS) || defined(RAPID_JAC)
#include "rapidsearch_user.h"
#endif

/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short GetFrameCount(const gmhmm_type *gvv)
{
  return   (gvv)->frm_cnt;
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API long GetTotalScore(const gmhmm_type *gvv)
{
  return   (gvv)->best_sym_scr;
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API ushort  GetCycleCount(const gmhmm_type *gvv)
{
  return (gvv)->nbr_cpy;
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API ushort GetAnswerCount(const gmhmm_type *gvv, short nbest_idx)
{
  if (nbest_idx == 0)
    return (gvv)->nbr_ans;
  else{
    if (nbest_idx < gvv->nbest.n_bests){
      return (gvv->nbest.elem[gvv->nbest.best_idx[nbest_idx]].n_ans);
    }else return 0; 
  }
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API const unsigned short *GetAnswerIndex(const gmhmm_type *gvv, short nbest_idx)
{
  if (nbest_idx == 0)
    return (gvv)->answer;
  else{
    if (nbest_idx < gvv->nbest.n_bests){
      return (gvv->nbest.elem[gvv->nbest.best_idx[nbest_idx]].answers);
    }else return NULL; 
  }
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API const char *GetAnswerWord(unsigned short i, const gmhmm_type *gvv, short nbest_idx)
{
  short k; 
   /*gmhmm_type *gv = (gmhmm_type *)gvv; */
  if (nbest_idx == 0)
    return gvv->vocabulary[ gvv->answer[ gvv->nbr_ans - 1 - i ] ];
  else{
    if (nbest_idx < gvv->nbest.n_bests){
      k = gvv->nbest.elem[gvv->nbest.best_idx[nbest_idx]].n_ans;
      return (gvv->vocabulary[gvv->nbest.elem[gvv->nbest.best_idx[nbest_idx]].answers[k - 1 - i]]);
    }else return NULL; 
  }
}


TIESRENGINECOREAPI_API ushort GetNumNbests(const gmhmm_type *gvv)
{
  return gvv->nbest.n_bests;
}

#ifdef USE_NBEST
ushort GetBestWordStartAt(unsigned short i, const gmhmm_type *gv)
{
  return gv->nbr_ans - gv->ptr_nbest_tmp->u_nbest_word_start_at[i]; 
}
#endif

#ifdef _MONOPHONE_NET

ushort GetBestWordStartAt(unsigned short i, const gmhmm_type *gvv)
{
  gmhmm_type * gv = (gmhmm_type*) gvv->pAux; 
  return gv->nbr_ans - gv->ptr_nbest_tmp->u_nbest_word_start_at[i]; 
}
#endif

/*--------------------------------*/
TIESRENGINECOREAPI_API void GetTIesrPrune(const gmhmm_type *gvv, short *value)
{
  *value = (gvv->prune) >> 6;  
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void GetTIesrTransiWeight(const gmhmm_type *gvv, short *value)
{
  *value = (gvv->tranwgt) >> 6; 
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short GetVolumeStatus(const gmhmm_type *gvv)
{
  return (gvv)->vol;
}

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short GetConfidenceScore(const gmhmm_type *gvv)
{
  return (gvv)->cm_score ; 
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void SetTIesrPrune(gmhmm_type *gvv, short value)
{
  (gvv)->prune = value * (1<<6);    /* pruning coefficient */
}


/*-------------------------------*/
TIESRENGINECOREAPI_API void SetTIesrTransiWeight(gmhmm_type *gvv, short value)
{
  (gvv)->tranwgt = (value)*(1<<6);
}



/*--------------------------------*/
TIESRENGINECOREAPI_API void GetTIesrSAD(const gmhmm_type *gvv, short  *a, short  *b, short  *c,
		 short  *d, short  *e)
{
   /*gmhmm_type *gv = (gmhmm_type *) gvv; */

  *a = gvv->th_speech_delta;
  *b = gvv->th_min_speech_db;
  *c = gvv->th_min_beg_frm;
  *d = gvv->th_min_end_frm;
  *e = gvv->th_noise_flr;
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void SetTIesrSAD(gmhmm_type *gvv, short a, short b, short c, short d, short e)
{
   /*gmhmm_type *gv = (gmhmm_type *) gvv; */

  gvv->th_speech_delta = a;
  gvv->th_min_speech_db = b; 
  gvv->th_min_beg_frm = c;
  gvv->th_min_end_frm = d;
  gvv->th_noise_flr = e ;
}

/*--------------------------------
  invoke N-best 
*/
TIESRENGINECOREAPI_API void SetTIesrNBest(gmhmm_type *gvv, Boolean bNbest)
{
  gvv->bUseNBest = bNbest; 
}

TIESRENGINECOREAPI_API void GetTIesrNBest(gmhmm_type *gvv, Boolean *bNbest)
{
  *bNbest = gvv->bUseNBest ;
}

/*--------------------------------
  setup parameters for Gaussian selection 
  @param perc_core_clusters default 16384 
  @param perc_inter_clusters dfault 0 
  all in Q15 format 
 */
TIESRENGINECOREAPI_API void SetTIesrGaussSel( gmhmm_type *gvv,
        ushort perc_cor_clusters,
        ushort perc_inter_clusters )
{
#ifdef USE_GAUSS_SELECT
  rj_set_param( perc_cor_clusters, perc_inter_clusters, gvv); 
#endif
}

/*--------------------------------
  return parameters of Gaussian selection 
  setup parameters for Gaussian selection 
  @param *a = perc_core_clusters default 16384 
  @param *b = perc_inter_clusters dfault 0 
  all in Q15 format 
*/
TIESRENGINECOREAPI_API void GetTIesrGaussSel(const gmhmm_type *gvv, ushort  *a, ushort  *b )
{
#ifdef USE_GAUSS_SELECT
  long l; 
  OFFLINE_VQ_HMM_type * vqhmm = (OFFLINE_VQ_HMM_type*) gvv->offline_vq;
  
  l = vqhmm->sVQ_CLS_TO_EVAL << 15; 
  l /= vqhmm->n_cs; 
  *a = l;

  l = vqhmm->sVQ_NUM_INTER << 15; 
  *b = (l/vqhmm->n_cs);
#endif
}

/* setup parameters for online reference modeling, VAD and EOS detection
   @param num_frms_gbg: number of begining frames to construct gabage model, default 9 
   @param noise_level : the noise level threshold to choose an LLR threshold
   @param th_llr_low_noise: the LLR threshold in low noise level
   @param th_llr_high_noise: the LLR threshold in high noise level 
   @param ctn_th : number of frames that continuously below a threshold, default 80
   @param llr_perc_th : the threshold is computed as $llr_perc_th * max_llr, where max_llr
   is the score of the best matched cluster, default 3277 in Q15
   @param start_from : don't do EOS decision making untill $start_from of frames have
   passed the peak of LLR, default 30
   @param nbr_frm_after_peak_LLR : the threshold of number of frames that have passed
   the peak of LLR
*/
TIESRENGINECOREAPI_API void SetTIesrORM(short num_frms_gbg, short noise_level,
		 short th_llr_low_noise, short th_llr_high_noise,
		 short cnt_th, ushort llr_perc_th, short start_from, 
		 short nbr_frm_after_peak_LLR, gmhmm_type * gv)
{
#if defined(USE_ORM_VAD)||defined(USE_ONLINE_REF)
  rj_set_ORM( num_frms_gbg, noise_level, th_llr_low_noise, th_llr_high_noise,
	      cnt_th, llr_perc_th, 
	      start_from, nbr_frm_after_peak_LLR, gv); 
#endif
}

TIESRENGINECOREAPI_API void GetTIesrORM(short * num_frms_gbg, short* noise_level,
		 short* th_llr_low_noise, short* th_llr_high_noise,
		 short * cnt_th, ushort * llr_perc_th, short * start_from, 
		 short *  nbr_frm_after_peak_LLR, 
		 gmhmm_type * gv)
{
#if defined(USE_ORM_VAD)||defined(USE_ONLINE_REF)
  OFFLINE_VQ_HMM_type * pOffline = (OFFLINE_VQ_HMM_type*) gv->offline_vq;

  *num_frms_gbg = pOffline->inum_frms_gbg;
#ifdef USE_ORM_VAD
  *noise_level = pOffline->th_noise_lvl; 
  *th_llr_low_noise = pOffline->th_llr_in_low_noise; 
  *th_llr_high_noise = pOffline->th_llr_in_high_noise; 

  *cnt_th = pOffline->ending_cnt_th;
  *llr_perc_th = pOffline->ending_llr_perc;
  *start_from = pOffline->ending_cnt_stt ;

  * nbr_frm_after_peak_LLR= pOffline->ending_cnt_after_peak;
#endif
#endif
}


/*setup parameters for noise subtraction 
  x = y - sAlpha * n if y - sAlpha * n > 0, 
    = n >> sBeta, otherwise
  @param sAlpha default 29491 in Q15
  @param sBeta default 3 in Q0
*/
TIESRENGINECOREAPI_API void SetTIesrSS( ushort sAlpha, short sBeta, gmhmm_type * gv)
{
#ifdef USE_SNR_SS
  ss_set_parm( sAlpha, sBeta, (NssType*)gv->pNss); 
#endif
}

TIESRENGINECOREAPI_API void GetTIesrSS(ushort *sAlpha, short * sBeta, gmhmm_type * gv)
{
#ifdef USE_SNR_SS
  NssType* p_ss_crtl = (NssType*) gv->pNss; 
  *sAlpha = p_ss_crtl->sAlpha; 
  *sBeta = p_ss_crtl->sBeta; 
#endif
}

/* 
   1) setup paramters for confidence-driven adaptation 
   2) set the threshold of nbest to declare low confidence score 
*/
TIESRENGINECOREAPI_API void SetTIesrCM( short cm_th, short cm_th_nbest, gmhmm_type * gv)
{
#ifdef  USE_CONF_DRV_ADP
    cm_set_param(cm_th, cm_th_nbest, gv); 
#endif
}

TIESRENGINECOREAPI_API void GetTIesrCM( short* cm_th, short * cm_th_nbest, gmhmm_type * gv)
{
#ifdef USE_CONFIDENCE
  ConfType * pconf = (ConfType*)gv->pConf; 
#ifdef  USE_CONF_DRV_ADP
  *cm_th = pconf->adp_th;
#endif
  *cm_th_nbest = pconf->th_nbest; 
#endif
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void GetTIesrJAC(const gmhmm_type *gvv,
		 short *vec_per_frame,
		 short *comp_like,
		 short *max_seg_size,
		 short *smooth_coef,
		 short *discount_factor,
		 short *deweight_factor,
		 short *forget_factor )
{
  *vec_per_frame = gvv->jac_vec_per_frame;
  *comp_like = gvv->jac_component_likelihood;
  *max_seg_size = gvv->jac_max_segment_size;
  *smooth_coef = gvv->jac_smooth_coef;
  *discount_factor = gvv->jac_discount_factor;
  *deweight_factor = gvv->jac_deweight_factor;
  *forget_factor = gvv->sva_var_forget_factor;
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void SetTIesrJAC( gmhmm_type *gvv,
		 short vec_per_frame,
		 short comp_like,
		 short max_seg_size,
		 short smooth_coef,
		 short discount_factor,
		 short deweight_factor,
		 short forget_factor )
{
  gvv->jac_vec_per_frame = vec_per_frame;
  gvv->jac_component_likelihood = comp_like;
  gvv->jac_max_segment_size = max_seg_size;
  gvv->jac_smooth_coef = smooth_coef;
  gvv->jac_discount_factor = discount_factor;
  gvv->jac_deweight_factor = deweight_factor;
  gvv->sva_var_forget_factor = forget_factor;
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void GetTIesrSBC(const gmhmm_type *gvv, short *sbc_fgt )
{
  *sbc_fgt = gvv->sbc_fgt;
}


/*--------------------------------*/
TIESRENGINECOREAPI_API void SetTIesrSBC( gmhmm_type *gvv, short sbc_fgt )
{
  gvv->sbc_fgt = sbc_fgt;
}

#ifdef REALTIMEGUARD 
//  int realtime_th;  
//  int realt_guard;

  TIESRENGINECOREAPI_API void GetRealTimeGuardTh(short* th, gmhmm_type * gv)
  { *th = gv->realtime_th  ;
  }
  
  TIESRENGINECOREAPI_API void GetRealTGuard(short* guard,  gmhmm_type * gv)
  { *guard=gv->realt_guard; 
  }

  TIESRENGINECOREAPI_API void SetRealTimeGuardTh(short th, gmhmm_type * gv)
  {  gv->realtime_th= th;
  }

  TIESRENGINECOREAPI_API void SetRealTGuard(short guard, gmhmm_type * gv)
  {  gv->realt_guard=guard;
  }

#endif


/*--------------------------------
  OpenSearchEngine

  This function initializes the gmhmm_type object parameters and variables
  in preparation for speech recognition. 
  --------------------------------*/
/* GMHMM_SI_API */ 
TIesrEngineStatusType  OpenSearchEngine(gmhmm_type *gvv)
{
  gvv->signal_cnt = 0;
  gvv->index_mean = 0;
  gvv->mfcc_cnt = 0;  

  init_uttdet(gvv);

  gvv->speech_detected = 0;
  gvv->frm_cnt = 0;
  gvv->amplitude_max = gvv->amplitude_min = 0;
  gvv->nbr_cpy = 0; 
  gvv->pred_first_frame = TRUE;

#ifdef USE_SNR_SS
  ss_reset((NssType * )gvv->pNss);

#endif

  /* init N-best */
  gvv->nbest.n_bests = 0; 
  
#if defined(OFFLINE_CLS) || defined(RAPID_JAC)
  rj_init( gvv); 
#endif

#if defined(USE_CONFIDENCE)
  cm_init( gvv );
#endif

#if defined(MODEL_LEVEL_PRUNE)
  mlp_init(gvv);
#endif

  return eTIesrEngineSuccess;
}
