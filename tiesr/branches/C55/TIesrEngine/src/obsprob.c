/*=======================================================================

 *
 * obsprob.cpp
 *
 * State observation likelihood calculation.
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


 HMM state observation likelihood calculation.
 
======================================================================*/

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "hlr_dist.h"
#include "pack.h"
*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"
#include "obsprob_user.h"
#include "pack_user.h"
#include "dist_user.h"



/*
** MFCC the whole file
*/

/*
** compute pdf and for one state pdf.
*/

void observation_likelihood(ushort seg_stt, short T, HmmType *hmm, short pdf[], short n_mfcc, gmhmm_type *gv)
{
  ushort t, i, crt_vec;        
  short x, pdf_i, nbr_dim = n_mfcc * 2;

#ifdef BIT8FEAT
   short mfcc_feature[ MAX_DIM ]; /* mfcc vector */
   int feat_size = n_mfcc;
#else
   short *mfcc_feature;
   int feat_size = gv->nbr_dim;
#endif
  
  FOR_EMS_STATES(i,hmm,gv->base_tran) { 
    pdf_i = GET_BJ_IDX(hmm,i);
    for (t=0, crt_vec = seg_stt * feat_size; t<T; t++, crt_vec += feat_size) {
#ifdef BIT8FEAT
      vector_unpacking(gv->mem_feature + crt_vec, mfcc_feature, gv->scale_feat, n_mfcc); 
#else
      mfcc_feature = (short *)gv->mem_feature + crt_vec;
#endif
      pdf[t] = hlr_gauss_obs_score_f(mfcc_feature, (int)pdf_i, FULL, &x, nbr_dim, gv);
    }
    pdf += T;
  }
}

