/*=======================================================================
 gmhmm_type_common_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header file was created to encapsulate the common functions 
 that interface with the gmhmm_type structure, and yet are not exposed
 to the TIesr application designer.

======================================================================*/

#ifndef GMHMM_TYPE_COMMON_USER_H
#define GMHMM_TYPE_COMMON_USER_H


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


#include "status.h"
#include "tiesrcommonmacros.h"
#include "gmhmm_type.h"
#include "tiesr_config.h"

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short GetFrameCount(const gmhmm_type *gvv);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API long GetTotalScore(const gmhmm_type *gvv);

#ifdef _MONOPHONE_NET

ushort GetNumNbests(const gmhmm_type *gvv); 

ushort GetBestWordStartAt(unsigned short i, const gmhmm_type *gvv);

#endif

#ifdef REALTIMEGUARD 
  TIESRENGINECOREAPI_API void GetRealTimeGuardTh(short* th, gmhmm_type * gvv);
  TIESRENGINECOREAPI_API void GetRealTGuard(short* time,  gmhmm_type * gvv);
  TIESRENGINECOREAPI_API void SetRealTimeGuardTh(short th, gmhmm_type * gvv);
  TIESRENGINECOREAPI_API void SetRealTGuard(short th, gmhmm_type * gvv);
#endif

#ifdef USE_NBEST

ushort GetBestWordStartAt(unsigned short i, const gmhmm_type *gvv);

#endif

TIESRENGINECOREAPI_API ushort GetNumNbests(const gmhmm_type *gvv);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API ushort GetAnswerCount(const gmhmm_type *gvv, short nbest_idx);

/* GMHMM_SI_API */
TIESRENGINECOREAPI_API const unsigned short *GetAnswerIndex(const gmhmm_type *gvv, short nbest_idx);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API const char *GetAnswerWord(unsigned short i, const gmhmm_type *gvv, short nbest_idx);

/* GMHMM_SI_API */
TIESRENGINECOREAPI_API short GetConfidenceScore(const gmhmm_type *gvv);

/* GMHMM_SI_API on Gaussian selection */
TIESRENGINECOREAPI_API void GetTIesrGaussSel(const gmhmm_type *gvv,
        ushort  *a, ushort  *b );
TIESRENGINECOREAPI_API void SetTIesrGaussSel( gmhmm_type *gvv,
        ushort perc_cor_clusters,
        ushort perc_inter_clusters );

/*--------------------------------
  invoke N-best 
*/
TIESRENGINECOREAPI_API void SetTIesrNBest(gmhmm_type *gvv, Boolean bNbest);

TIESRENGINECOREAPI_API void GetTIesrNBest(gmhmm_type *gvv, Boolean *bNbest);

/* GMHMM_SI_API on On-line Reference Modeling */
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
		 short  nbr_frm_after_peak_LLR, gmhmm_type * gv);

TIESRENGINECOREAPI_API void GetTIesrORM(short * num_frms_gbg, short* noise_level,
		 short* th_llr_low_noise, short* th_llr_high_noise,
		 short * cnt_th, ushort * llr_perc_th, short * start_from, 
		 short *  nbr_frm_after_peak_LLR, gmhmm_type * gv);

/* GMHMM_SI_API on noise subtraction */
TIESRENGINECOREAPI_API void SetTIesrSS( ushort sAlpha, short sBeta, gmhmm_type * gv);
TIESRENGINECOREAPI_API void GetTIesrSS(ushort *sAlpha, short * sBeta, gmhmm_type * gv);

/* --------------- GMHMM_SI_API on confidence measure setup ------- */
/* 
   1) setup paramters for confidence-driven adaptation 
   2) set the threshold of nbest to declare low confidence score 
*/
TIESRENGINECOREAPI_API void SetTIesrCM( short cm_th, short cm_th_nbest, gmhmm_type * gv) ;
TIESRENGINECOREAPI_API void GetTIesrCM( short* cm_th, short * cm_th_nbest, gmhmm_type * gv);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API ushort GetCycleCount(const gmhmm_type *gvv);

TIESRENGINECOREAPI_API void GetTIesrPrune(const gmhmm_type *gvv, short *value);

TIESRENGINECOREAPI_API void GetTIesrTransiWeight(const gmhmm_type *gvv, short *value);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short GetVolumeStatus(const gmhmm_type *gvv);

TIESRENGINECOREAPI_API void SetTIesrPrune(gmhmm_type *gvv, short value);

TIESRENGINECOREAPI_API void SetTIesrTransiWeight(gmhmm_type *gvv, short value);

TIESRENGINECOREAPI_API void GetTIesrSAD(const gmhmm_type  *gvv, short  *a, short  *b, short  *c,
		 short  *d, short  *e);

TIESRENGINECOREAPI_API void SetTIesrSAD(gmhmm_type *gvv, short a, short b, short c, short d, short e);

TIESRENGINECOREAPI_API void GetTIesrJAC( const gmhmm_type *gvv,
		  short *vec_per_frame,
		  short *comp_like,
		  short *max_seg_size,
		  short *smooth_coef,
		  short *discount_factor,
		  short *deweight_factor,
		  short *forget_factor );

TIESRENGINECOREAPI_API void SetTIesrJAC( gmhmm_type *gvv,
		  short vec_per_frame,
		  short comp_like,
		  short max_seg_size,
		  short smooth_coef,
		  short discount_factor,
		  short deweight_factor,
		  short forget_factor );

TIESRENGINECOREAPI_API void GetTIesrSBC(const gmhmm_type *gvv, short *sbc_fgt );

TIESRENGINECOREAPI_API void SetTIesrSBC( gmhmm_type *gvv, short sbc_fgt );


/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API TIesrEngineStatusType  OpenSearchEngine(gmhmm_type *gvv);

#endif
