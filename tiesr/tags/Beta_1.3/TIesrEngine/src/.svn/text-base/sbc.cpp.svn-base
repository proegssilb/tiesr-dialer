/*=======================================================================

 *
 * sbc.cpp
 *
 * Stochastic bias compensation.
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

 Implementation of stochastic bias compensation (SBC)

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "sbc_user.h"
#include "sbc.h"
#include "noise_sub_user.h"
#include "stdlib.h"
#include "search_user.h"
#include "gmhmm_type.h"
#include "dist_user.h"
#include "pack_user.h"
#include "load_user.h"


//#define _SBC_DEBUG
#ifdef _SBC_DEBUG
static void sbc_assert(char expr, short dim){
  if (expr == 0) {
    PRT_ERR(fprintf(stdout, "overflow at dif2[%d]\n", dim));
  }
}
#endif


/*--------------------------------*/
static char sbc_is_marked(short a)
{
  short b; 
  b = a & 0x8000;
  if (b != 0) return TRUE;
  else return FALSE;
}


/*--------------------------------*/
static ushort sbc_mark(ushort* a, FeaHLRAccType* sa)
{
  short ix;

  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  ushort b = *a; 
  /* check all terminal nodes that have the same transformation index */
  if (sbc_is_marked(*a)) return *a; 
  for (ix = tree->n_nodes - 1;ix>=0;ix--){
    if (sa->hlrTransformIndex[ix] == b){
      sa->hlrTransformIndex[ix] |= 0x8000;
    }
  }
  return b;
}


/*--------------------------------*/
static void sbc_unmark(FeaHLRAccType* sa)
{
  short ix ; 
  if ( sa == NULL || sa->hlrTreeBase == NULL ) return;
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  for (ix=tree->n_nodes - 1; ix >= 0; ix--)
    sa->hlrTransformIndex[ix] &= 0x7fff;
}


/*--------------------------------*/
short sbc_phone2terminalidx(ushort hmm_idx, FeaHLRAccType * sa)
{
  short monophone_index; 
  short terminal_index; 

  monophone_index = *( sa->pHMM2phone + hmm_idx ); 
  monophone_index++; 

  terminal_index = (short) sa->hlrTerminal[ monophone_index ];
  return terminal_index;
}

/*--------------------------------*/
static ushort sbc_parent_is(sbcTreeNode* node){
  ushort a;
  a = node->parent_node_and_transform_idx & 0xff00;
  a = a >> 8;
  return a;
}


/*--------------------------------*/
/* Code only used in sbc_dump */
#ifdef _SBC_DEBUG
static ushort sbc_tranform_idx_is(sbcTreeNode* node){
  ushort a;
  a = node->parent_node_and_transform_idx & 0xff;
  return a;
}
#endif

/*--------------------------------*/
static sbcTreeNode* sbc_get_tree_node(ushort inodeidx, FeaHLRAccType* sa)
{
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase; 

  if (inodeidx < tree->n_nodes){
    return tree->node[inodeidx];
  }
  else
    return NULL;
}


/*--------------------------------*/
static ushort sbc_get_reliable_node(ushort iTerminalIdx, FeaHLRAccType* sa)
{
  ushort i = iTerminalIdx, iparent;
  sbcTree *tree = (sbcTree*) sa->hlrTreeBase;

  while(tree->node[i]->vector_count < SBC_MINTOKENS &&
	i < tree->n_nodes){
    iparent = sbc_parent_is(tree->node[i]);
    if (iparent >= tree->n_nodes) break; 
    i = iparent;
  }
  return i; 
}


/*--------------------------------
   Find the transformation index corresponding to the hmm of hmm_index.
   The physical hmm index is used to determine the monophone index, 
   and thus define the mapping of the hmm to the hlr tree terminal 
   node.
   
   The sil model has monophone index of -1, so
   the monophone index is incremented to have zero-based
   indices into the hlrTerminal array.
   --------------------------------*/

/* Code not used at this time */
/*
static ushort sbc_set_phone2transformidx(ushort hmm_code, FeaHLRAccType *sa)
{
  ushort transform_index; 
  short terminal_index = sbc_phone2terminalidx(hmm_code, sa); 
  
  transform_index = sbc_get_reliable_node(terminal_index, sa); 

  sa->hlrTransformIndex[terminal_index] = transform_index; 
  
  return transform_index; 
}
*/

/*--------------------------------*/
static ushort sbc_phone2transidx(ushort hmm_idx, FeaHLRAccType * sa)
{
  ushort transform_index; 
  short terminal_index = sbc_phone2terminalidx(hmm_idx, sa); 

  transform_index = sa->hlrTransformIndex[terminal_index];

  return transform_index ; 
}


/*--------------------------------*/
static void sbc_nbr_of_vec_under_the_node(short iTerminalIdx, 
					  short seg_length,
					  FeaHLRAccType* sa)
{
  ushort ascend, i = iTerminalIdx; 
  sbcTree*   tree = (sbcTree *)sa->hlrTreeBase;

  ascend = iTerminalIdx;
  while(ascend < tree->n_nodes){
    ascend = sbc_parent_is(tree->node[i]); 
    if (ascend >= tree->n_nodes) break; 
    tree->node[ascend]->vector_count += seg_length; //tree->node[i]->vector_count; 
    /* to protect from overflow */
    tree->node[ascend]->vector_count = MIN(tree->node[ascend]->vector_count,
					   SBC_MINTOKENS);
    i = ascend; 
  }
}


/*--------------------------------*/
static ushort sbc_map_to_hiera(ushort hmm_code, FeaHLRAccType* sa, 
			       ushort seg_length, 
			       char bUpdate)
{
  short iTerminalIdx = sbc_phone2terminalidx(hmm_code, sa);
  sbcTreeNode * node = sbc_get_tree_node(iTerminalIdx, sa); 
  if (bUpdate) {
    node->vector_count += seg_length;
    node->vector_count = MIN(node->vector_count,
			     SBC_MINTOKENS);
  }
  return iTerminalIdx; 
}


/*--------------------------------*/
static ushort sbc_determine_reliable_node(ushort hmm_code, FeaHLRAccType* sa, 
					  ushort useglength, char bUpdate)
{
  ushort inode; 
  short iTerminalIdx = sbc_map_to_hiera(hmm_code, sa, useglength, bUpdate);

  if (bUpdate) sbc_nbr_of_vec_under_the_node(iTerminalIdx, useglength , sa);

  inode = sbc_get_reliable_node(iTerminalIdx, sa);

  return iTerminalIdx; 
}


/*--------------------------------*/
static long * sbc_get_transform_stat_dif1(ushort iNode, FeaHLRAccType* sa)
{
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  sbcTreeNode* p_node; 

  p_node = tree->node[iNode];
  return p_node->p_acc_dif1; 
}


/*--------------------------------*/
static long * sbc_get_transform_stat_dif2(ushort iNode, FeaHLRAccType* sa)
{
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  sbcTreeNode* p_node; 

  p_node = tree->node[iNode];
  return p_node->p_acc_dif2; 
}


/*--------------------------------*/
static short * sbc_get_transform(ushort i_txm_idx, FeaHLRAccType* sa)
{
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  ushort ioffset = i_txm_idx * sa->hlr_TRANSFORM_SIZE ; 
  if (i_txm_idx >= tree->n_nodes) return NULL;
  return &sa->hlrTransforms[ioffset]; 
}



/*--------------------------------*/
static void sbc_update_SBC_phone2trans( gmhmm_type* gv)
{
  short s;
  ushort t_index; 
  FeaHLRAccType* sa = gv->sbcState;
  
  for (s=sa->n_terminals-1;s>=0;s--){
    t_index = sbc_get_reliable_node(s, sa); 
    sa->hlrTransformIndex[s] = t_index;
  }
}

/* return pointer to the HMM2phone */
short * sbc_rtn_hmm2phone(FeaHLRAccType *sa)
{
  if (sa)
    return sa->pHMM2phone; 
  else return NULL;
}

/*--------------------------------
  @param bTriphone: TRUE if the HMM2phone maps triphone to monophone
                    FALSE if the HMM2phone maps monophone to itself
*/
TIESRENGINECOREAPI_API
TIesrEngineHLRStatusType sbc_load_hmm2phone(const char gdir[], 
					    ushort n_hmms, 
					    FeaHLRAccType *sa,
					    Boolean bTriphone )
{
   char fname[ MAX_STR ];
   size_t nread;
   short si; 
   FILE *fp;

#ifdef _DYN_ALLOC_SBC
   if( sa->pHMM2phone )
   {
      free( sa->pHMM2phone );
      sa->pHMM2phone = NULL;
   }
#endif

   /* !! load the hmm to monophone mapping; should be done in gmhmm */
   //   extern short     *base_net;

#ifndef _DYN_ALLOC_SBC
   sa->pHMM2phone = (short*) mem_alloc( sa->base_mem, &sa->mem_count, 
					n_hmms, SBC_SIZE, SHORTALIGN, 
					"HMM to phone index" );
   if( sa->pHMM2phone == NULL )
      return eTIesrEngineHLRMemoryBase;
#else

   /* use dynamic memory allocation, because n_hmms is varying */
   sa->pHMM2phone = (short*) malloc(n_hmms * sizeof(short));

   if( sa->pHMM2phone == NULL )
      return eTIesrEngineHLRFail;

#endif

   /* Open the hmm to phone mapping file */
   if (bTriphone){
     sprintf(fname,"%s/%s",gdir,"hmm2phone.bin");
     
     if (  (fp = fopen( fname, "rb" )) == NULL )
       {
#ifdef _DYN_ALLOC_SBC
	 free( sa->pHMM2phone );
	 sa->pHMM2phone = NULL;
#endif
         return eTIesrEngineHLRFail;
       }

     nread = fread( sa->pHMM2phone, sizeof(short), n_hmms, fp );
     fclose( fp );

   /*   if( nread != n_hmms )
	{
	#ifdef _DYN_ALLOC_SBC
	free( sa->pHMM2phone );
	#endif
	return eTIesrEngineHLRFail;
	}
	modified on Feb/15/2006 */
     if (nread == 0){
#ifdef _DYN_ALLOC_SBC
       free( sa->pHMM2phone );
#endif
       return eTIesrEngineHLRFail;
     }
   }else{
     for (si=0;si< n_hmms; si++)
       sa->pHMM2phone[si] = si - 1; 
   }

   return eTIesrEngineHLRSuccess;
}

/*--------------------------------*/
static FeaHLRAccType* sbc_set_parent_node(ushort inodeidx, ushort parent_node, 
					  FeaHLRAccType* sa){
  sbcTreeNode * node; 
  ushort pp; 

  node = sbc_get_tree_node(inodeidx, sa);
  pp = parent_node << 8;
  node->parent_node_and_transform_idx =
    (node->parent_node_and_transform_idx & 0x00ff) + pp;
  return sa;
}
  

/*--------------------------------*/
static FeaHLRAccType* sbc_set_transform_idx(ushort inodeidx, ushort transform_idx, 
					    FeaHLRAccType* sa){
  sbcTreeNode * node; 
  ushort pp; 

  node = sbc_get_tree_node(inodeidx, sa);
  pp = 0x00ff & transform_idx; 
  node->parent_node_and_transform_idx =
    (node->parent_node_and_transform_idx & 0xff00) + pp;
  return sa;
}


/*--------------------------------
  sbc_open

  This function clears the contents of the FeaHLRAccType structure
  that holds the SBC state information.
  --------------------------------*/
TIESRENGINECOREAPI_API void sbc_open( FeaHLRAccType *sa )
{
  sa->mem_count = 0;
  sa->max_mem_usage = SBC_SIZE;

  sa->n_terminals = 0;
  sa->hlrTreeBase = NULL;
  sa->hlrTerminal = NULL;
  sa->pHMM2phone = NULL;
  sa->hlr_TRANSFORM_SIZE = 0;
  sa->hlrTransforms = NULL;
  sa->pSegStat_dif1 = NULL;
  sa->pSegStat_dif2 = NULL;
  sa->pEnhanced = NULL;

  return;
}


/*--------------------------------*/
static short* sbc_get_enhanced(FeaHLRAccType *pSBC, 
		       ushort itrans)
{
  ushort ix;
  sbcTree* tree = (sbcTree*) pSBC->hlrTreeBase;

  if (itrans >= tree->n_nodes) return NULL;
  ix = itrans * pSBC->hlr_TRANSFORM_SIZE; 
  return &(pSBC->pEnhanced[ix]); 
}

/*--------------------------------*/
void sbc_backup(FeaHLRAccType * sa)
{
#ifdef USE_CONF_DRV_ADP
  sbcTree* tree = (sbcTree *)sa->hlrTreeBase;
  register short i; 
  for (i= tree->n_nodes - 1; i>=0; i--)
    sa->bk_node_cnt[i] = tree->node[i]->vector_count; 
#endif
}

/*--------------------------------*/
void sbc_restore(FeaHLRAccType * sa)
{
#ifdef USE_CONF_DRV_ADP
  sbcTree* tree = (sbcTree *)sa->hlrTreeBase;
  register short i; 
  for (i= tree->n_nodes - 1; i>=0; i--)
    tree->node[i]->vector_count = sa->bk_node_cnt[i];
#endif
}

TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_save(FeaHLRAccType* sa, FILE *pf)
{
   size_t nwrite;
   short ix, *pShortObj, *p_trans, cep_dim = sa->hlr_TRANSFORM_SIZE; 
   long  *dif1, *dif2; 
   sbcTree* tree = (sbcTree *)sa->hlrTreeBase;
#ifdef _DYN_ALLOC_SBC
  if (sa->pHMM2phone) free(sa->pHMM2phone);
  sa->pHMM2phone = NULL;
#endif

  if (sa && pf){
    fwrite(&(tree->n_utt), sizeof(ushort), 1, pf);
    nwrite = fwrite(&(tree->n_nodes), sizeof(ushort), 1, pf);
    if( nwrite != 1 )
      return eTIesrEngineHLRFail;
    nwrite = fwrite(&(sa->hlr_TRANSFORM_SIZE), sizeof(ushort), 1, pf);
    if( nwrite != 1 )
      return eTIesrEngineHLRFail;
    
    for (ix=0;ix<tree->n_nodes;ix++){
      fwrite(&(tree->node[ix]->vector_count), sizeof(ushort), 1, pf);
      fwrite(&(tree->node[ix]->parent_node_and_transform_idx), sizeof(short), 1, pf);
      p_trans = sbc_get_transform(ix, sa); 
      dif1 = sbc_get_transform_stat_dif1(ix, sa); 
      dif2 = sbc_get_transform_stat_dif2(ix, sa); 
      fwrite(dif1, sizeof(long), cep_dim, pf); 
      fwrite(dif2, sizeof(long), cep_dim, pf); 
      fwrite(p_trans, sizeof(short), cep_dim,pf);
    }
    
    /* save Terminal transformation index information */
    fwrite(&(sa->n_monophones), sizeof(ushort), 1, pf);
    nwrite = fwrite(sa->hlrTerminal, sizeof(ushort), sa->n_monophones, pf); 
    if( nwrite != sa->n_monophones) return eTIesrEngineHLRFail;
    
    nwrite = fwrite(sa->hlrTransformIndex, sizeof(ushort), tree->n_nodes, pf); 
    if (nwrite != tree->n_nodes) return  eTIesrEngineHLRFail;
    
    pShortObj = sa->hlrTransforms; 
    nwrite = fwrite(pShortObj, sizeof(short), 
		    tree->n_nodes * sa->hlr_TRANSFORM_SIZE, pf); 
    if (nwrite != (size_t)tree->n_nodes * sa->hlr_TRANSFORM_SIZE ) 
      return  eTIesrEngineHLRFail;
  }
  
  return eTIesrEngineHLRFail; 
}

/*--------------------------------

TIesrEngineHLRStatusType sbc_save(FeaHLRAccType* sa , FILE* pf)
{
   size_t nwrite;

#ifdef _DYN_ALLOC_SBC
  if (sa->pHMM2phone) free(sa->pHMM2phone);
  sa->pHMM2phone = NULL;
#endif

  if (sa && pf){
     nwrite = fwrite(&(sa->mem_count), sizeof(ushort), 1, pf);
     if( nwrite != 1 )
	return eTIesrEngineHLRFail;

     nwrite = fwrite(sa->hlrTreeBase, sizeof(short), sa->mem_count, pf);
     if( nwrite == sa->mem_count )
	return eTIesrEngineHLRSuccess;
  }

  return eTIesrEngineHLRFail;
}
*/

TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_load(FeaHLRAccType* sa, FILE *pf)
{
   size_t nread;
   short *pShortObj, sNodes, sCepDim, ix, *p_trans; 
   ushort utmp; 
   long *dif1, *dif2; 
   sbcTree* tree = (sbcTree *)sa->hlrTreeBase;
   Boolean bOverWrite = TRUE; /* if overwrite the original values using the saved values */

   if (sa && pf){
      if (!feof(pf)){
	nread = fread(&utmp, sizeof(ushort), 1, pf);
	if (nread!= 1) return eTIesrEngineHLRLoadTree;

	utmp ++; 
	tree->n_utt = utmp;
	
#ifdef SBC_REFRESH
	/* refresh SBC for every 128 utterances */
	if ((utmp >> 7) != 0){
	  tree->n_utt = 0; 
	  bOverWrite = FALSE ; 
	}
#endif

	nread = fread(&(sNodes), sizeof(ushort), 1, pf);
	if( nread != 1 )
	  return eTIesrEngineHLRLoadTree;
	if (sNodes != tree->n_nodes) return eTIesrEngineHLRLoadTree;
	  
	nread = fread(&(sCepDim), sizeof(short), 1, pf);
	if (nread != 1)
	  return eTIesrEngineHLRLoadTree;
	if (sCepDim != sa->hlr_TRANSFORM_SIZE) return eTIesrEngineHLRLoadTree;
	  
	for (ix=0;ix<tree->n_nodes;ix++){
	  if (bOverWrite) {
	    nread = fread(&(tree->node[ix]->vector_count), sizeof(ushort), 1, pf);
	    if (nread != 1) return eTIesrEngineHLRLoadTree;
	  }
	  else 
	    fseek(pf, sizeof(ushort), SEEK_CUR); 

	  if (bOverWrite) {
	    nread = fread(&(tree->node[ix]->parent_node_and_transform_idx), 
			  sizeof(short), 1, pf);
	    if (nread != 1) return eTIesrEngineHLRLoadTree;
	  }else 
	    fseek(pf, sizeof(short), SEEK_CUR); 

	  if (bOverWrite){
	    dif1 = sbc_get_transform_stat_dif1(ix, sa); 
	    dif2 = sbc_get_transform_stat_dif2(ix, sa); 
	    p_trans = sbc_get_transform(ix, sa); 
	    nread = fread(dif1, sizeof(long), sCepDim, pf); 
	    if (nread != (size_t)sCepDim) return eTIesrEngineHLRLoadTree;
	    nread = fread(dif2, sizeof(long), sCepDim, pf); 
	    if (nread != (size_t)sCepDim) return eTIesrEngineHLRLoadTree;
	    nread = fread(p_trans, sizeof(short), sCepDim, pf);
	    if (nread != (size_t)sCepDim) return eTIesrEngineHLRLoadTree;
	  }else
	    fseek(pf, sizeof(long)*(sCepDim << 1) + sizeof(short)*sCepDim, SEEK_CUR); 
	}
	  
	if (bOverWrite){
	  /* read Terminal transformation index information */
	  fread(&(sa->n_monophones), sizeof(ushort), 1, pf); 
	  nread = fread(sa->hlrTerminal, sizeof(ushort), sa->n_monophones, pf); 
	  if( nread != sa->n_monophones) return eTIesrEngineHLRLoadTree;
	  
	  nread = fread(sa->hlrTransformIndex, sizeof(ushort), tree->n_nodes, pf); 
	  if (nread != tree->n_nodes) return  eTIesrEngineHLRLoadTree;
	  
	  pShortObj = sa->hlrTransforms; 
	  nread = fread(pShortObj, sizeof(short), 
			tree->n_nodes * sa->hlr_TRANSFORM_SIZE, pf); 
	  if (nread != (size_t) tree->n_nodes * sa->hlr_TRANSFORM_SIZE) 
	    return eTIesrEngineHLRLoadTree;
	}else
	  fseek(pf, sizeof(short)*(1 + sa->n_monophones + tree->n_nodes + tree->n_nodes*sa->hlr_TRANSFORM_SIZE), SEEK_CUR); 

	return eTIesrEngineHLRSuccess;
      }
   }
   return eTIesrEngineHLRLoadTree;
}

#ifdef _SBC_DEBUG
void sbc_dump_each_transform(FeaHLRAccType *sa, ushort inode)
{
  ushort ix, jx; 
  short    *ptrans; 
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  sbcTreeNode* p_node; 

  ix = inode;
  p_node = tree->node[ix];
  if (tree->node[ix]) {
    printf("node[%d] : ", inode);
    ptrans = sbc_get_transform(inode, sa);
    for (ix = 0;ix<20;ix++)
      printf("%d ", ptrans[ix]);
  }
}

void sbc_dump_transform(FeaHLRAccType *sa)
{
  short i; 
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;

  for( i = 0; i <  tree->n_nodes; i++)
    {
      sbc_dump_each_transform(sa, i); 
    }
  printf("\n");
}

/* a^b for both a and b are Q0 */
static short sPOW(short a, short b){
  short x;
  if (b > 0) 
    x = a*sPOW(a,b-1);
  else x = 1; 
  
  return x; 
}

/*--------------------------------
  sbc_dump is only used during debug, so is commented out now.
  --------------------------------*/

void sbc_dump(FeaHLRAccType *sa, ushort inode)
{
  ushort ix, jx; 
  short QDIF1 = sPOW(2, DIF1_PREC), QDIF2 = sPOW(2, DIF2_PREC), div,
    *ptrans; 
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  sbcTreeNode* p_node; 
  long* dif1, *dif2;

  /*  if (sa->hlrTerminal){
    for (ix=0;ix<sa->n_terminals;ix++){
      fprintf( stdout, "%d ", (short) sa->hlrTerminal[ix]); 
    }
    fprintf(stdout, "\n");
  }
  */

  ix = inode;
  p_node = tree->node[ix];
  if (tree->node[ix]) {
    fprintf( stdout, "node %d\n", ix);
#ifndef SBC_PRIOR
    fprintf( stdout, "vec count = %d, parent = %d, trans idx = %d\n ", 
	     tree->node[ix]->vector_count, 
	     sbc_parent_is(tree->node[ix]), 
	     sbc_tranform_idx_is(tree->node[ix]));
    
    dif1 = sbc_get_transform_stat_dif1(ix, sa); 
    dif2 = sbc_get_transform_stat_dif2(ix, sa); 
    fprintf(stdout, "dif1\n");
    for (ix=0;ix<16;ix++){
      fprintf(stdout, "%5.4f\t", (float)dif1[ix]/QDIF1);
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "dif2\n");
    for (ix=0;ix<16;ix++){
      fprintf(stdout, "%5.4f\t", (float)dif2[ix]/QDIF2);
    }
    fprintf(stdout, "\n");
#else
    printf("node[%d] : ", inode);
    ptrans = sbc_get_transform(inode, sa);
    for (ix = 0;ix<20;ix++)
      printf("%d ", ptrans[ix]);
#endif

#ifndef SBC_PRIOR
    if (sa->hlrTransforms){
      fprintf(stdout, "bias\n");
      jx = sbc_tranform_idx_is(p_node);
      for (ix=0;ix<16;ix++){
	div = (short) div32_32_Q(dif1[ix], 
				 dif2[ix]<<(DIF1_PREC - DIF2_PREC),
				 MEAN_PREC);
	fprintf(stdout, "%5.4f\t", (float)div/MEAN_PREC);
      }
      fprintf(stdout, "\n");
    }
#endif
  }
}

void sbc_dbg_dump(FeaHLRAccType *sa)
{
  short i; 
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;

  for( i = 0; i <  tree->n_nodes; i++)
    {
      sbc_dump(sa, i); 
    }
}


#endif
/*--------------------------------*/
TIESRENGINECOREAPI_API void sbc_rst_SBC_stat(FeaHLRAccType *sa)
{
  sbc_unmark(sa); 
}


/*--------------------------------*/
TIESRENGINECOREAPI_API void sbc_rst_seg_stat(FeaHLRAccType* sa)
{
  short ix;
  if (sa == NULL || sa->hlrTreeBase == NULL ) return; 
  for (ix=sa->hlr_TRANSFORM_SIZE-1;ix>=0;ix--){
    sa->pSegStat_dif1[ix] = 0;
    sa->pSegStat_dif2[ix] = 0;
  }
}

#ifdef SBC_PRIOR 

static short sbc_prior_invvar(short dim, short cep_dim)
{
  /* support 20 dimensional MFCC + delta MFCC */
  /* support 18 dimensional MFCC + delta MFCC */

  if (cep_dim == 18 && dim >= 9) 
    return SBC_PRIOR_INVVAR[dim + 1];
  else {
    if (dim < 20)
      return SBC_PRIOR_INVVAR[dim]; 
    else
      return SBC_PRIOR_INVVAR[19];
  }  
}

#endif

/*================================================================
  sbc_init_tree (from hlr_init_tree)
   
  This function loads the structure of the regression tree, and the
  monophone to tree terminal node array. Note that it must be
  compatible with the monophone index assignment in flexphone.
   
  Notes:
   
  The hlr information is as follows: The first unsigned short is the
  number of terminal nodes in the binary tree.  ( In concept, each
  terminal node corresponds to one of the monophones in the flexphone
  list, but this is not really necessary. The only thing required is
  a mapping of phone models to terminal nodes of the tree. )  This is
  followed by the tree structure, which consists of a listing of the
  parent node number of each of the 2*number_terminal_nodes - 1 nodes
  in the tree.  The order of nodes in the binary tree is such that
  all terminal nodes are listed first, and progression is from left
  to right and lower to higher branches in the tree, with no branches
  overlapping.  This makes it possible to simplify tree operations.

  The next entry in the binary file is the number of monophones in
  the underlying phonetic model set, including silence.  This is
  followed by an index of the terminal node of the tree for each of
  the monophones, in the order in which the monophones are indexed in
  the underlying model set.

  Memory is allocated from base memory which is a short array. Some data
  allocated is long data.  It is assumed that long data and short data
  are powers of two bytes in size.

  if (load_monophone_only) then memory for tree is not allocated. 

  @param bTriphone: TRUE if maps triphone to monophone
                  FALSE if maps monophone to itself
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_init_tree( FeaHLRAccType *sa,
					const char* fnDir, 
					const char *fname, 
					short load_monophone_only,
					ushort cep_dim, ushort n_hmms,
					Boolean bTriphone )
{
   FILE *fp;
   ushort n_monophones, p_node, utmp[SBC_MAX_MONOPHONES];
   size_t nread;
   sbcTree *tree = NULL;
   sbcTreeNode *treenode; 
   short i;
   short* pShortObj;
   long* pLongObj;
   int clearCount;

   /* debug purpose , void SBC */
   /*   return eTIesrEngineHLRFail; */

   /* Initialize the sbcState structure */
#ifdef _DYN_ALLOC_SBC
   if( sa->pHMM2phone )
     {
       free( sa->pHMM2phone );
       sa->pHMM2phone = NULL;
     }
#endif


   /* Initialize all SBC parameters to null state */
   sbc_open( sa ); 

   /* If user input NULL for tree file, no SBC will be used */
   if( fname == NULL )
      return eTIesrEngineHLRSuccess;

   /* check for valid hlr input file */
   if( ( fp = fopen(fname, "rb") ) == NULL )
      return eTIesrEngineHLRFail;


   sa->hlr_TRANSFORM_SIZE = cep_dim; 
   /* Load the number of terminal nodes in the tree. */
   nread = fread( &(sa->n_terminals), sizeof(ushort), 1, fp );
   if( nread != 1 )
      goto InitFail;

   /* read the structure of the 2*n_terminals-1 node tree */
   /* sizeof(short) holds the size */
   /* the next two arraies are in ROM: */
   if (!load_monophone_only) 
   {
      sa->hlrTreeBase = (short*) mem_alloc( sa->base_mem, &sa->mem_count, 
					    sizeof(sbcTree) >> SHORTSHIFT, SBC_SIZE, LONGALIGN, 
					    "tree" );

      if( sa->hlrTreeBase == NULL )
	 goto InitMemoryFail;

      tree = (sbcTree *)sa->hlrTreeBase;
      tree->n_utt = 0; 
      tree->n_nodes = 2*sa->n_terminals - 1;
      tree->node = (psbcTreeNode*) mem_alloc( sa->base_mem, &sa->mem_count, 
					      (tree->n_nodes*sizeof(psbcTreeNode)) >> SHORTSHIFT,
					      SBC_SIZE, LONGALIGN, 
					      "node");
      
      if( tree->node == NULL )
	 goto InitMemoryFail;
   }

   for( i = 0; i <  tree->n_nodes; i++)
   {
      tree->node[i] = (psbcTreeNode)  mem_alloc( sa->base_mem, &sa->mem_count, 
						 sizeof(sbcTreeNode) >> SHORTSHIFT, 
						 SBC_SIZE, LONGALIGN, 
						 "node");

      if( tree->node[i] == NULL )
	 goto InitMemoryFail;

      nread = fread( &p_node, sizeof(ushort), 1, fp );
      if( nread != 1 )
	 goto InitFail;

      treenode = sbc_get_tree_node(i, sa); 
      treenode->vector_count = (ushort) 0;
      treenode->parent_node_and_transform_idx = 0; 

      /* Initialize diff 1 accumulator */
      treenode->p_acc_dif1 = (long*) mem_alloc( sa->base_mem, &sa->mem_count, 
						sa->hlr_TRANSFORM_SIZE << LONGSHIFT,
						SBC_SIZE, LONGALIGN, 
						"diff 1");

      if( treenode->p_acc_dif1 == NULL )
	 goto InitMemoryFail;

      /* Initialize the accumulator to zeros */
      pLongObj = treenode->p_acc_dif1;
      for( clearCount=0; clearCount < sa->hlr_TRANSFORM_SIZE; clearCount++ )
      {
	 *pLongObj++ = 0;
      }


      /* Initialize diff2 accumulator */
      treenode->p_acc_dif2 = (long*) mem_alloc( sa->base_mem, &sa->mem_count, 
						sa->hlr_TRANSFORM_SIZE << LONGSHIFT,
						SBC_SIZE, LONGALIGN, 
						"diff 2");

      if( treenode->p_acc_dif2 == NULL )
	 goto InitMemoryFail;

      /* Initialize the accumulator to zeros */
      pLongObj = treenode->p_acc_dif2;
      for( clearCount=0; clearCount < sa->hlr_TRANSFORM_SIZE; clearCount++ )
      {
	 *pLongObj++ = 0;
      }


      if (!load_monophone_only){
	
	 sbc_set_parent_node(i, p_node, sa); 
	 sbc_set_transform_idx(i, tree->n_nodes - 1, sa); 
      }
   }

   /* read the mapping from monophones to tree terminal nodes */
   nread = fread( &n_monophones, sizeof( ushort), 1, fp );
   if( nread != 1 )
      goto InitFail;

   sa->hlrTerminal = (ushort *) mem_alloc( sa->base_mem, &sa->mem_count, 
					   n_monophones, 
					   SBC_SIZE, SHORTALIGN, 
					   "monophone list" );
   if( sa->hlrTerminal == NULL )
      goto InitMemoryFail;

   /* note: (n_monophones != n_terminals) is possible */
   sa->n_monophones = n_monophones; 

   nread = fread( utmp, sizeof(ushort), n_monophones, fp );
   if( nread != n_monophones )
      goto InitFail;

   fclose( fp );

   /* make sure of the range: */
   for( i = 0; i < n_monophones; i++ ) {
     sa->hlrTerminal[i] = (uchar) utmp[i]; 
     if( (short) sa->hlrTerminal[ i ] >= sa->n_terminals ) {
       PRT(fprintf( stderr, "HLR: exceeded maximum (%d) terminals: %d\n", sa->n_terminals, i));
       goto InitFail;
     }
   }


   /* Initialize first order statistic */
   sa->pSegStat_dif1 = (short*)  mem_alloc( sa->base_mem, &sa->mem_count, 
					    cep_dim, 
					    SBC_SIZE, SHORTALIGN, 
					    "suff. 1st order stat. for current seg" );
					      
   if( sa->pSegStat_dif1 == NULL )
      goto InitMemoryFail;

   /* Initialize the statistic to zeros */
   pShortObj = sa->pSegStat_dif1;
   for( clearCount=0; clearCount < cep_dim; clearCount++ )
   {
      *pShortObj++ = 0;
   }


   /* Initialize second order statistic */
   sa->pSegStat_dif2 = (short*) mem_alloc( sa->base_mem, &sa->mem_count, 
					   cep_dim, 
					   SBC_SIZE, SHORTALIGN, 
					   "suff. 2st order stat. for current seg" );

   if( sa->pSegStat_dif2 == NULL )
      goto InitMemoryFail;

   /* Initialize the statistic to zeros */
   pShortObj = sa->pSegStat_dif2;
   for( clearCount=0; clearCount < cep_dim; clearCount++ )
   {
      *pShortObj++ = 0;
   }


   /* initialize terminal-to-transformation index */
   sa->hlrTransformIndex = (ushort *) mem_alloc( sa->base_mem, &sa->mem_count, 
						 tree->n_nodes, 
						 SBC_SIZE, SHORTALIGN, 
						 "transform index" );
						    
   if( sa->hlrTransformIndex == NULL )
      goto InitMemoryFail;

   /* All terminals initially use top tree node transform */
   for (i=tree->n_nodes-1; i >= 0; i--)
      sa->hlrTransformIndex[i] = tree->n_nodes - 1; 


   /* Initialize the transformations for all tree nodes */
   sa->hlrTransforms = (short*) mem_alloc( sa->base_mem, &sa->mem_count, 
					   tree->n_nodes * sa->hlr_TRANSFORM_SIZE, 
					   SBC_SIZE, SHORTALIGN, 
					   "bias" );

   if( sa->hlrTransforms == NULL )
      goto InitMemoryFail;

   /* Initialize the transformations to zeros */
   pShortObj = sa->hlrTransforms;
   for( clearCount=0; clearCount < tree->n_nodes * sa->hlr_TRANSFORM_SIZE; clearCount++ )
      *pShortObj++ = 0;


   /* Enhanced features memory does not need to be initialized, it will be set by SBC. */
   sa->pEnhanced = (short*) mem_alloc( sa->base_mem, &sa->mem_count, 
				       tree->n_nodes * sa->hlr_TRANSFORM_SIZE, 
				       SBC_SIZE, SHORTALIGN, 
				       "enhanced feature" );
					  
   if (sa->pEnhanced == NULL) 
      goto InitMemoryFail;

#if defined(USE_CONF_DRV_ADP) && defined(USE_CONFIDENCE)
   sa->bk_node_cnt = (short*) mem_alloc(sa->base_mem, &sa->mem_count,
					tree->n_nodes, SBC_SIZE, SHORTALIGN, 
					"backup node cnt");
   if (sa->bk_node_cnt == NULL)
     goto InitMemoryFail; 
#endif

   /* Load the hmm to phone mapping. This is available from HMM binary model directory. */
   if(sbc_load_hmm2phone(fnDir, n_hmms, sa, bTriphone) == eTIesrEngineHLRSuccess )
     return eTIesrEngineHLRSuccess;

  InitFail:
   /* Clear all sbc structure parameters */
   sbc_open( sa );
   return eTIesrEngineHLRLoadTree;

  InitMemoryFail:
   sbc_open( sa );
   return eTIesrEngineHLRMemoryBase;
}


/*--------------------------------*/
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_re_enhance( FeaHLRAccType * pSBC,
					 short *obs,
					 short *obs_enh,
					 ushort physical_hmm, 
					 short nbr_dim, 
					 short* p_sbc_bias)
{
  short ix;
  short * transform; 

  if (pSBC == NULL || pSBC->hlrTreeBase == NULL ) {
    for (ix=nbr_dim-1;ix>=0;ix--) obs_enh[ix] = obs[ix]; 
    return eTIesrEngineHLRFail;
  }

  ix = sbc_phone2transidx(physical_hmm, pSBC); 
  transform = sbc_get_transform(ix, pSBC);
 
  for (ix=nbr_dim-1;ix>=0; ix--){
    obs_enh[ix] = obs[ix] - transform[ix]; 
    if (p_sbc_bias != NULL) p_sbc_bias[ix] = transform[ix];
  }
  return( eTIesrEngineHLRSuccess );
}


/*--------------------------------*/
TIESRENGINECOREAPI_API short* sbc_enhance(
		  FeaHLRAccType * pSBC, 
		  short *obs,
		  ushort physical_hmm, 
		  short nbr_dim, 
		  short* p_sbc_bias)
{
  short ix;
  short * transform, *obs_enh; 

  if (pSBC == NULL || pSBC->hlrTreeBase == NULL ) return obs; 

  ix = sbc_phone2transidx(physical_hmm, pSBC); 
  obs_enh = sbc_get_enhanced(pSBC, ix);

  if (p_sbc_bias){
    transform = sbc_get_transform(ix, pSBC);
    for (ix=nbr_dim-1;ix>=0; ix--){
      p_sbc_bias[ix] = transform[ix];
    }
  }
  return( obs_enh );
}


/*--------------------------------*/
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_enhance_all_trans( FeaHLRAccType * pSBC,
						short *obs,
						short nbr_dim)
{
  short ix, jx;
  ushort itrans;
  short * transform, *penh; 
  short iprev_trans = -1; 

  if (obs == NULL) return eTIesrEngineHLRFail;
  if (pSBC == NULL || pSBC->hlrTreeBase == NULL ) return eTIesrEngineHLRFail;
  for (ix=pSBC->n_terminals-1;ix>=0;ix--){
    itrans =  pSBC->hlrTransformIndex[ix] ; 
    if (iprev_trans == itrans) continue;

    transform = sbc_get_transform(itrans, pSBC);
    penh = sbc_get_enhanced(pSBC, itrans); 
    if (penh != NULL && transform != NULL)
      for (jx=nbr_dim-1;jx>=0; jx--) penh[jx] = obs[jx] - transform[jx]; 
    else
      return eTIesrEngineHLRFail; 

    iprev_trans = itrans; 
  }  
  return( eTIesrEngineHLRSuccess );
}

/*--------------------------------*/
/* accumulate statistics 
   @param nbr_dim dimension of cepstral vector
   @param T number of frames of the current segment
   @param p_mean pointer to mean vector, Q11
   @param p_invvar pointer to inverse variance vector, Q9
   @param mfcc MFCC cepstral, Q11
   @param gamma posterior probabilty, Q15
   @param p_bias pointer to the bias of SBC method for the current model, Q11
   @param sa pointer to FeaHLRAccType. 

 */
TIESRENGINECOREAPI_API void sbc_acc_cep_bias(ushort nbr_dim, ushort T, short * p_mean,
		      short *p_invvar, const short * mfcc, 
		      ushort gamma, short * p_bias, FeaHLRAccType* sa)
{
  short i ;
  short ftmp;
  long f1, f2; 
  if (sa == NULL || sa->hlrTreeBase == NULL ) return;

  for (i = nbr_dim - 1;i>= 0; i--){
    ftmp = mfcc[i] - p_mean[i] + p_bias[i];
    ftmp /= T; 
    f1 = (long) ftmp * p_invvar[i]; /* Q11 * Q9 */
    f1 = LONG_RIGHT_SHIFT_N(f1, 9); /* Q20 -> Q11 */
    f1 = q15_x(gamma, f1); /* Q11*/
    sa->pSegStat_dif1[i] += f1; 
    
    f2 = q15_x(gamma, p_invvar[i]); 
    f2 /= T; /* Q9*/
    sa->pSegStat_dif2[i] += f2; 
  }
}

/*--------------------------------*/
TIESRENGINECOREAPI_API Boolean sbc_acc_SBC(ushort hmm_code, ushort cep_dim,
		 ushort seglength, FeaHLRAccType * sa,
		 ushort fgtrho )
{
  ushort ix;
  short jx;
  long *dif1, *dif2, ltmp; 
  sbcTree *tree;
  ushort iterminalidx ; 

  if (sa == NULL || sa->hlrTreeBase == NULL ) return FALSE; 

  tree = (sbcTree*) sa->hlrTreeBase; 
  iterminalidx = 
    sbc_determine_reliable_node(hmm_code, sa, seglength, TRUE); 
  
  for (ix=iterminalidx;ix<tree->n_nodes;){
    dif1 = sbc_get_transform_stat_dif1(ix, sa); 
    dif2 = sbc_get_transform_stat_dif2(ix, sa); 
    for (jx = cep_dim - 1; jx >= 0; jx --){

      ltmp = (long) sa->pSegStat_dif1[jx];
      dif1[jx] += ltmp; 
      ltmp = (long) sa->pSegStat_dif2[jx];
      dif2[jx] += ltmp; 
      if (dif2[jx] < 0) {
	PRT_ERR(fprintf(stdout, "Error : HMM[%d] dif2[%d] = %d\n", hmm_code, jx));
	return FALSE  ; 
      }

    }
    ix = sbc_parent_is(tree->node[ix]);
    if (ix >= tree->n_nodes) break; 
  }

  return TRUE; 
}

static ushort sbc_update_SBC_seg(ushort hmm_code, ushort cep_dim, 
				 FeaHLRAccType* sa, unsigned short afgt)
{
  ushort inode, iprec = 2 , ihier;
  short jx;
  sbcTree* tree = (sbcTree*) sa->hlrTreeBase;
  short* p_trans;
  long *dif1, *dif2, ltmp, ltmp3; 
  short div;
     
#ifdef SBC_PRIOR
     long ltmp2;
#endif

  short iTerminalIdx = sbc_phone2terminalidx(hmm_code, sa); 

  for (ihier=iTerminalIdx;ihier < tree->n_nodes; ){

    if (sbc_is_marked(sa->hlrTransformIndex[ihier]))
      return iTerminalIdx;  

    inode = sa->hlrTransformIndex[ihier];
    p_trans = sbc_get_transform(inode, sa); 
    dif1 = sbc_get_transform_stat_dif1(inode, sa); 
    dif2 = sbc_get_transform_stat_dif2(inode, sa); 

    /* by default, dif1 is Q11 and dif2 is Q9 */
    for (jx= cep_dim -1 ;jx>=0;jx--){
      ltmp = dif2[jx] << iprec ; 
      ltmp3 = dif1[jx] ; 

#ifdef SBC_PRIOR
      ltmp2 = sbc_prior_invvar(jx, cep_dim);
      ltmp2 = LONG_RIGHT_SHIFT_N(ltmp2, 2); /* 0.25 * the variance */
      ltmp3 += LONG_RIGHT_SHIFT_N(ltmp2, 11) * p_trans[jx];
      div = (short) div32_32_Q(ltmp3, ltmp + ltmp2, MEAN_PREC);
#else
      div = (short) div32_32_Q(ltmp3, ltmp, MEAN_PREC);
#endif
      p_trans[jx] = div; 

      dif1[jx] = q15_x(afgt, dif1[jx]);
      dif2[jx] = q15_x(afgt, dif2[jx]);

    }

    sbc_mark(&(sa->hlrTransformIndex[ihier]), sa);

    ihier = sbc_parent_is(tree->node[ihier]);
  }

  return iTerminalIdx; 
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void sbc_update_SBC(gmhmm_type* gv)
{
  short s;

  if (gv->sbcState == NULL || gv->sbcState->hlrTreeBase == NULL ) return; 
  sbc_update_SBC_phone2trans(gv);

  for (s = gv->nbr_seg -1; s >=0 ; s--){ /* for each segment */
    sbc_update_SBC_seg(gv->hmm_code[s], gv->nbr_dim, 
		       gv->sbcState, gv->sbc_fgt );
  }

  sbc_unmark( gv->sbcState );

#ifdef _SBC_DEBUG
  sbc_dbg_dump(gv->sbcState);
#endif

  /*
    used to generate SBC for learning prior statistics 
    #ifdef SBC_PRIOR
    #ifdef USE_SNR_SS
    printf("NoiseLvl = %d ", noiselvl);
    #endif
    sbc_dump_transform( gv->sbcState);
    #endif
  */
}

/*--------------------------------
  compute pdf for one state pdf.
  --------------------------------*/
TIESRENGINECOREAPI_API void sbc_observation_likelihood(ushort seg_stt, short T, ushort hmm_code,
				HmmType *hmm, 
				short pdf[], short n_mfcc, gmhmm_type *gv )
{
  ushort t, i, crt_vec;        
  short x, pdf_i, nbr_dim = n_mfcc * 2;
  short mfcc_feature[ MAX_DIM ];
  short obs_enh[MAX_DIM]; 
  
  FOR_EMS_STATES(i,hmm,gv->base_tran) { 
    pdf_i = GET_BJ_IDX(hmm,i);
    for (t=0, crt_vec = seg_stt * n_mfcc; t<T; t++, crt_vec += n_mfcc) {
#ifdef BIT8FEAT
      vector_unpacking(gv->mem_feature + crt_vec, mfcc_feature, gv->scale_feat, n_mfcc); 
#else
      copy_feature(mfcc_feature, crt_vec, n_mfcc);
#endif
      sbc_re_enhance( gv->sbcState, mfcc_feature, obs_enh, hmm_code, nbr_dim, NULL);
      pdf[t] = hlr_gauss_obs_score_f(obs_enh, (int)pdf_i, FULL, &x, nbr_dim, gv);
    }
    pdf += T;
  }
}


/*--------------------------------
  sbc_set_default_params

  Initialize SBC parameters with default values.
  --------------------------------*/
TIESRENGINECOREAPI_API void sbc_set_default_params( gmhmm_type *gv )
{
   gv->sbc_fgt = SBC_FGT_DEFAULT;
}


/*--------------------------------
  sbc_free

  Free any allocated memory during sbc operation.
  --------------------------------*/
TIESRENGINECOREAPI_API void sbc_free( FeaHLRAccType *aSBC )
{

   if( aSBC == NULL )
      return;

#ifdef _DYN_ALLOC_SBC
   if( aSBC->pHMM2phone )
   {
      free( aSBC->pHMM2phone );
      aSBC->pHMM2phone = NULL;

      sbc_open( aSBC );
   }
#endif

}
