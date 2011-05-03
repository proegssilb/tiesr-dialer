/*=======================================================================
 obsprob.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


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

#ifdef BIT8FEAT

#else

extern short *mem_mfcc;
extern short *mem_d_mfcc;

/*
** this could be avoided by usinge DIM-dim mfcc (currently DIM/2)
*/
void copy_feature(short feature[], ushort crt_vec, short n_mfcc)
{
  ushort d;

  for (d = 0; d < n_mfcc; d++) { /* copy static and dynamic */
    feature[d] = mem_mfcc[ crt_vec + d];
    feature[d + n_mfcc] = mem_d_mfcc [crt_vec + d];
  }
}
#endif

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
  short mfcc_feature[ MAX_DIM ]; /* mfcc vector */
  
  FOR_EMS_STATES(i,hmm,gv->base_tran) { 
    pdf_i = GET_BJ_IDX(hmm,i);
    for (t=0, crt_vec = seg_stt * n_mfcc; t<T; t++, crt_vec += n_mfcc) {
#ifdef BIT8FEAT
      vector_unpacking(gv->mem_feature + crt_vec, mfcc_feature, gv->scale_feat, n_mfcc); 
#else
      copy_feature(mfcc_feature, crt_vec, n_mfcc);
#endif
      pdf[t] = hlr_gauss_obs_score_f(mfcc_feature, (int)pdf_i, FULL, &x, nbr_dim, gv);
    }
    pdf += T;
  }
}

