/*=======================================================================
 confidence_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
 
======================================================================*/

#ifndef CONFIDENCE_USER_H
#define CONFIDENCE_USER_H

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

/*--------------------------------------------------------*/

#include "tiesrcommonmacros.h"
#include "status.h"
#include "gmhmm_type.h"
#include "tiesr_config.h"
#include "search_user.h"
#include "confidence.h"

const ushort CM_NBR_NBEST_LOW_CONF = 20; 
/* the number of N-bests that shows that the utterance has low confidence */

/* open space for confidence measure calculation */
TIESRENGINECOREAPI_API TIesrEngineStatusType cm_open( ConfType * , gmhmm_type * );

/* The parameters for confidence-driven adaptation
   @param th_adp, the threshold for unsupervised adaptation, default = CM_ADP_THRE, which is
   -200.
   @param th_nbest, the threshold of number of N-bests to declare low 
		     confidence of an uttearnce 
		     default is 20
*/
TIESRENGINECOREAPI_API void cm_set_param( short th_adp,
		   short th_nbest,
		   gmhmm_type * gv );

/* initialize confidence measure for one utterance */
void cm_init(gmhmm_type * gv);

/* load parameters of confidence measure computation */
TIESRENGINECOREAPI_API void cm_load(FILE*fp, gmhmm_type *gv);

/* save updated parameter for confidence measure computation */
TIESRENGINECOREAPI_API void cm_save(FILE *fp,
	     gmhmm_type* gv);

/* return confidence score */
short cm_score(gmhmm_type *gv);

/* accumulate statistics */
TIESRENGINECOREAPI_API void cm_acc_seg_ratio_to_antimodel(gmhmm_type *gv,
        ushort hmm_code, 
        short stt, short stp,
        long scaling);

/* close confidence measure space */
TIESRENGINECOREAPI_API void cm_close( gmhmm_type* gv );

/* freeze background scores */
TIESRENGINECOREAPI_API void cm_freeze(ConfType* pconf);

/* get the address to the anti score at frame $frm_cnt */
short * cm_antiscr( ConfType* pconf, ushort frm_cnt );

/* compute confidence score and update confidence measure */
TIESRENGINECOREAPI_API void cm_update_status(gmhmm_type *gv);


/* --------------------- used for confidence-driven adaptation ------------------*/

/* backup channel and SBC parameters */
TIESRENGINECOREAPI_API void cm_backup_adp( gmhmm_type * gv);

/* restore channel and SBC parameters */
TIESRENGINECOREAPI_API void cm_restore_adp(gmhmm_type * gv);

/* ------------------------------------------------------------------------------*/

#endif
