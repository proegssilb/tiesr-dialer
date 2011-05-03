/*=======================================================================
 
 *
 * uttdet_user.h
 *
 * Header for the interface of utterance detection.
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

 This header file defines the interface with the tiesr utterance extractor.

======================================================================*/

#ifndef UTTDET_USER_H
#define UTTDET_USER_H


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


#include "gmhmm_type.h"
#include "mfcc_f_user.h"
#include "uttdet.h"

/* Utterance detector parameters exposed to the user.  These should
   not be exposed to the user but for now they are exposed.  The
   utterance detector state should be in an enum.
*/

#define  NON_SPEECH     0
#define  PRE_SPEECH     1
#define  IN_SPEECH      2
#define  PRE_NONSPEECH  3


/* 30 seconds at 50 frames/second */
#define TIMEOUT 1500

/* the minimum frame count in decoder to declare end of grammar */
#define TH_END_GRAMMAR_MIN_END_FRM 40

/*the cushion in number of frames before detected speech for noise estimate*/
#define NOISECUSHION 8  

/*--------------------------------*/
/* The uttdet user interface functions */

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short SpeechDetected(gmhmm_type *gvv);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API short SpeechEnded(gmhmm_type *gvv);

TIESRENGINECOREAPI_API void init_uttdet(gmhmm_type *);

TIESRENGINECOREAPI_API void set_default_uttdet(gmhmm_type *);

TIESRENGINECOREAPI_API void compute_uttdet(short *power_spectrum, short cnt,short frm_cnt, short *, gmhmm_type *, NormType *);

TIESRENGINECOREAPI_API short update_level(short new_val, short old, short uptc, short dntc);

TIESRENGINECOREAPI_API void init_spect_sub(short *);

#endif
