/*=======================================================================
 noise_sub_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header exposes the functions of the noise_sub.cpp file.

======================================================================*/

#ifndef NOISE_SUB_USER_H
#define NOISE_SUB_USER_H


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

#include "noise_sub.h"
#include "gmhmm_type.h"

TIESRENGINECOREAPI_API void init_spect_sub(short *noise_smooth);

TIESRENGINECOREAPI_API void noise_subs(short *ps_signal, short norm,
		short *ps_noisy_smoothed, short *noise_floor, 
		unsigned short count_frm, short *noise_smooth);

/* ------------- user interface for the Berotti-style noise supression ---------------- */
/* Open space for noise supression. Parameters for noise supression is also initialized. 
*/
TIESRENGINECOREAPI_API void ss_open( NssType * ns , gmhmm_type *gv) ;

/* reset status */
void ss_reset(NssType * ctrl);

/* set default parameters for noise supression 
   The following parameters are set by default
  ns->sAlpha = ALPHA;
  ns->sBeta =  NSS_BETA ; 

  ns->alpha_p = PROB_ALPHA; 
  ns->lambdaLTEhigherE = LAMBDA_LTE_HIGHER_E; 
*/
void ss_set_default( NssType * ns ) ;

/* set SS parameters 
   @param sAlpha : factor to times reference signal. sAlpha = 0, corresponds to no SS
                   default is NSS_ALPHA.
		   x = y - sAlpha * n
   @param sBeta :  factor to times noise, sBeta * n		
*/
void ss_set_parm( ushort sAlpha, short sBeta, NssType* p_ss_crtl);

/* Close spectral subtraction
   @param gv pointer to recognizer structure 
*/
TIESRENGINECOREAPI_API void ss_close(gmhmm_type *gv);

#ifdef USE_SNR_SS

/* The function extracts enhanced MFCC, which is obtained from noise removed spectra. 
   @param mfcc pointer to generated MFCC 
   @param power_spectrum pointer to enhanced spectra
   return Frame Dropping decision: TRUE if the frame is speech, FALSE if the frame is non-speech */
TIESRENGINECOREAPI_API
Boolean ss_mfcc_a_window(short *sig, short *mfcc, short *log_mel_energy,
		 const short n_mfcc, const short n_filter,
		 const mel_filter_type mel_filter[],
		 const short *cosxfm[  MAX_DIM_MFCC16 ],
		 short power_spectrum[], const short *mu_scale_p2, 
		 NormType *var_norm, short *last_sig, NssType * p_ss_crtl);

#endif

#endif
