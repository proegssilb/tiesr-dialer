/*=======================================================================
 obsprob_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header defines the interface with the obsprob functions.

======================================================================*/

#ifndef _OBSPROB_USER
#define _OBSPROB_USER


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

#include "tiesrcommonmacros.h"
#include "search_user.h"
#include "gmhmm_type.h"

TIESRENGINECOREAPI_API void copy_feature(short feature[], ushort crt_vec, short n_mfcc);

TIESRENGINECOREAPI_API void observation_likelihood(ushort seg_stt, short T, HmmType *hmm, short pdf[], short n_mfcc, gmhmm_type *gv);

#endif
