/*=======================================================================

 *
 * jac_one_user.h
 *
 * Header for jac processing interface.
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


 This header defines the interface with the jac_one functions that
 implement part of the jac functionality.

======================================================================*/

#ifndef _JAC_ONE_USER_H
#define _JAC_ONE_USER_H

#include "tiesr_config.h"
#include "status.h"
#include "gmhmm_type.h"


void GetTIesrJacRate(gmhmm_type *gvv, short *number_mu_per_frame);

void SetTIesrJacRate(gmhmm_type *gvv, short number_mu_per_frame);

TIesrEngineJACStatusType per_frame_jac_compensate(short mfcc_buf[], gmhmm_type *gvv);

void jac_one_mean(gmhmm_type *gv, ushort index_mean);

#endif
