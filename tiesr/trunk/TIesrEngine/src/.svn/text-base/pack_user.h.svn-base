/*=======================================================================

 *
 * pack_user.h
 *
 * Header for interface for byte/16-bit packing.
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

 Encapsulates the user interface for pack functionality.

======================================================================*/

#ifndef PACK_USER_H
#define PACK_USER_H


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



/*--------------------------------*/
/* Define a helper macro, for those using pack functionality */
/*
** separate static and dynamic mean values from a short word
*/
#define DECODE_STDY(tmp_sh, mu_static, mu_dynamic) \
    mu_static  = (tmp_sh & 0xff00); \
    mu_dynamic = (tmp_sh) << 8;


/*--------------------------------*/
/* User interface to packing functionality */


TIESRENGINECOREAPI_API void vector_packing(const short v_in[], unsigned short v_out[],
		    const short max_scale[], short D);

TIESRENGINECOREAPI_API void vector_unpacking(const unsigned short v_in[],
			     short v_out[], const  
			     short scale[], short D);

TIESRENGINECOREAPI_API short nbr_coding(short o_mu, short max_mu, int prt);


void trouncate(char *mesg, short *mfcc, int n_mfcc, int delta);


#endif
