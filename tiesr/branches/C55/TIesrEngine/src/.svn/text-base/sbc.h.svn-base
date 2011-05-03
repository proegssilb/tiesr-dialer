/*=======================================================================

 *
 * sbc.h
 *
 * Header for implementation of stochastic bias compensation.
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
 
 Header for internal components of stochastic bias compensation. 
======================================================================*/


#ifndef _SBC_H
#define _SBC_H

#include "tiesr_config.h"
#include "tiesrcommonmacros.h"

#include "sbc_user.h"

/* Use malloc to dynamic allocate space because n_hmms is varying, 
   it may be better to do so. */
#define _DYN_ALLOC_SBC 1

/*--------------------------------
  Constants used by the method, not subject to change with different applications.
  --------------------------------*/


static const unsigned short DIF1_PREC = 11;

static const unsigned short DIF2_PREC = 9;
/* 
   If the above precisions are changed, need to check at least the following command
   div = (short) div32_32_Q(dif1[jx], dif2[jx]<<iprec, MEAN_PREC);
   in sbc.cpp.
*/

static const unsigned short MEAN_PREC = 11;

static const unsigned short MAX_TRANSFOM_MEM = 5000;

static const short SBC_MINTOKENS = 700; 


#define SBC_MAX_MONOPHONES 75


/* Parameter defaults of SBC */

/* 0.8 in Q15 */
static const unsigned short SBC_FGT_DEFAULT = 26214;


/*--------------------------------
  Structures used locally within SBC
  --------------------------------*/

/* SBC node structure */

typedef struct{
  /* pointer to the allocated sufficient statistics for SBC */
  long * p_acc_dif1 ; /* Q DIF1_PREC */
  long * p_acc_dif2 ; /* Q DIF2_PREC */

  ushort vector_count; 
  short parent_node_and_transform_idx; /* the upper byte contains parent node
					  the lower byte contains transform index */
}sbcTreeNode, *psbcTreeNode;


/*  SBC Tree structure */

typedef struct
{
  psbcTreeNode *node; 
  ushort n_nodes;
  ushort n_utt; /* number of utterances used for SBC updating and adaptation */
} sbcTree;

/* Set this flag refresh SBC for every 128 utterances.  This completely resets
   all transformations. */
//#define SBC_REFRESH

#ifdef SBC_PRIOR

/* prior inverse variance of SBC vector, 
   in Q11 */
const short SBC_PRIOR_INVVAR[20] = {23331, 13709, 27631, 16671, 17975, 
				    28147, 6042, 6384, 10980, 13018,
				    32508, 32508, 32508, 32508, 32508, 
				    32508, 32508, 32508, 32508, 32508};
#endif

/*--------------------------------
  Macros used locally
  --------------------------------*/

/*--------------------------------
  Local function prototypes
  --------------------------------*/

/* Division of two longs, and return long with specific Q precision.
   This should be defined within a math core API */

//static long div32_32_Q(long num, long den, ushort q_prec);

/**
   given hmm_idx, obtain corresponding bias removal vector 
   @param hmm_idx hmm code
   @param sa pointer to FeaHLRAccType
   return transformation index */
static ushort sbc_phone2transidx(ushort hmm_idx, FeaHLRAccType * sa);

#ifdef _SBC_DEBUG

void sbc_dbg_dump(FeaHLRAccType *sa);
void sbc_dump(FeaHLRAccType *sa, ushort inode);

#endif

#endif
