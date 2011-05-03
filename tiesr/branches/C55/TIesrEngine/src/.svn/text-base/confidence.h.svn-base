/*=======================================================================
 
 *
 * confidence.h
 *
 * Header for confidence functionality implementation.
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
======================================================================*/

#ifndef _CONFIDENCE_H
#define _CONFIDENCE_H

#include "tiesr_config.h"

#define CM_UPDATE_MEAN /* online update of mean of the hypothesis models */
//#define CM_UPDATE_VAR /* online update of variance of the hypothesis models */


#define CM_MAX_FRM 500
#define CM_MAX_WORD 10
#define CM_SCRS 5 /* number of computed measures for confidence measure */

#define  CM_NBR_BETA 3

#ifdef USE_CONFIDENCE

/* ----------------------- trained UV model parameters -----------------------
   CM_UV_ACC and CM_UV_REJ are mean, inv, and gconst of the acceptance/rejection models
   for the feature of LLR between the best path and the accumulated maximum likelihood 
   score. LLR is on speech segments. 
   [mean, inv, gconst] with Q11, Q9, and Q6; 
*/

#ifndef C55x
const short CM_UV_ACC[3];
const short CM_UV_REJ[3];
const short CM_LOGIST_PARA[CM_NBR_BETA];
const short CM_NBEST_LOW_CONF;
#else 
extern const short CM_UV_ACC[3];
extern const short CM_UV_REJ[3];
extern const short CM_LOGIST_PARA[CM_NBR_BETA];
/* trained from OOV, IV in 8-types of Aurora noise, TIMIT noise in 10dB, and WAVES noise. */
//const short CM_LOGIST_PARA[CM_NBR_BETA] = {296, 64, -97}; // triained from OOV in WAVES noise
extern const short CM_NBEST_LOW_CONF;
#endif //C55x
/* the confidence score for those utterances have too many N-bests */

#endif

#endif
