/*=======================================================================
 
 *
 * noise_subs.cpp
 *
 * Noise subtraction.
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
 
 
 Noise subtraction operating in power spectral domain. Added noise
 subtraction with VAD.
 
======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif


#include "tiesr_config.h"
#include "tiesrcommonmacros.h"
#include "mfcc_f_user.h"
#include "uttdet_user.h"
#include "dist_user.h"
#include "noise_sub_user.h"
#include "noise_sub.h"
#include "load_user.h"

//#define _DBG_NSS

#ifndef USE_AUDIO /* when audio on, disable print trace */
// #define PRT_TRACE  
#endif

#ifdef PRT_TRACE 
static FILE  *psm;
static FILE  *pns;
static FILE  *psi;
static FILE  *pso;
/* log2, in Q9, to dB, in float: */
#define log2todB(x) ((x)/(float)(1<<9) * log10(2) * 10)
/* norm: scaling */
#define power2dB(x,norm) 10*log10( ( (x)==0? 1: (x)) * pow(2., -norm))
#endif

TIESRENGINECOREAPI_API void init_spect_sub(short *noise_smooth)
{
  *noise_smooth =  NOISE_SMOOTH;
}

/*
** in-place spectral subtraction:
** both in log2 domain:
short *ps_noisy_smoothed;
short *noise_floor;
*/

TIESRENGINECOREAPI_API void noise_subs(short *ps_signal, short norm,
		short *ps_noisy_smoothed, short *noise_floor, unsigned short count_frm, short *noise_smooth)
{
  long  tmp_l;
  long  reference_lin, signal_lin, noise_lin, clean_lin, diff_lin;
  short i, dim = WINDOW_LEN >> 1;
  short signal_abs_log2, noise_abs_log2;
  short counter_sh, max_abs, norm_pow, total_norm;

#ifdef PRT_TRACE
  if (count_frm == 0) {
    psm = fopen("SMOf.d","w");
    pns = fopen("NYSf.d","w");
    psi = fopen("SIGIf.d","w");
    pso = fopen("SIGOf.d","w");
  }
#endif
  for (i=0; i < dim ; i++) { /* for each frequency bin */
      tmp_l = (long) ps_signal[i]; 
      if (tmp_l > 0) /* log, with correction of  normalization factor */
	signal_abs_log2 = log_polyfit( tmp_l, norm);   /* Q 9 */
      else 
	signal_abs_log2 = (count_frm == 0)? 20 <<9 : noise_floor[i]; /* ~65 dB in Q9 */
      
      if (count_frm == 0) noise_floor[i] = ps_noisy_smoothed[i] = signal_abs_log2;

      /* The smoothing and dip tracking are in absolute dB: */
      /* filtered PS of noisy speech, in log2  */
      tmp_l = SMOOTHING(ps_noisy_smoothed[i],(*noise_smooth),signal_abs_log2);
      ps_noisy_smoothed[i] = LONG_RIGHT_SHIFT_N(tmp_l,15);

      /* noise floor: */
      noise_floor[i] = update_level(ps_noisy_smoothed[i], noise_floor[i], SSNLUPTC, SSNLDNTC); 
      
      /* we have signal and noise estimate, in log2, both in Q9 */

      noise_abs_log2 =  noise_floor[i];

      max_abs = MAX(noise_abs_log2 /* Q9 */, signal_abs_log2 /* Q9 */);

      tmp_l = max_abs;
      tmp_l -= 15870; /* max value w/o overflowing pow_poly */
      if (tmp_l < 0) norm_pow = 0;
      else if (tmp_l == 0) norm_pow =1;
      else norm_pow = (tmp_l >> 9) + 1; 

      noise_abs_log2 -= 512 * norm_pow; /* norm_pow is dim-specific */
      signal_abs_log2   -= 512 * norm_pow;
      /* linear noise and signal: */
      signal_lin = pow_polyfit((short)signal_abs_log2);
      noise_lin  = pow_polyfit((short)noise_abs_log2);

      // Test whether new noise_lin >> 3 vs. old signal_lin >> 6 is causing problems with VAD
      // reference_lin = signal_lin >> 6;
      reference_lin = noise_lin >> 3; 

      counter_sh = 0;
      while (noise_lin & 0xfc000000){ /* make sure no overflow after mult by ALPHA (6bits) */
	signal_lin >>= 1;
	noise_lin >>= 1;
	counter_sh++;
      }
      
      diff_lin = signal_lin -  ALPHA * noise_lin;

      if (diff_lin < 0) clean_lin = reference_lin;

      else {
	diff_lin <<= counter_sh;
	/* clean_lin: cleaned-up speech signal */
	if (diff_lin < reference_lin)  clean_lin = reference_lin;
	else  clean_lin = diff_lin;
      }
      
      total_norm = norm_pow + norm;
      if (total_norm >= 0) clean_lin <<= total_norm;
      else clean_lin >>= (-total_norm);

#ifdef PRT_TRACE      
      fprintf(psm,"%f ", log2todB(ps_noisy_smoothed[i]));
      fprintf(pns,"%f ", log2todB(noise_floor[i]));
      fprintf(pso,"%f ", power2dB(clean_lin,norm));
      fprintf(psi,"%f ", power2dB(ps_signal[i],norm));
#endif
      if (clean_lin > 32767) { /* should not happen */
	ps_signal[i] =  32767;
	//	printf("clean limited\n");
      }
      else ps_signal[i] = (short) clean_lin; /* output cleaned, normalized speech */
  }

  if (count_frm == 9) *noise_smooth = MIN_SMOOTH;
  
#ifdef PRT_TRACE
  fprintf(psm,"\n");
  fprintf(pns,"\n");
  fprintf(psi,"\n");
  fprintf(pso,"\n");
#endif  
}

static void ss_init(NssType * ctrl)
{
  ctrl->nbSpeechFrame = 0; 
  ctrl->hangOver = 0;
  ctrl->meanEn = 0;
  ctrl->prevMeanEn = 0;

  ctrl->prob_sp = 0; 
  ctrl->frm_count = 0;
}

void ss_reset(NssType * ctrl)
{
  ctrl->nbSpeechFrame = 0; 
  ctrl->hangOver = 0;
  ctrl->prob_sp = 0; 
  ctrl->frm_count = 0;
}

void ss_set_parm( ushort sAlpha, short sBeta, NssType* p_ss_crtl)
{
  if (p_ss_crtl){
    p_ss_crtl->sAlpha = sAlpha;
    p_ss_crtl->sBeta = sBeta ;     
  }
}

static Boolean ss_VAD(short frm_eng, NssType * ctrl)
{
  short frmEn = ss_update_meanEn(frm_eng, (short*)ctrl);
  Boolean flagVADnest = 0; 

  ctrl->frmEn = frmEn; 
  if (ctrl->frm_count > 4){
    if (frmEn - ctrl->meanEn > SNR_THRESHOLD_VAD){
      flagVADnest = 1; 
      ctrl->nbSpeechFrame ++; 
    }else{
      if (ctrl->nbSpeechFrame > MIN_SPEECH_FRAME_HANGOVER)
	ctrl->hangOver = HANGOVER; 
      ctrl->nbSpeechFrame = 0; 
      if (ctrl->hangOver != 0){
	ctrl->hangOver --; 
	flagVADnest = 1; 
      }
      else
	flagVADnest = 0;
    }
  }
  return flagVADnest;
}

/* spectral subtraction, code implemented according to X. Huang's book. 
   with VAD to contral smoothing parameter in noise estimation.
*/
static void ss_noise_subs(short *pds_signal, short norm, 
			  short *noise_floor, short *snr, 
			  unsigned short count_frm, NssType * ctrl)
{
  long  tmp_l, ltmp2;
  long  signal_lin, clean_lin;
  long  lIndic;
  ushort Q15 = 32767;
  short i, dim = WINDOW_LEN >> 1;
  short signal_abs_log2, noise_abs_log2;
  short norm_pow, total_norm;
  ushort onemalpha_p= Q15 - ctrl->alpha_p; 
  long lgain_fm1 ; /* the gain at frequency f-1 */
  long gain; 
  short *sLongTermNoise = ctrl->ps_long_term_noise; 

  signal_abs_log2 = ss_est_frmEn(dim, pds_signal, norm); 
  if (ss_VAD(signal_abs_log2, ctrl))
    lIndic = 32767;
  else lIndic = 0; 

#ifdef DBG_SS
  printf("gain : ");
#endif

  /* calculate speech presence probability */
  ctrl->prob_sp = q15_x( onemalpha_p, lIndic ) + 
    q15_x( ctrl->alpha_p, ctrl->prob_sp ) ;
  for (i=0; i < dim ; i++) { /* for each frequency bin */
      tmp_l = (long) pds_signal[i]; 
      if (tmp_l > 0) /* log, with correction of  normalization factor */
	signal_abs_log2 = log_polyfit( tmp_l, norm);   /* Q 9 */
      else 
	signal_abs_log2 = (ctrl->frm_count == 0)? -3401 : noise_floor[i]; /* 0.01 in log2 in Q9 */

      unsigned short alpha_smooth; 
      if (count_frm == 0) {
	noise_floor[i] = signal_abs_log2 ; 
	sLongTermNoise[i] = noise_floor[i]; 
      }
      else{
	if (count_frm <= 10){
	  tmp_l = count_frm << 15 ; 
	  tmp_l /= (count_frm + 1);
	  alpha_smooth = LONG2SHORT(tmp_l);  /* 1-1/t in Q15 */

	  noise_floor[i] = q15_x(alpha_smooth, noise_floor[i]) 
	    + q15_x(32768 - alpha_smooth, signal_abs_log2);

	}else if ( ctrl->prob_sp < 6553 /* 0.2 in probability */){
	  if (sLongTermNoise[i] <= signal_abs_log2)
	    tmp_l = div32_32_Q(sLongTermNoise[i], signal_abs_log2, 9); /* Q9 */
	  else 
	    tmp_l = div32_32_Q(signal_abs_log2, sLongTermNoise[i], 9); /* Q9 */
	  tmp_l = (1<<9) - tmp_l; 
	  gain = tmp_l * tmp_l ; 
	  gain >>= 9; /* the 1 - 2*long_term_noise/y^l + (long_term_noise/y^l)^2 */
	    
	  tmp_l = noise_floor[i] - sLongTermNoise[i]; 
	  if (sLongTermNoise[i] <= signal_abs_log2)
	    tmp_l = div32_32_Q(tmp_l, signal_abs_log2, 9);
	  else
	    tmp_l = div32_32_Q(tmp_l, sLongTermNoise[i], 9);
	  tmp_l *= tmp_l; 
	  tmp_l >>= 9; 
	  tmp_l += gain; 
	  
	  if (tmp_l ==0) alpha_smooth = 0; 
	  else 
	    alpha_smooth = div32_32_Q(gain, tmp_l, 15); 
	
	  alpha_smooth = MAX(9830, alpha_smooth);
	  alpha_smooth = MIN(31785, alpha_smooth); 

	  noise_floor[i] = q15_x(alpha_smooth, noise_floor[i]) 
	    + q15_x(32768 - alpha_smooth, signal_abs_log2);

	}
	
	if (count_frm <= 10) {
	  tmp_l = count_frm << 15 ; 
	  tmp_l /= (count_frm + 1);
	  alpha_smooth = LONG2SHORT(tmp_l);  /* 1-1/t in Q15 */
	}
	else 
	  alpha_smooth = 29491; 
	sLongTermNoise[i] = q15_x(alpha_smooth, sLongTermNoise[i]) + 
	  q15_x(32768 - alpha_smooth, noise_floor[i]); 
      }

      noise_abs_log2 =  noise_floor[i];

      if (count_frm == 0)
	snr[i] = signal_abs_log2 - noise_abs_log2; 
      else// if (count_frm <= 9)// time smoothing 
	snr[i] = q15_x(3276, snr[i]) + q15_x(29491, signal_abs_log2 - noise_abs_log2);
      tmp_l = -snr[i]; 

      /* the noise to signal threshold is computed as 
	 N^l - Y^l > log2(1 - a) 
	 where a is 10^(-A/10) and A is the minimum SNR gain. 
	 The A is set to -15dB in the current setup. 
	 The following setup can be used 
	 [ -8dB -127 -1361 ]
	 [ -10dB -77 -1701 ]
	 [ -13.86dB -31  -2358]
	 [ -15dB -23 -2551]
	 [ -20dB -7 -3401 ] 
      */

      if (tmp_l >= -127){
	tmp_l = -1361; 
      }else{
	tmp_l = LOG2Q9 * tmp_l;
	tmp_l >>= 9; /* log(N/Y) in Q9 */
	tmp_l = hlr_expn(tmp_l, 9); /* Q15 */
	tmp_l = Q15 - tmp_l; /* 1 - N/Y in Q15 */
	tmp_l = log_polyfit(tmp_l, 15); /* Q9 */
	tmp_l = MAX(tmp_l, -1361);
      }

      ltmp2 = tmp_l; 
      if (i > 0) {
	/* smoothing across frequency */
	gain = 7*ltmp2 + lgain_fm1 ;
	gain >>= 3; 
      }else gain = tmp_l; 
      lgain_fm1 = gain; 

#ifdef DBG_SS
      printf("%d ", gain);
#endif

      /* enhanced signal */
      ltmp2 = signal_abs_log2 + gain; 
      
      /* transform to linear spectral domain */
      tmp_l = ltmp2; 
      tmp_l -= 15870; /* max value w/o overflowing pow_poly */
      if (tmp_l < 0) norm_pow = 0;
      else if (tmp_l == 0) norm_pow =1;
      else norm_pow = (tmp_l >> 9) + 1; 
    
      signal_abs_log2 = ltmp2; 
      signal_abs_log2 -= 512 * norm_pow;
      signal_lin = pow_polyfit((short)signal_abs_log2);
    
      clean_lin = signal_lin; 
    
      total_norm = norm_pow + norm;
      if (total_norm >= 0) clean_lin <<= total_norm;
      else clean_lin >>= (-total_norm);
    
      pds_signal[i] = LONG2SHORT(clean_lin);
  }

#ifdef DBG_SS
  printf("\n");
#endif
  /* output noise level at the 10th frame */
  //  if (count_frm == SS_NUM_FRM_FOR_NOISE_DET ){
  if (count_frm <= SS_NUM_FRM_FOR_NOISE_DET ){
    PRT_ERR(printf("Noise level (log2) = %d\n", ctrl->meanEn)); 
    ctrl->noiselvl = ctrl->meanEn; 
  } 
}

#ifdef SS_OUT_PSD
static void ss_dump_power_spectrum(short norm_log, short norm_lin, 
				   const short* power_spectrum, 
				   short size, FILE *pf_sp, Boolean bLogDomain){
  short i; 
  float spect_f;
  long tmp_l; 
  short signal_abs_log2; 
  if (pf_sp == NULL) return; 

  for (i=0; i< size; i++) {
    tmp_l = (long)  power_spectrum[ i ];
    if (bLogDomain) signal_abs_log2 = tmp_l; 
    else{
      if (tmp_l > 0) /* log, with correction of  normalization factor */
	signal_abs_log2 = log_polyfit( tmp_l, norm_log);   /* Q 9 */
      else 
	signal_abs_log2 =  /* 20 */ 1<< 9 ; /* ~65 dB in Q9 */
    }
    if (!bLogDomain) spect_f = (float)   signal_abs_log2/ (1<< 9);
    else spect_f = (float)   signal_abs_log2/ (1<< norm_lin);
    fwrite(&spect_f, sizeof(float),1,pf_sp);
  }
}
#endif

/* ---------------------------------------------------------------------------
   MFCC is in Q11 
   ---------------------------------------------------------------------- */

/* return TRUE if the frame is speech */
TIESRENGINECOREAPI_API Boolean
ss_mfcc_a_window(short *sig, short *mfcc, short *log_mel_energy,
		 const short n_mfcc, const short n_filter,
		 const mel_filter_type mel_filter[],
		 const short *cosxfm[  MAX_DIM_MFCC16 ],
		 short power_spectrum[], const short *mu_scale_p2, 
		 NormType *var_norm, short *last_sig, NssType * p_ss_crtl)
{
  short real_sig[ WINDOW_LEN ], imag_sig[ WINDOW_LEN ];
  short ps_psd[WINDOW_LEN >> 1];
  short is; 
  short norm;

  signal_to_power_spectrum(sig, real_sig, imag_sig, power_spectrum, var_norm, last_sig);

  for (is=((WINDOW_LEN>>1) - 1);is>=0;is--) ps_psd[is] = power_spectrum[is]; 

  norm = ( var_norm->norm0 + var_norm->norm1 - WINDOW_PWR ) * 2 + ( var_norm->norm2 - 2 ) - 15;

  /* in-place spectral subtraction */
  ss_noise_subs(ps_psd, norm, p_ss_crtl->s_noise_floor, p_ss_crtl->snr, 
		p_ss_crtl->frm_count, p_ss_crtl);

  ss_power_spectrum_to_mfcc(ps_psd, mfcc, 
			    log_mel_energy, n_mfcc, n_filter, mel_filter, 
			    mu_scale_p2, var_norm, cosxfm); 
#ifdef SS_OUT_PSD
  ss_dump_power_spectrum(norm, 9, ps_psd, WINDOW_LEN/2, p_ss_crtl->fp, FALSE);
  ss_dump_power_spectrum(9, 9, p_ss_crtl->s_noise_floor, WINDOW_LEN/2, p_ss_crtl->fp2, TRUE);
  ss_dump_power_spectrum(15, 15, (short*) & (p_ss_crtl->prob_sp), 1, p_ss_crtl->fp3, TRUE);
#endif

#ifdef _DBG_NSS
  for (is=0;is < 10;is++) printf("%d ", mfcc[is]);
  printf("\n");
#endif

  p_ss_crtl->frm_count ++; 

  return TRUE;
}

void ss_set_default( NssType * ns ) 
{

  ns->sAlpha = NSS_ALPHA;
  ns->sBeta =  NSS_BETA ; 

  ns->alpha_p = PROB_ALPHA; 
  ns->lambdaLTEhigherE = LAMBDA_LTE_HIGHER_E; 

}

TIESRENGINECOREAPI_API void ss_open( NssType * ns , gmhmm_type * gv )
{
  ns->mem_count = 0; 
  ns->s_noise_floor = mem_alloc(ns->base_mem, &ns->mem_count, 
				NSS_HALF_WINDOW, NSS_HALF_WINDOW * 3, 
				SHORTALIGN, 
				"noise floor" );
  ns->snr = mem_alloc(ns->base_mem, &ns->mem_count, 
		      NSS_HALF_WINDOW, NSS_HALF_WINDOW * 3, 
		      SHORTALIGN, 
		      "noise floor" );

  ns->ps_long_term_noise = mem_alloc(ns->base_mem, &ns->mem_count, 
				     NSS_HALF_WINDOW, NSS_HALF_WINDOW * 3, 
				     SHORTALIGN, 
				     "noise floor" );

  ss_set_default(ns);
  ss_init( ns ); 
}

TIESRENGINECOREAPI_API void ss_close(gmhmm_type * gv)
{
#ifdef SS_OUT_PSD
  NssType * p_ss_crtl = (NssType*)gv->pNss; 
  fclose(p_ss_crtl->fp);
  fclose(p_ss_crtl->fp2);
  fclose(p_ss_crtl->fp3);
#endif
}

