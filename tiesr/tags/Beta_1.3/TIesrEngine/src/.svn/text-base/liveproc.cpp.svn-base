/*=======================================================================

 *
 * liveproc.cpp
 *
 * Per-frame call to the TIesr recognizer.
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

 This function defines the call to the TIesr SI recognizer.

======================================================================*/

/*  
** main loop for live version
*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif



#include "tiesr_config.h"

#include "status.h"
#include "winlen.h"
#include "gmhmm_type.h"
#include "mfcc_f_user.h"
#include "noise_sub_user.h"
#include "uttdet_user.h"
#include "search_user.h"
#include "jac_one_user.h"
#include "gmhmm_si_api.h"
#if defined(USE_GAUSS_SELECT) || defined(USE_ORM_VAD) 
#include "rapidsearch_user.h"
#endif
#ifdef USE_CONFIDENCE
#include "confidence_user.h"
#endif
#ifdef BIT8FEAT
#include "pack_user.h"
#endif


/* for multi-pass decoding */
static TIesrEngineStatusType 
MP_CallEachPass(short sig_frm[], gmhmm_type * gvv, Boolean bDoSearch)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  TIesrEngineStatusType a_status = eTIesrEngineSuccess;
  short power_spectrum[ WINDOW_LEN ]; /* 1/2 is NOT enough! (in autocorreletion) */  
  short idx, i, j, log_mel_energy[ N_FILTER26 ], noise_start_time;
  long  tmp;
  ushort idx_noise;
  short reg_mfcc[ MAX_DIM_MFCC16 ]; /* Q 11 */  
  NormType var_norm;
  short mfcc_feature[ MAX_DIM ], *p_sp = mfcc_feature;
  short *p_fea = p_sp; 
  TIesrEngineJACStatusType jacStatus;

#ifdef WORDBACKTRACE 
  gvv->word_backtrace=WORDBT; //HMMBT; //WORDBT; 
#else
  gvv->word_backtrace=HMMBT;
#endif
  /* do nothing but shifting input buffer */
  if (gv->pred_first_frame) {
    /* overlap */
    for (i = 0; i < OVERLAP; i++)  gv->sample_signal[ i ] = sig_frm[ i + (FRAME_LEN - (OVERLAP)) ];
    gv->pred_first_frame = FALSE;

    return  eTIesrEngineSuccess;
  }
  /* pack the input data into proper location */
  for (i = 0; i < FRAME_LEN; i++)  gv->sample_signal[ i + OVERLAP] = sig_frm[ i ];

  /* put into circular buffer, for regression computation */
  idx = circ_idx( gv->signal_cnt );

#ifdef USE_SNR_SS
    ss_mfcc_a_window(gv->sample_signal, gv->mfcc_buf[ idx ], log_mel_energy, gv->n_mfcc, 
		     gv->n_filter, gv->mel_filter, gv->cosxfm,power_spectrum, 
		     gv->muScaleP2, &var_norm, &(gv->last_sig), (NssType*) gv->pNss);
#else
    mfcc_a_window(gv->sample_signal, gv->mfcc_buf[ idx ], log_mel_energy, gv->n_mfcc, 
		  gv->n_filter, gv->mel_filter, gv->cosxfm,power_spectrum, 
		  gv->muScaleP2, &var_norm, &(gv->last_sig), gv->pNss);
#endif

  /* utterance detection */
  compute_uttdet( power_spectrum, gv->signal_cnt, gv->signal_cnt+1, gv->mfcc_buf[ idx ], gv, &var_norm);

  if (gv->comp_type & CMN) 
    for ( i = 0; i <  gv->n_mfcc; i++ )  gv->mfcc_buf[ idx ][ i ] -= gv->mfcc_mean[ i ];     /* CMN */

#ifdef LIVEMODE

  if (gv->speech_detected == 0 && gv->uttdet_state == IN_SPEECH) 
  {
      gv->speech_detected = 1; 
      
      /*last JAC step, just deal with the rest of the model as much as you can */
      noise_start_time = gv->signal_cnt - NOISECUSHION ;//ongoing test code, unfinished ( MFCC_BUF_SZ - 1 ) + REG_SPAN;
      if (noise_start_time >=0) 
	 idx_noise = circ_idx( noise_start_time );
      else 
	 idx_noise = 0;
      
      jacStatus = per_frame_jac_compensate(gv->mfcc_buf[ idx_noise ], gv); 
      if( jacStatus != eTIesrEngineJACSuccess )
	 return eTIesrEngineCompFail;
  }
#else
  if (gv->speech_detected == 0 && gv->signal_cnt > FILEMODEWAIT  ) 
     gv->speech_detected = 1;   
#endif
  
  //init noise est with the first frame!
  if (gv->signal_cnt ==0 ) 
    for ( i = 0; i < gv->n_mfcc; i++ )  {
      gv->mfcc_noise[ i ] =  gv->mfcc_buf[ 0 ][ i ];
      for (j= MFCC_BUF_SZ - 1;j>=1;j--)
	gv->mfcc_buf[j][i] = gv->mfcc_buf[0][i];
    }

  /* modified so that CMN vector is accumulated for all frames */
  if (gv->speech_detected) { /* update CMN mean */
    if (gv->comp_type & CMN) 
      for ( i = 0; i < gv->n_mfcc; i++ ) {
	tmp = gv->mfcc_buf[ idx ][ i ] + (long) gv->mfcc_mean[ i ] * 127;
	gv->mfcc_mean[ i ] = (short) ( tmp >> 7 );
      }
  }
  else { /* silence, non-speech frame */
    if (gv->comp_type & JAC) {
      noise_start_time = gv->signal_cnt - NOISECUSHION;
      
      if (noise_start_time >=0) idx_noise = circ_idx( noise_start_time );
      else {
	idx_noise = 0;
      }
      /* cannot use log_mel_energy, as it is not bufferized */
#ifndef LIVEMODE  //FILEMODE: update jac every frame or only 10th frame.
      if (gv->signal_cnt < FILEMODEWAIT )  /*update for the first 10 frames. */
      {
	 jacStatus = per_frame_jac_compensate(gv->mfcc_buf[ idx_noise ], gv);
	 if( jacStatus != eTIesrEngineJACSuccess )
	    return eTIesrEngineCompFail;
      }
#else
      jacStatus = per_frame_jac_compensate(gv->mfcc_buf[ idx_noise ], gv);
      if( jacStatus != eTIesrEngineJACSuccess )
	 return eTIesrEngineCompFail;
#endif
    }
  }
  
#ifndef LIVEMODE
  /*FILEMODE: do JAC for all modles at 10th frame!*/
  if (gv->signal_cnt ==FILEMODEWAIT )
  {
     jacStatus = per_frame_jac_compensate(gv->mfcc_buf[ idx_noise ], gv); 
     if( jacStatus != eTIesrEngineJACSuccess )
	return eTIesrEngineCompFail;
  }
#endif

  if ( gv->speech_detected  ) {
    
    /* several frames has already passed detection, go back */
    idx = circ_idx( idx - ( MFCC_BUF_SZ - 1 ) + REG_SPAN );
    /* fprintf(pf, "speech = %d\n",  signal_cnt - ( MFCC_BUF_SZ - 1 ) + REG_SPAN); */

    compute_regression( idx, 1, reg_mfcc, gv->n_mfcc, gv->muScaleP2,  gv->mfcc_buf);
    
    for (i = 0; i <  gv->n_mfcc; i++)  {
      mfcc_feature[i] = gv->mfcc_buf[idx][i];
      mfcc_feature[i +  gv->n_mfcc] = reg_mfcc[i];
    }

#if defined(USE_GAUSS_SELECT) || defined(USE_ORM_VAD)
    
    /* compute Gaussian cluster scores */
    i = rj_clear_obs_scr(gv, mfcc_feature);
    /* update scores from an on-line reference model */
    rj_update_noise(gv, mfcc_feature, i); 

#if defined(USE_ORM_VAD)
    /* GMM-based VAD */
    while( (p_fea = rj_vad_proc( gv, mfcc_feature )) != NULL){
#else
    {
#endif

#endif

#ifdef BIT8FEAT
      if (gv->mem_feature) 
	vector_packing(p_fea, gv->mem_feature + gv->mfcc_cnt, gv->scale_feat, gv->n_mfcc);
#else
      if (mem_mfcc) 
      {
	for (i = 0; i < gv->n_mfcc; i++)
	{
	   mem_mfcc[gv->mfcc_cnt + i]   = p_fea[i];
	   mem_d_mfcc[gv->mfcc_cnt + i] = p_fea[i + gv->n_mfcc];
	}
      }

      // Store integer values of features during recognition
      // TODO: modify to handle static, delta, acc ...
      if( gv->mem_feature )
      {
	 short* memFeats = (short*)(gv->mem_feature);
	 for (i = 0; i < 2*gv->n_mfcc; i++)
	 {
	    memFeats[2*gv->mfcc_cnt + i]   = p_fea[i];
	 }
      }
#endif  


      /* search needs frm_cnt to start with 0 */
      
      if (bDoSearch) 
	a_status = (TIesrEngineStatusType) search_a_frame(p_fea, 1, gv->frm_cnt, gv);
      gv->frm_cnt++;
      gv->mfcc_cnt += gv->n_mfcc;
      if ( gv->frm_cnt == gv->max_frame_nbr) 
	return (eTIesrEngineFeatureMemoryOut);
      
#if defined(USE_GAUSS_SELECT) || defined(USE_ORM_VAD)
    }
#endif
    
  }
  
  (gv->signal_cnt)++;
  if ( gv->signal_cnt > TIMEOUT) return eTIesrEngineTimeOut;
  
  /* overlap */
  for (i = 0; i <  OVERLAP /* WINDOW_LEN - FRAME_LEN */ ; i++) {
    gv->sample_signal[ i ] = gv->sample_signal[ i + FRAME_LEN ];
#ifdef USE_AUDIO
    if (gv->sample_signal[i] > gv->amplitude_max) gv->amplitude_max = gv->sample_signal[i];
    else if (gv->sample_signal[i] < gv->amplitude_min) gv->amplitude_min = gv->sample_signal[i];
#endif
  }
  return a_status;
}

/*
** recognize one utterance, live
** return search status:
** compute regression and search 
*/ 

/* GMHMM_SI_API */ 
TIesrEngineStatusType CallSearchEngine(short sig_frm[], gmhmm_type *gvv, 
				       unsigned int aFramesQueued ) 
{
  TIesrEngineStatusType a_status = eTIesrEngineSuccess;
  gvv->FramesQueued= aFramesQueued;

#ifdef _MONOPHONE_NET
  a_status = MP_CallEachPass(sig_frm, (gmhmm_type*) gvv->pAux, TRUE); 
    /* use triphone to do second-pass */
  if (a_status != eTIesrEngineSuccess) return a_status; 
  a_status = MP_CallEachPass(sig_frm, gvv, FALSE); 
#else
  a_status = MP_CallEachPass(sig_frm, gvv, TRUE); 
#endif 

  return a_status;
}

