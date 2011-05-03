/*=======================================================================
 volume_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 Encapsulates the user interface to setting volume.

======================================================================*/

#ifndef VOLUME_USER_H
#define VOLUME_USER_H


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

#define LOW_VOL_LIMIT 1024

/* Note: 30000 doesn't work well with some PCs */
#define HIGH_VOL_LIMIT 28000 


/* GMHMM_SI_API */
TIESRENGINECOREAPI_API void SetTIesrVolRange(gmhmm_type *gvv, unsigned short low_v,
		      unsigned short high_v);

/* GMHMM_SI_API */ 
TIESRENGINECOREAPI_API void GetTIesrVolRange(gmhmm_type *gvv, unsigned short *low_v,
		      unsigned short *high_v);


TIESRENGINECOREAPI_API short set_volume_flag(gmhmm_type *gv);

#endif
