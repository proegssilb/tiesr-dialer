/*=======================================================================

 *
 * volume_user.h
 *
 * Header for interface for volume measure.
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
