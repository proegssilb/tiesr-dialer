/*=======================================================================
 pmc_f_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header file exposes the interface for the pmc operations.

======================================================================*/

#ifndef PMC_F_USER_H
#define PMC_F_USER_H


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

#include "tiesrcommonmacros.h"
#include "gmhmm_type.h"
#include "mfcc_f_user.h"

TIESRENGINECOREAPI_API void log_spectral_compensation(short sp_log_ps[], short reg_sp_log_ps[],
			       short pmc_log_ps[],  short reg_pmc_log_ps[], 
			       const short noise_log_ps[], const short chn[],
			       short jac, short *log_df, short n_filter);

TIESRENGINECOREAPI_API void pmc(short *sp_mfcc_o,     /* original non-PMC MFCC mean */
    short *noise_log_ps, /* noise log mel energy */
    short *pmc_mfcc,     /* PMC-compensated MFCC mean */
    short *chn,          /* channel estimate  */
    short jac,           /* jac/pmc switch  */
    short n_mfcc, short n_filter, const short *mu_scale_p2, short *scale_mu,
    const short *cosxfm[  MAX_DIM_MFCC16 ], short * sp_bias ,
	 Boolean b8bitMean );

void SFB_pmc(short *sp_mfcc_o,     /* original non-PMC MFCC mean */
    short *noise_log_ps, /* noise log mel energy */
    short *pmc_mfcc,     /* PMC-compensated MFCC mean */
    short *chn,          /* channel estimate  */
    short jac,           /* jac/pmc switch  */
    short n_mfcc, short n_filter, const short *mu_scale_p2, short *scale_mu,
    const short *cosxfm[  MAX_DIM_MFCC16 ]);

#endif
