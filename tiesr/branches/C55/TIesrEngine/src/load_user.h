/*=======================================================================

 *
 * load_user.h
 *
 * Header for interface for loading data.
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
 
 This header defines the interface with the model and grammar loading
 functions.

======================================================================*/

#ifndef LOAD_USER_H
#define LOAD_USER_H


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
#include "gmhmm_type.h"

/* This is included in load_user.h to remind the user that the files
   containing models and grammar must have fixed names */
#include "filename.h"

typedef struct fileConfiguration
{
      short byteMu;
      short byteVar;
} fileConfiguration_t;




/*--------------------------------*/
/* The user interface to the loading functions */

TIESRENGINECOREAPI_API TIesrEngineStatusType load_scales( char *fname, gmhmm_type *gv );

TIESRENGINECOREAPI_API unsigned short load_n_mfcc( char *fname );

TIESRENGINECOREAPI_API void free_scales(gmhmm_type *gv);

TIESRENGINECOREAPI_API void free_models(gmhmm_type *gv);

/* @param bNotReadNetwork TRUE if the function does not read recognition network 
 @param network_file char* to name of symbol network file 
 @param word_list_file char* to the name of word list file
*/
TIESRENGINECOREAPI_API TIesrEngineStatusType load_models(char *mod_dir, gmhmm_type *gv,
				  char bNotReadNetwork, 
				  char * network_file, 
				  char * word_list_file );

#ifdef USE_16BITMEAN_DECOD
/* load mean vectors to either base mean for decoding or original mean for adaptation 
   @param mod_dir pointer to model directory
   @param gv pointer to gmhmm_type 
   @bLoadBaseMu TRUE if load to base mean, FALSE if load to original mean */
TIESRENGINECOREAPI_API TIesrEngineStatusType load_mean_vec(char * mod_dir, gmhmm_type * gv ,
				    Boolean bLoadBaseMu );
#endif

/* @param fname file name of a model ,
      @param len pointer to the length of the read data */
TIESRENGINECOREAPI_API short *load_model_file(char fname[], unsigned short *len);

TIesrEngineStatusType load_model_config(char fname[], 
					fileConfiguration_t *fileConfig );

TIesrEngineStatusType unpacking(short *base, ushort nbr, short n_mfcc,
				FILE *fp, short scale[]);

/* allocate a pointer to a space from a block of memory 
   @param base_mem pointer to the block of memory
   @param mem_count pointer to the currently used memory size
   @param size_in_short requested memory size in word
   @param max_mem_usage the maximum size of the block in word
   @param aAlign if the requested data needs to adjust for SHORT INT or LONG data format
   @param mesg message of this operation */
pMemory mem_alloc( short * base_mem, ushort * mem_count, 
		  ushort size_in_short, unsigned int max_mem_usage, 
		  unsigned short aAlign, char *mesg );
// You have tp implement
TIesrEngineStatusType load_word_list( char *fname, gmhmm_type *gv );
#endif
