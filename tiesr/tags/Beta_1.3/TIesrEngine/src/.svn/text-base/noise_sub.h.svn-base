/*=======================================================================

 *
 * noise_sub.h
 *
 * Header for implementation of noise subtraction.
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


 This header holds the parameters needed for the noise_sub
 functionality.

======================================================================*/

#ifndef NOISE_SUB_H
#define NOISE_SUB_H

#include "tiesr_config.h"

#define SS_NUM_FRM_FOR_NOISE_DET 9

/* 1/16, Q15 */
#define MIN_SMOOTH 2048    

/* 0.5 in Q15 */
#define NOISE_SMOOTH 16384 

/* 0.96, Q15  */
#define  SSNLUPTC 31457 

/* used NLDNTC from uttdet, but this should be a separate 
   constant in case one desires to set the two differently */
/* exp(-1) = 0.3679, Q 15 */
#define  SSNLDNTC       12055

/* ----------------------- Spectral Subtraction ---------------------------------------------------------- 
   The following functions are added to spectral subtraction: 1) VAD for noise estimation, which is 
   copied from ETSI advanced front end, 2) Noise estimation with the VAD information, which basically
   updates noise estimates only in non-speech intervals. 3) Bernotti-style noise removal. 

   The parameters of the noise subtraction were tuned with 10dB 9 types noise plus WAVES noise in three
   driving conditions, namely highway, stopngo and parked. In all noise conditions, except for TIMIT noise,
   word error rates for 90 English name recognition with phone-book trained models was below 2 percent. 

   ------------------------------------------------------------------------------------------------------*/

//#define SS_OUT_PSD /* dump spectral subtraction results */

static const short coef_1downto1over10_in_Q15[ 10 ] = 
  { 32767 /* 1 in Q15 */, 16384 /*1/2 in Q15 */, 10923 /*1/3 in Q15*/,
    8192 /* 1/4 in Q15 */, 6554 /* 1/5 in Q15 */, 5461 /* 1/6 in Q15*/,
    4681 /* 1/7 in Q15 */, 4096 /* 1/8 in Q15 */, 
    3641 /* 1/9 in Q15 */, 3277 /* 1/10 in Q15 */};

/* ---------------- parameters of VAD for noise supression ---------------------------*/
#define BUFFER_SIZE 7	   // Number of frames in analysis buffer
#define SNR_THRESHOLD_UPD_LTE 640 /* 20/16 in Q9 */
#define MIN_FRAME 10 
#define ENERGY_FLOOR 2560 /* 80/16 in Q9 */
#define SNR_THRESHOLD_VAD 480 /* 15/16 * Q9*/
#define MIN_SPEECH_FRAME_HANGOVER 15 
#define LAMBDA_LTE 31785 /* 0.97 in Q15 */
#define LAMBDA_LTE_HIGHER_E 32440 /* 0.99 in Q15 */
#define HANGOVER 5

#define PROB_ALPHA 6554 // 0.2 in Q15 used for speech presence probability

#define NSS_MIN_FRAME_ENR -3401 /* 0.01 in log2 in Q9 used for frame energy estimation*/

/* ---------------- Beroutti-style noise supression parameters -------------------------*/
/* must be <= 32 */
#define ALPHA 30
#define NSS_ALPHA 29491 /* 0.9 in Q15 */
//#define NSS_ALPHA 0 /* 0.9 in Q15 */
#define NSS_BETA 3 /* 2^{-3} = 1/8 */

#define NSS_HALF_WINDOW (WINDOW_LEN>>1)
typedef struct 
{
  short base_mem[NSS_HALF_WINDOW * 3 ]; 
  ushort mem_count; 
  /* parameters */

  ushort alpha_p; /* for averaging over VAD decision, in Q15 */
  ushort sAlpha; /* alpha for noise removal with sAlpha * N, sAlpha in Q0 */
  short sBeta;  /* beta for noise removal with N * 2^(- sBeta), sBeta in Q0*/
  short lambdaLTEhigherE ;  /* forgetting factor, Q15 */

#ifdef SS_OUT_PSD
  FILE* fp;
  FILE * fp2; 
  FILE * fp3; 
#endif

  /* status */
  short frm_count;

  ushort prob_sp; /* speech presence probability, in Q15 */

  short *s_noise_floor; /* estimated noise level, log2 in Q9 */ 

  short *snr ; /* smoothed SNR estimate, log2 in Q9 */

  short *ps_long_term_noise;  /* long-term noise estimate, log2 in Q9 */

  short frmEn; /* energy of the current frame , log2 in Q9 */
  short meanEn; /* long-term energy , log2 in Q9 */
  short prevMeanEn; /* long-term energy of the previous utterance at the 10th frame, log2 in Q9 */
  short noiselvl; /* noise level at the 10th frame of the current utterance */
  short lambdaLTE; /* forgetting factor for long-term average, Q15 */
  short nbSpeechFrame; /* number of speech frames */
  short hangOver ;   /* hangover frames */

} NssType;


#endif


