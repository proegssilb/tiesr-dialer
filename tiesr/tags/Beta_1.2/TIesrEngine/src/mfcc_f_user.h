/*=======================================================================
 mfcc_f_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 This header file defines the interface with the tiesr mfcc feature
 processor.  This is separated from mfcc_f.h, so that fundamental
 constants are not exposed to the user of the functions.
 Some constants here are used throughout other code, and so these are not
 encapsulated.

======================================================================*/

#ifndef MFCC_F_USER_H
#define MFCC_F_USER_H


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


#include "tiesr_config.h"
#include "mfcc_f_def_struct_user.h"
#include "gmhmm_type.h"


/*--------------------------------*/
/* The user interface functions */

TIESRENGINECOREAPI_API void mfcc_a_window( short *sig, short *mfcc, short *log_mel_energy,
		    const short n_mfcc, const short n_filter, 
		    const mel_filter_type mel_filter[], 
		    const short *cosxfm[],
		    short [], const short [], NormType *, short * ,
		    short *pNss);


TIESRENGINECOREAPI_API void cos_transform( short *mfcc, short *mel_energy, const short *scale_p2,
		    const short n_mfcc, const short n_filter,  
		    const short *cosxfm[]);


TIESRENGINECOREAPI_API void inverse_cos_transform( short *log_mel_energy, short *mfcc,
			   const short scale_p2[], const short n_mfcc, 
			   const short n_filter,  
			   const short *cosxfm[] );


TIESRENGINECOREAPI_API void fft(short *real, short *imag);

TIESRENGINECOREAPI_API short circ_idx(short idx);


/* void dim_p2_init( const short nbr_mfcc, gmhmm_type *gvv ); */
TIESRENGINECOREAPI_API void dim_p2_init( const short nbr_mfcc, gmhmm_type *gvv );


// Now defined in dist.cpp
/* log2()
   @param value fixed-point value
   @param compensate Q number of the value
   @return log2(value) in Q9 */
// TIESRENGINECOREAPI_API short log_polyfit(long value, short compensate);


TIESRENGINECOREAPI_API void compute_regression(short buf_idx, short type,  short *reg_mfcc,
			short n_mfcc, const short *mu_scale_p2, 
			short mfcc_buf[ /*MFCC_BUF_SZ*/ ][ MAX_DIM_MFCC16 ]);

void signal_to_power_spectrum(short *sig, short real_sig[], short imag_sig[], 
			      short *power_spectrum, NormType *nv, short *last_sig);

void
mel_scale_fft_spectrum(const short *real_sig, const short *imag_sig, 
		       long *mel_energy, const short n_filter, 
		       const mel_filter_type mel_filter[]);

void
fft_spectrum_to_mfcc(const short *real_sig,const short *imag_sig,
		     short *mfcc,            /* Q 11 */
		     short *log_mel_energy,  /* Q 9 */
		     const short n_mfcc, const short n_filter, 
		     const mel_filter_type mel_filter[],
		     const short *mu_scale_p2, NormType *var_norm, 
		     const short *cosxfm[  MAX_DIM_MFCC16 ]);

void
ss_power_spectrum_to_mfcc(const short *power_spectrum, 
		       short *mfcc,            /* Q 11 */
		       short *log_mel_energy,  /* Q 9 */
		       const short n_mfcc, const short n_filter, 
		       const mel_filter_type mel_filter[],
		       const short *mu_scale_p2, NormType *var_norm, 
		       const short *cosxfm[  MAX_DIM_MFCC16 ]);

short ss_est_frmEn(short dim, short * pds_signal, short norm);
short ss_update_meanEn(short frm_eng, short * ctrl);


#endif
