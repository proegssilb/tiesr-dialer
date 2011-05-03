/*=======================================================================

 *
 * jac_estm_user.h
 *
 * Header for interface of jac estimation.
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


 This header file defines the user interface with the JAC functions.

======================================================================*/

#ifndef _JAC_ESTM_USER_H
#define _JAC_ESTM_USER_H

#include "gmhmm_type.h"

/* for debug of JAC */
//#define _JAC_DEBUG 1

/*-----------------------------------------------------------------------------
  JAC_default_params

  Initialize the default JAC and SVA parameters used for adaptation of
  models to present noise and channel conditions.  These may
  subsequently changed by the API.

  Arguments:

  gv: Pointer to the ASR structure holding recognition state information
  --------------------------------*/
TIesrEngineJACStatusType JAC_set_default_params( gmhmm_type *gv ); 


/*-----------------------------------------------------------------------------
 JAC_load
  
 Load the prior jac channel estimate, global accumulator, and 
 sva accumulator values so that jac estimation can continue from
 prior utterances.

 Arguments:
 bias_file: JAC state information file
 gv: Pointer to the ASR structure holding recognition state information
 pNetDir: Directory holding grammar hmm and grammar network info
 pPhoneticTreeFile: Full name of file containing hierarchical regression tree
  --------------------------------*/
TIesrEngineJACStatusType JAC_load (const char *bias_file, gmhmm_type *gv,
				   const char* pNetDir, const char* pPhoneticTreeFile);


/*-----------------------------------------------------------------------------
  JAC_save

  Save the jac channel estimate, global accumulators and sva accumulators
  to a file, so that jac estimation can continue in the future from 
  the present accumulated values.

  Arguments:
  bias_file: JAC state information file
  gv: Pointer to the ASR structure holding recognition state information
  --------------------------------*/
TIesrEngineJACStatusType JAC_save(const char *bias_file, gmhmm_type *gv );


/*-----------------------------------------------------------------------------
  JAC_clear

  Clear the jac channel estimate, global accumulators, and sva accumulators
  so that jac estimation starts afresh with the next utterance.

  Arguments:
  gv: Pointer to the ASR structure holding recognition state information
  aModelDir: Directory containing current HMM model and grammar network files
  aTreeFile: File containing regression tree for SBC.
  --------------------------------*/
/* GMHMM_SI_API */
TIesrEngineJACStatusType JAC_clear(gmhmm_type *gv, 
				   const char* aModelDir, 
				   const char* aTreeFile ); 


/*-----------------------------------------------------------------------------
  JAC_update

  Update the jac channel, global accumulators, and sva accumulators 
  with the results of the presently recognized utterance.  The updated
  jac channel and sva corrections can be used for recognition of 
  subsequent utterances.

  Arguments:
  gv: Pointer to the ASR structure holding recognition state information
  --------------------------------*/
/* GMHMM_SI_API */ 
TIesrEngineJACStatusType JAC_update(gmhmm_type *gv);

#endif
