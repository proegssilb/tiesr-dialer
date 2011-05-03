/*=======================================================================
 uttdet.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 Utterance detector functions.

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"
#include "tiesrcommonmacros.h"
#include "mfcc_f_user.h"
#include "noise_sub_user.h"
#include "dist_user.h"

#include "uttdet_user.h"
#include "uttdet.h"

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "uttdet.h"
*/

/*
** reset sad control parameters
*/

/*
void SetTIesrSAD(void *gvv, short a, short b, short c, short d, short e)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;

  gv->th_speech_delta = a;
  gv->th_min_speech_db = b; 
  gv->th_min_beg_frm = c;
  gv->th_min_end_frm = d;
  gv->th_noise_flr = e ;
}
*/

/*
void GetTIesrSAD(void *gvv, short  *a, short  *b, short  *c, short  *d, short  *e)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;

  *a = gv->th_speech_delta;
  *b = gv->th_min_speech_db;
  *c = gv->th_min_beg_frm;
  *d = gv->th_min_end_frm;
  *e = gv->th_noise_flr;
}

*/


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short SpeechDetected(gmhmm_type *gvv)
{
/*  gmhmm_type *g = (gmhmm_type *)gvv;*/

  return ( gvv->speech_detected );
}


/*--------------------------------*/
/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short SpeechEnded(gmhmm_type *gvv)
{
  //  return ( gvv->speech_detected && ( gvv->uttdet_state == NON_SPEECH ) );

#ifdef USE_ORM_VAD
  /* the modification corrected 401 error, no alignment,
     also works if there is sniff before the speech signal*/
  /* the new modification to end speech 
     printf(" %d %d %d %d \n", gvv->speech_detected ,
     gvv->uttdet_state == NON_SPEECH , 
     gvv->sr_is_end_of_grammar, 
     gvv->gbg_end_of_speech); 
  */
  return ( gvv->sr_is_end_of_grammar && gvv->gbg_end_of_speech);

  /*
  return ( gvv->sr_is_end_of_grammar && 
	   ((gvv->speech_detected && gvv->uttdet_state == NON_SPEECH ) 
	    || gvv->gbg_end_of_speech));
  */
  //  return ( (gvv->speech_detected && ( gvv->uttdet_state == NON_SPEECH ) 
  //    && gvv->sr_is_end_of_grammar ) || gvv->gbg_end_of_speech);
#else
  /* the modification corrected 401 error, no alignment */
  return ( gvv->speech_detected && ( gvv->uttdet_state == NON_SPEECH ) 
	   && gvv->sr_is_end_of_grammar );
#endif
}

/* ---------------------------------------------------------------------------
   extern
   ---------------------------------------------------------------------- */

/* Now defined in mfcc_f_user.h
extern void fft(short *real, short *imag);
*/
/* ---------------------------------------------------------------------------
   attenuate low and high frequency components
   ---------------------------------------------------------------------- */

static void frequency_shaping( short *ps )
{
  short i, alpha, win2 = WINDOW_LEN >>1;
  long  tmp;

  /* attenuate low freq */

  for (i = L_CUT, alpha = 32767; i >= 0; i--) {
    
    tmp = (long) ps[ i ] * alpha;
    tmp >>= 15;
    ps[ i ] = (short) tmp;

    tmp = (long) alpha * L_RATE;
    tmp >>= 15;
    alpha = (short) tmp;
  }

  /* attenuate high freq */

  for (i = H_CUT, alpha = 32767; i < win2; i++) {
    
    tmp = (long) ps[ i ] * alpha;
    tmp >>= 15;
    ps[ i ] = (short) tmp;

    tmp = (long) alpha * H_RATE;
    tmp >>= 15;
    alpha = (short) tmp;
  }

  /* power spectrum is symmetric */

  ps[ win2 ] = 0;
  for (i = 0; i < win2; i++)
    ps[ win2 + i ] = ps[ win2 - i ];

}

/* ---------------------------------------------------------------------------
   normalize
   ---------------------------------------------------------------------- */

static void
normalize(short *sig, short len, short *norm)
{
  short i, tmp, max;

  max = 0;

  /* find max */

  for (i = 0; i < len; i++) {
    tmp = sig[ i ] > 0 ? sig[ i ] : (- sig[ i ]);
    if ( tmp > max ) max = tmp;
  }

  /* normalize to most precision */

  if ( max > 0 ) {

    for (*norm = 0; max < 0x4000; (*norm) ++)
      max <<= 1;
    
    *norm = *norm > 0 ? *norm - 1 : *norm;

  } else {

    *norm = 0;
  }

  /* normalize */

  for (i = 0; i < len; i++) {

    sig[ i ] <<= *norm;
  }
}


/* ---------------------------------------------------------------------------
   update speech level and noise level
   ---------------------------------------------------------------------- */

TIESRENGINECOREAPI_API short
update_level(short new_val, short old, short uptc, short dntc)
{
  long  tmp;
  short tc;

  tc = new_val > old ? uptc : dntc;
  tmp =  SMOOTHING(new_val, tc, old);
  return (short) LONG_RIGHT_SHIFT_N(tmp, 15);


  // tmp = (long) ( 32767 - tc ) * new_val;
  // tmp += (long) tc * old;
  // tmp += ( 1 << 14 );
  // tmp >>= 15;
}

/*
**(per utterance) init utterance detection parameters
*/

TIESRENGINECOREAPI_API void
init_uttdet(gmhmm_type *gv)
{
  gv->sm_autoc = INI_NOISE * AC_SCALE;
  gv->noise_level = INI_NOISE * AC_SCALE;
  gv->speech_level = INI_SPEECH * AC_SCALE;
  gv->uttdet_state = NON_SPEECH;
  gv->speech_detected = 0; 
  gv->sr_is_end_of_grammar = FALSE; 

  gv->uttdet_end_frm = -1; 
  gv->uttdet_beg_frm = -1; 
  
  init_spect_sub(&(gv->noise_smooth));

}



/*--------------------------------
  set_default_uttdet

  Set utterance detector default parameters.  This should 
  be called when opening the asr.
  --------------------------------*/
TIESRENGINECOREAPI_API void set_default_uttdet(gmhmm_type *gv)
{
  /*
  ** set default thresholds
  */
  gv->th_noise_flr = TH_NOISE_FLR_DEFAULT;
  gv->th_speech_delta = TH_SPEECH_DELTA_DEFAULT;
  /* min dB level to be considered as speech */
  gv->th_min_speech_db = TH_MIN_SPEECH_DB_DEFAULT;
  /* larger (e.g. 5)  -> more robust to  noise pulses */
  gv->th_min_beg_frm = TH_MIN_BEG_FRM_DEFAULT;
  /* smaller (.e.g. 30) --> quicker latency  time */
  gv->th_min_end_frm = TH_MIN_END_FRM_DEFAULT;

}

//void set_uttdet(short min_speech_db, gmhmm_type *gv)
//{
//  /* min dB level to be considered as speech */
//  gv->th_min_speech_db = min_speech_db;
//}

/* ---------------------------------------------------------------------------
   utterance detection state machine, 
   input: uttdet_isspeech, uttdet_state; 
   output: uttdet_state
   ---------------------------------------------------------------------- */
static void
uttdet_state_machine(short frm_cnt, gmhmm_type *gv)
{
  switch ( gv->uttdet_state ) {

  case NON_SPEECH:
    
    if (  gv->uttdet_isspeech ) {

      gv->uttdet_cnt = 1;
      gv->uttdet_beg_frm = frm_cnt - 1;
      gv->delta_sum_accum = (long) ( gv->sm_autoc -  gv->noise_level);
      gv->uttdet_state = PRE_SPEECH;
    }

    break;

  case PRE_SPEECH:

    if ( gv->uttdet_isspeech ) {
      short go_to_speech;
      gv->uttdet_cnt++;
      gv->delta_sum_accum += gv->sm_autoc - gv->noise_level;

      //      if ( uttdet_cnt < MIN_BEG_FRM ) {
	/* SPEECH_DELTA * 1.25: */
      go_to_speech = (gv->uttdet_cnt == gv->th_min_beg_frm) ||
        	     ((gv->delta_sum_accum > ((gv->th_speech_delta + (gv->th_speech_delta >> 2)) * gv->uttdet_cnt)) &&
		      (gv->uttdet_cnt >= gv->th_min_beg_frm - 2));
      if  (!go_to_speech) 

	gv->uttdet_state = PRE_SPEECH;

      else {

	gv->uttdet_state = IN_SPEECH;
      }
    } else {

      gv->uttdet_state = NON_SPEECH;
    }

    break;

  case IN_SPEECH:

    if ( gv->uttdet_isspeech ) {

      gv->uttdet_state = IN_SPEECH;

    } else {

      gv->uttdet_end_frm = frm_cnt;
      gv->uttdet_cnt = 1;
      gv->uttdet_state = PRE_NONSPEECH;
    }
      
    break;

  case PRE_NONSPEECH:

    if ( gv->uttdet_isspeech ) {

      gv->uttdet_state = IN_SPEECH;

    } else {

      if ( gv->uttdet_cnt < gv->th_min_end_frm ) {

	gv->uttdet_cnt++;
	gv->uttdet_state = PRE_NONSPEECH;

      } else {

	gv->uttdet_state = NON_SPEECH;
      }
    }

    break;
  }
}
 

/* ---------------------------------------------------------------------------
   compute autocorrelation
   ---------------------------------------------------------------------- */

#ifndef USE_AUDIO
FILE *pf = NULL; /* for debug use */
#endif


void
compute_uttdet(short *power_spectrum, short cnt, short frm_cnt, short *mfcc, 
	       gmhmm_type *gv, NormType *var_norm)
{
  short imag[ WINDOW_LEN ];
  short i;
  unsigned long  tmp_l;
  short tmp_s, max_pow;
  short norm3, norm4;
  short autoc;               /* autocorrelation */
  short arg_max;

  /* fixed-point MFCC front end power spectrum need this much compensation */
  norm3 = ( var_norm->norm0 + var_norm->norm1 - 8 ) * 2 + ( var_norm->norm2 - 2 ) - 15;

  /* in-place spectral subtraction */
  noise_subs(power_spectrum, norm3, gv->ps_noisy_smoothed, gv->noise_floor, 
	     gv->signal_cnt, &(gv->noise_smooth)); 

  /* attenuate lower and higher frequency bins */
  frequency_shaping( power_spectrum );

  //n=WINDOW_LEN>>1; energy=0;for (i = 0; i < n; i++) energy+=power_spectrum[i]; energy=energy
  for (i = 0; i < WINDOW_LEN; i++) imag[ i ] = 0;

  /* normalize to the most precision */
  normalize( power_spectrum, WINDOW_LEN, &norm4 );

  norm3 += norm4;

  /* back to time-domain, get AC */
  fft( power_spectrum, imag );    /* IFFT, input symmectric, by FFT , */

  /* search for the fundamental frequency */
  tmp_s = power_spectrum[ LEND ];
  for (i = LEND, arg_max = LEND; i < HEND; i++)
    if ( power_spectrum[ i ] > tmp_s ) {
      tmp_s = power_spectrum[ i ];
      arg_max = i;
    }
  
  max_pow = tmp_s;

  tmp_l = tmp_s;
  tmp_s = log_polyfit( tmp_l, norm3);   /* Q 9 */

  tmp_l = (long) tmp_s * LOG2TOLOG10;/* Q 9 * Q 15 */
  tmp_l *= 10;
  autoc = tmp_l >> 18;               /* 15 + 9 - 18, Q 6 */
  
  /* smoothed autoc */

  tmp_l = SMOOTHING(gv->sm_autoc, SMOOTH,  autoc);
  gv->sm_autoc = LONG_RIGHT_SHIFT_N(tmp_l,15);

  /* track peak and dip */

  gv->speech_level = update_level( gv->sm_autoc, gv->speech_level, SLUPTC, SLDNTC );// 0.36 0.998 if arg1 big, use fast update, if arg1 small, use slow keep
  gv->noise_level  = update_level(gv-> sm_autoc, gv->noise_level,  NLUPTC, NLDNTC );//
  
  tmp_s=gv->sm_autoc;
  tmp_s+=tmp_s>>4 ;  
  tmp_s+=tmp_s>>5;
  
  // tmp_s-=100;  //2.5dB *64 
  /*noise floor default =0,  th speech delta=10 db, speech level=50*/
  if ( tmp_s > ( MAX((gv->th_noise_flr *  AC_SCALE), gv->noise_level) + gv->th_speech_delta * AC_SCALE ) &&
       ( tmp_s > gv->th_min_speech_db * AC_SCALE ) )
    gv->uttdet_isspeech = 1; 
  else
    gv-> uttdet_isspeech = 0;

  /* state */
  
  uttdet_state_machine(frm_cnt, gv);
  
#ifndef USE_AUDIO
  if (pf) fprintf(pf,"[%3d] %6d %6d %6d %6d %6d %6d %6d %6d %6d %6d %6d %6d\n",frm_cnt,
		  gv->sm_autoc, gv->speech_level, gv->noise_level, gv->uttdet_isspeech, gv->uttdet_state, 
		  mfcc[0],gv->noise_level+ gv->th_speech_delta * AC_SCALE, log_pow, arg_max,
		  gv->th_noise_flr * AC_SCALE ,gv->th_speech_delta,gv->th_min_speech_db
		  ); 
#endif

}
