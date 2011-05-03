/*=======================================================================
 sbc_struct_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 
 This header file defines the FeaHLRAccType structure used to contain
 the SBC state.  This structure is defined separately from the
 sbc_user.h file since the sbc_user.h file also uses gmhmm_type.h,
 and gmhmm_type.h includes the FeaHLRAccType structure.  (This is part
 of the lack of modularity of TIesr at this time.)

======================================================================*/

#ifndef _SBC_STRUCT_USER_H
#define _SBC_STRUCT_USER_H

#include "tiesr_config.h"
#include "tiesrcommonmacros.h"

/* Memory required to store sbc transform management data */
#define SBC_SIZE 23000


/*--------------------------------
  Structures defined and used by the interface
  --------------------------------*/

typedef struct { 
  short   base_mem[SBC_SIZE];
  ushort mem_count; /* memory top level index */
  ushort max_mem_usage; /* max size */
  
  ushort n_terminals;

  ushort n_monophones;

  /* HLR globals */
  /* HLR regression Tree memory info */
  short *hlrTreeBase;
  /* The monophone index to tree terminal node index */
  ushort *hlrTerminal;
  
  /* pointer to hmm2phone index */
  short *pHMM2phone; 

  /* mapping from terminal tree nodes to transforms */
  ushort *hlrTransformIndex; // [0..n_nodes] index of transformation by hlrTransform
  /* pointer to the index of transformation for phones */
  ushort hlr_TRANSFORM_SIZE;

  short *hlrTransforms; /* Q11 */
  
  short* pSegStat_dif1; /* 1st order statistics of this segment , Q11 */
  short* pSegStat_dif2; /* 2nd order statistics of this segment , Q9 */

  short *pEnhanced;    /*  space allocated for enhanced feature at current frame by bias removal */
  /* should have 97*16*2 = 3104 byte = 1052 short*/

  /* ------------- for confidence driven adaptation -------------- */
#ifdef USE_CONF_DRV_ADP
  short * bk_node_cnt; /* node count */
#endif

} FeaHLRAccType ;

#endif
