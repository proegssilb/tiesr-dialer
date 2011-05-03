/*=======================================================================
 jac_one_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


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
