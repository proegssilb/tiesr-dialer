/*=======================================================================
 
 *
 * siinit.cpp
 *
 * TIesr initialization.
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

 This source provides for opening and closing the TIesr SI ASR, and
 functions for initializing parameters and memory allocation.

======================================================================*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif


#include "tiesr_config.h"
#include "tiesrcommonmacros.h"

#include "siinit.h"

#include "gmhmm_type.h"
#include "search_user.h"
#include "uttdet_user.h"
#include "load_user.h"
#include "volume_user.h"
#include "gmhmm_si_api.h"
#include "jac-estm_user.h"
#include "noise_sub_user.h"

//#ifdef USE_SBC
/* Includ header if using stochastic bias compensation in order to
 access functions to set defaults */
#include "sbc_user.h"
//#endif

#ifdef USE_NBEST
#include "nbest_user.h"
#endif

#if defined(USE_RAPID_SEARCH) || defined(RAPID_JAC)
/* Include header if using clustered Gaussian processing for rapid
 likelihood calculations in order to access functions to set defaults. */
#include "rapidsearch_user.h"
#endif

#ifdef USE_CONFIDENCE
/* Include header if calculating confidence measures in order to access
 functions to set defaults. */
#include "confidence_user.h"
#include "confidence.h"
/* ----------------------- trained UV model parameters -----------------------
   CM_UV_ACC and CM_UV_REJ are mean, inv, and gconst of the acceptance/rejection models
   for the feature of LLR between the best path and the accumulated maximum likelihood 
   score. LLR is on speech segments. 
   [mean, inv, gconst] with Q11, Q9, and Q6; 
*/

#ifdef C55x
const short CM_UV_ACC[3] = {-4685, 128, 206}; /* trained from OOV, IV in 8-types of Aurora noise,
						 TIMIT noise in 10dB, and WAVES noise. */
//const short CM_UV_ACC[3] = {-4189, 166, 190}; // trained from OOV in WAVES noise
const short CM_UV_REJ[3] = {-9793, 5, 412}; /* trained from OOV, IV in 8-types of Aurora noise,
					       TIMIT noise in 10dB, and WAVES noise. */
//const short CM_UV_REJ[3] = {-9120, 86, 231};   // trained from OOV in WAVES noise

/* ---------------------- logistic regression ------------------------------
   train UV with logistic regression
   1 / (1 + exp(-W X)
   where W = [w0, w1, w2], and X = [1, cm[0], cm[1]]
*/
const short CM_LOGIST_PARA[CM_NBR_BETA] = {377, 62, -18};
/* trained from OOV, IV in 8-types of Aurora noise, TIMIT noise in 10dB, and WAVES noise. */
//const short CM_LOGIST_PARA[CM_NBR_BETA] = {296, 64, -97}; // triained from OOV in WAVES noise

const short CM_NBEST_LOW_CONF = -50;
/* the confidence score for those utterances have too many N-bests */
#endif //C55x

#endif

/*
** shared by on-line and offline SI (JAC). (jac-main.c, main.c)
*/

// static short mem_mfcc_C [ MAX_FRM * MAX_NBR_MFCC ];   /* mfcc */

/* 
** search space split:
** used name 200 statistics: BEAM 4.3900000e-01, SYMB  1.3500000e-01, STATE  4.9300000e-01
** normalized, ./ by [3 4 3] and scaled to Q16: 
*/

// ratio = [.46 .13 .43]; 

#ifdef _MONOPHONE_NET
void output_network(FILE*, short *base_net);
#endif

/* 
#define BEAM_Z   32766 // MAX. (idx_flag limits to 2^15)  ( for SI name dilaing) // 10000 
#define SYMB_Z   6000  // 4000 
#define STATE_Z  30000  // 20000
#define TIME_Z   2000 
*/

const short K_mean[ MAX_DIM_MFCC16 ] = /* {11668, 506, 957, -1425, -472, -668, -2806, 1911};   */
  { 14149,  -4777,  -746,  287,  -2014,  -749,  -773,  -171 , 0,0, 
    0,0,0,0,0,0};

/*
** If we assume: 1. 100MIPS --> 2mil cyc / frame.   2. 20K cyc / vector for JAC
** then we should be able to do 100 vector/frame.
*/


/*--------------------------------*/
unsigned short GetMaxFrames(gmhmm_type *gvv )
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  return gv->max_frame_nbr;
}


/*---------------------------------------------------------------------------
 Init_Search

 Initialize search parameters in the ASR structure.

 Arguments:
 gv: Pointer to the ASR structure
 p_K_mean: pointer to estimate of initial CMN mean vector, used only for CMN


------------------------------ */
static void Init_Search(gmhmm_type * gv, const short * p_K_mean)
{
  ushort i; 
  Boolean bUseNBest = TRUE;

  /* Use JAC by default */
  short compType = JAC;

  #ifdef USE_SVA
  compType |= SVA;
  #endif

  #ifdef USE_SBC
  compType |= SBC;
  #endif

  gv->comp_type = compType;

  gv->scale_feat = gv->scale_mu;

  /*  Note: feature vector assumed static + delta mfcc */
  gv->nbr_dim = gv->n_mfcc * 2;

  //  const_init();
  //  dim_init(gv->n_mfcc, &(gv->mu_scale_p2));

  /* Initialize feature processing parameters */
  dim_p2_init(gv->n_mfcc, gv);


  /* Initialize initial CMN vector guess */
  for (i = 0; i < gv->n_mfcc; i++) {
    gv->mfcc_mean[ i ] = p_K_mean[ i ]; 
  }

  /* Initialize last sample of prior window */
  gv->last_sig = 0;

  /* Set some default parameters */
  gv->low_vol_limit  = LOW_VOL_LIMIT;
  gv->high_vol_limit = HIGH_VOL_LIMIT;
  
  gv->prune = PRUNE_COEFF * (1<<6);    
  gv->tranwgt = (WIP)*(1<<6); /* word insertion balance */

#ifdef REALTIMEGUARD
  // frames that trigger more pruning as realtime guard, 50 is one second.
  gv->realtime_th = REALTIME_TH;  
  gv->realt_guard = REALT_GUARD;
#endif

  gv->bUseNBest = bUseNBest; 
#ifdef USE_NBEST
  gv->ptr_nbest_tmp = NULL;
#endif

#ifdef _MONOPHONE_NET
  gv->pAux = NULL;
  gv->ptr_nbest_tmp = NULL;
#endif

}

static void SetDefault(gmhmm_type * gv)
{
  /* Initialize JAC default parameters */
  JAC_set_default_params( gv );

#ifdef USE_SBC
   sbc_set_default_params( gv );
#endif

  /* Initialize SAD default thresholds */
  set_default_uttdet(gv);

#if defined(USE_CONFIDENCE) && defined(USE_CONF_DRV_ADP)
  cm_set_param(CM_ADP_THRE, CM_NBR_NBEST_LOW_CONF, gv); 
#endif

#ifdef  USE_ONLINE_REF
  rj_set_ORM( GBG_num_ids, 
	      GBG_HNOISE,  GBG_VAD_VERIFY_TH_LNOISE,  GBG_VAD_VERIFY_TH_HNOISE, 
	      GBG_END_SPEECH_CNT_TH, GBG_END_SPEECH_TH_PERC, 
	      GBG_END_SPEECH_START, GBG_NUM_AFTER_PEAK_TO_TRIGGER_END_SPEECH,
	      gv);
#endif
  
}

/*-------------------------------------------------------------------------
 AllocateTIesrMemory

 Allocate memory in memory pool above the gmhmm_type ASR structure to the
 variables needed for search and mfcc processing.

 Arguments:
 memoryPool: short pointer to memory pool
 max_frm: Maximum frames of speech to process, excluding leading non-speech
 ptr_model_file_path: Name of directory holding model and grammar files

 ----------------------------------*/
static TIesrEngineStatusType AllocateTIesrMemory( short* memoryPool, 
						  unsigned short max_frm,
						  char * ptr_model_file_path )
{
  gmhmm_type *gv;
  Uint32 BEAM_Z, SYMB_Z, STATE_Z;
  Uint32 total_BSS, a_size;
  //Uint32 total_words; /* in short */
  Uint32 mfcc_memory_size; /* storage of mfcc of the utterance for JAC */
  short *search_mem;
  Uint32 curt_mem;     /* size of used memory, in short */
  Uint32 stt_mem = 0;  /* index of first available memory space */
    /* basic gmhmm_type space */
  gv = (gmhmm_type *)( memoryPool + stt_mem );
  curt_mem = (sizeof(gmhmm_type) + 1) >> 1; /* in short */
  stt_mem += curt_mem;
  display_size("gmhmm_type memory (scratch)",curt_mem, stt_mem,  gv->total_words);

  /* Save model directory */
  if( ptr_model_file_path )
  { 
    /*  gv->chpr_Model_Dir = (char*)(memoryPool + stt_mem); */
    /*  curt_mem =  (strlen( ptr_model_file_path ) + 1) >> 1; */
    /*  curt_mem++; */
    /*  /\* Ensure next allocation is on int boundary *\/ */
    /*  if( curt_mem & 0x1 ) */
	/* curt_mem++; */

    /*  stt_mem += curt_mem; */
    /*  if( stt_mem >= gv->total_words ) */
    /*     return eTIesrEngineSearchMemorySize; */
      gv->chpr_Model_Dir = (char*) malloc(sizeof(char)*strlen( ptr_model_file_path )+1);
      strcpy( gv->chpr_Model_Dir, ptr_model_file_path );
  }
  else
  {
      gv->chpr_Model_Dir = (char*) NULL;
  }

  /* phone-level pruning */
  gv->pPhoneLevelPrune = NULL; 
#ifdef MODEL_LEVEL_PRUNE

  gv->pPhoneLevelPrune = (MODEL_PRUNE_type*) malloc(sizeof(MODEL_PRUNE_type));
  /* /\* size of cluster-dependent JAC structure and data *\/ */
  /* curt_mem = ( sizeof( MODEL_PRUNE_type ) + 1 ) >> 1 ;  */
  mlp_open( gv ) ; 
  /* stt_mem += curt_mem; */
  /* display_size("Phone-level pruning data structure memory", curt_mem, stt_mem,  gv->total_words); */
  if ( (gv->pPhoneLevelPrune->p_s_active==NULL) || !(gv->pPhoneLevelPrune->p_s_cur_mono))
      return eTIesrEngineSearchMemorySize;

#endif

  /* SBC compensation structure, if used */
  gv->sbcState =(FeaHLRAccType *) NULL;
#ifdef USE_SBC

  gv->sbcState = (FeaHLRAccType *)malloc(sizeof(FeaHLRAccType));

  /* /\* size of SBC state structure *\/ */
  /* curt_mem = ( sizeof( FeaHLRAccType ) + 1 ) >>1; */
  /* stt_mem += curt_mem; */
  /* display_size("SBC structure memory", curt_mem, stt_mem,  gv->total_words); */

  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSearchMemorySize; */

  /* Construction initialization */
  sbc_open( gv->sbcState );

#endif

  gv->pConf = NULL;

#ifdef USE_CONFIDENCE
  gv->pConf =(ConfType*) malloc(sizeof(ConfType)); 
  //((ConfType*)gv->pConf)->base_mem=(short*)(memoryPool + stt_mem);
  //
  /* size for the confidence measures */
  /* curt_mem = ( sizeof ( ConfType )+sizeof(short)*1000 + 1) >> 1 ;  */
  /* stt_mem += curt_mem; */
  /* display_size("Confidence measure data structure memory", curt_mem, stt_mem,  gv->total_words); */
  /* if( stt_mem >= gv->total_words ) */
  /*   return eTIesrEngineSearchMemorySize; */
  if( cm_open( (ConfType*) gv->pConf, gv ) == eTIesrEngineCMMemorySize )
     return eTIesrEngineCMMemorySize;

#endif

  /* noise supression structure, if used */
  gv->pNss = NULL;
#ifdef USE_SNR_SS

  gv->pNss = (NssType*) malloc(sizeof(NssType));

  /* /\* size of noise spectral subtraction state structure *\/ */
  /* curt_mem = ( sizeof( NssType ) + 1 ) >> 1; */
  /* stt_mem += curt_mem; */
  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSearchMemorySize; */
  /* display_size("Noise supression structure memory", curt_mem, stt_mem,  gv->total_words); */

  /* Construction initialization */
  ss_open( (NssType*) gv->pNss , gv); 
  if ( (gv->pNss->s_noise_floor==NULL) || (gv->pNss->snr==NULL) || (gv->pNss->ps_long_term_noise==NULL))
      return eTIesrEngineSearchMemorySize;

#endif

  /* cluster-dependent JAC structure, if used */
  gv->offline_vq = (short*) NULL; 

#ifdef OFFLINE_CLS
  gv->offline_vq =  memoryPool + stt_mem ;
  /* size of cluster-dependent JAC structure and data */
  curt_mem = ( sizeof( OFFLINE_VQ_HMM_type ) + 10 ) >> 1 ;
  stt_mem += curt_mem;

  if( stt_mem >= gv->total_words )
     return eTIesrEngineSearchMemorySize;

  display_size("Cluster-dependent JAC data structure memory", curt_mem, stt_mem,  gv->total_words);
  /* Initialize vq cluster info and variables for rapid JAC calculations */
  if (rj_open( u_rj_perc_core,  u_rj_perc_inte , gv) == eTIesrEngineVQHMMMemorySize)
    return eTIesrEngineVQHMMMemorySize;

#endif

  /* Allocate for sym2pos map arrays */
  /* gv->sym2pos_map.syms = (unsigned short *)(memoryPool + stt_mem); */
  /* curt_mem = gv->trans->n_sym * gv->trans->n_set + 1; */
  /* stt_mem += curt_mem; */
  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSym2PosMapSize; */

  /* gv->sym2pos_map.sym2pos = memoryPool + stt_mem; */
  /* stt_mem += curt_mem; */
  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSym2PosMapSize; */

  /* gv->sym2pos_map.max_nbr_syms = curt_mem; */

  gv->sym2pos_map.syms = (unsigned short *)malloc(gv->trans->n_sym * gv->trans->n_set*sizeof(unsigned short));
  gv->sym2pos_map.sym2pos = (short *)malloc(sizeof(short));
  if(  (gv->sym2pos_map.syms==NULL) || (gv->sym2pos_map.sym2pos==NULL) )
     return eTIesrEngineSym2PosMapSize;

  gv->sym2pos_map.max_nbr_syms = gv->trans->n_sym * gv->trans->n_set+1;

  /* Allocate space to hold HMM codes and start/stop frames mapped to each
   HMM in the best recognition result. Assume no more than MAX_NBR_SEGS
   HMMs are in the best recognition result path. If there are more, then
   JAC will not use them for adaptation. */
  /* variable size, so SI, SA, and SD can all use: */
  /* gv->hmm_code = (ushort *)( memoryPool + stt_mem ); */
  /* curt_mem = MAX_NBR_SEGS * 3; */
  /* stt_mem += curt_mem; */

  /* display_size("segmental information",curt_mem, stt_mem, gv->total_words); */
  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSearchMemorySize; */

  /* gv->stt =  gv->hmm_code +  MAX_NBR_SEGS; */
  /* gv->stp =  gv->hmm_code +  MAX_NBR_SEGS * 2; */
  gv->hmm_code = (ushort *)malloc(MAX_NBR_SEGS * 3*sizeof(ushort));
  gv->stt =  gv->hmm_code +  MAX_NBR_SEGS;
  gv->stp =  gv->hmm_code +  MAX_NBR_SEGS * 2;
  if  (gv->hmm_code==NULL)
     return eTIesrEngineSearchMemorySize;


  /* answer list size */
  /* gv->answer = (ushort *)( memoryPool + stt_mem ); */
  /* curt_mem = MAX_WORD_UTT; */
  /* stt_mem += curt_mem; */
  /* display_size("answer word list memory",curt_mem, stt_mem, gv->total_words); */

  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSearchMemorySize; */
  gv->answer = (ushort *)malloc( MAX_WORD_UTT*sizeof(ushort));
  if (gv->answer ==NULL)
      return eTIesrEngineSearchMemorySize;
  gv->max_frame_nbr = max_frm; /* modifier */


/* Allocate memory to hold incoming feature vectors.
   TODO: change code to accommodate static, delta, acc ... */
#ifdef BIT8FEAT
  mfcc_memory_size = gv->max_frame_nbr * gv->n_mfcc;  
#else
  mfcc_memory_size = gv->max_frame_nbr * gv->nbr_dim;
#endif
  
  /* feature vector memory for JAC: */
  /* gv->mem_feature = (ushort *)( memoryPool + stt_mem ); */
  /* curt_mem = mfcc_memory_size; */
  /* stt_mem += curt_mem; */
  /* display_size("MFCC memory",curt_mem, stt_mem, gv->total_words); */
  /* if( stt_mem >= gv->total_words ) */
  /*    return eTIesrEngineSearchMemorySize; */
  gv->mem_feature = (ushort *)malloc(mfcc_memory_size*sizeof(ushort));
  if( gv->mem_feature == NULL )
     return eTIesrEngineSearchMemorySize;

  /* /\* Remainder of memory pool used for search. *\/ */
  /* total_words = gv->total_words - stt_mem;   */
  /* if (total_words <= 0) return eTIesrEngineSearchMemorySize; */

  /* Allocate search space memory.
   TODO: check for not enough memory available. */
  /* search_mem = (short *) memoryPool + stt_mem; */
  /* curt_mem = total_words; */
  /* stt_mem += curt_mem; */
  /* display_size("search memory",curt_mem, stt_mem, gv->total_words); */

  //  total_BSS = (TIME_SZ *TIME_Z+ BEAM_Z*BEAM_SZ+STATE_SZ * STATE_Z)*SYMB_SZ;
  //search_mem = (short *) malloc(total_BSS*sizeof(short));
                                
  /* Time cells */
  /* if( total_words < TIME_SZ *TIME_Z ) */
  /*    return eTIesrEngineSearchMemorySize; */
 
  total_BSS=TOTAL_BSS;
  
#ifdef WORDBACKTRACE
  /* Beam cells */
  a_size = BEAM_CELL_LIMIT * 2 ; //+ (BEAM_CELL_LIMIT>>1) ; // and beam may go beyond limit, so add 50% more 
  //a_size =(BEAM_R * total_BSS) >> 16; /* must < 32766 */
  //if (a_size >= 32766) return eTIesrEngineSearchMemoryLimit;

  BEAM_Z = a_size;
  if( total_BSS <= BEAM_Z*BEAM_SZ ) 
      return eTIesrEngineSearchMemorySize; 

  /* State cells */
  STATE_Z  = (total_BSS - BEAM_Z*BEAM_SZ)*17/20/STATE_SZ;     //90% of the rest.. 1:10 ratio to SYM CeLL   (STATE_R * total_BSS) >> 16;
  //  STATE_Z  = (total_BSS-a_size)*9/10/STATE_SZ;     //90% of the rest.. 1:10 ratio to SYM CeLL   (STATE_R * total_BSS) >> 16;
  //STATE_Z  = (STATE_R * total_BSS) >> 16;

  /* if( STATE_Z == 0 ) */
  /*     return eTIesrEngineSearchMemorySize; */

  /* Symbol cells */
  SYMB_Z = (total_BSS - BEAM_SZ * BEAM_Z - STATE_SZ * STATE_Z) /SYMB_SZ ;//>> 2; /* e.g. div by SYMB_SZ */
//printf("BEAM %d, STATE %d, SYM %d ===\n", BEAM_SZ * BEAM_Z, STATE_SZ * STATE_Z, SYMB_Z*3 );

  if( SYMB_Z == 0 ) 
      return eTIesrEngineSearchMemorySize; 
#else
  
  a_size = (BEAM_R * total_BSS) >> 16; /* must < 32766 */
  if (a_size >= 32766) return eTIesrEngineSearchMemoryLimit;

  BEAM_Z = a_size;
  STATE_Z  = (STATE_R * total_BSS) >> 16;
  SYMB_Z = (total_BSS - BEAM_SZ * BEAM_Z - STATE_SZ * STATE_Z) /SYMB_SZ; /* e.g. div by SYMB_SZ */

#endif


/*  
  a_size = (BEAM_R * total_BSS) >> 16; // must < 32766 
  if (a_size >= 32766) return eTIesrEngineSearchMemoryLimit;

  BEAM_Z = a_size;
  STATE_Z  = (STATE_R * total_BSS) >> 16;
  SYMB_Z = (total_BSS - BEAM_SZ * BEAM_Z - STATE_SZ * STATE_Z) >> 2; // e.g. div by SYMB_SZ 
*/


  //PRT_ERR(printf("Total search memory: %d (words)\n",SEARCH_SIZE(BEAM_Z, SYMB_Z, STATE_Z, TIME_Z)));
  
/*
  search_mem = (short *) malloc(SEARCH_SIZE(BEAM_Z, SYMB_Z, STATE_Z, TIME_Z));
*/
  search_mem = (short *) malloc(SEARCH_SIZE(BEAM_Z, SYMB_Z, STATE_Z, TIME_Z)*sizeof(short));
  if ( search_mem == NULL )
     return eTIesrEngineSearchMemorySize;
  set_search_space(search_mem, BEAM_Z, SYMB_Z, STATE_Z, TIME_Z, gv);
  return eTIesrEngineSuccess;
}


/*---------------------------------------------------------------------------
 OpenASR

 Initialize TIesr state structure. Load speech HMM and sentence network,
 initializes parameters, and allocates memory structures.

 Arguments:
 path: Directory containing HMM models and grammar network for recognition
 total_words: Number of shorts in memoryPool
 memoryPool: Pointer to memory, the beginning holds the TIesr state structure
 
-----------------------------*/
TIesrEngineStatusType OpenASR(char model_file_name[], int total_words, 
			      gmhmm_type* memoryPool )
{
   gmhmm_type *gv;
   TIesrEngineStatusType status;
   unsigned int gvSize;

#ifdef _MONOPHONE_NET
   FILE* fp; 
#endif

  /* Size of gmhmm_type in whole shorts */
  gvSize = (sizeof(gmhmm_type) + 1) >> 1;
  if( total_words < gvSize )
     return eTIesrEngineMemorySizeFail;

  gv = (gmhmm_type*)memoryPool;
  gv->vocabulary = (char**) NULL;



  /* Load HMM models and grammar network. Allocates space from heap. */
  status = load_models(model_file_name, gv, FALSE, (char*) NULL, (char*) NULL); 
  if (status != eTIesrEngineSuccess) return status;
 
  /* total memory available for gmhmm_type ASR structure,
   search and mfcc storage */
  gv->total_words = total_words; 

  /* Initialize search parameters and variables in ASR structure */
  Init_Search(gv, K_mean); 

#ifdef _MONOPHONE_NET
  fp = my_fopen("test.xvcg", "wt");
  output_network(fp, (short*)((gmhmm_type*)gv->pAux)->trans);
  my_fclose(fp);
  status = AllocateTIesrMemory( (short*) rs_get_decoder(gv), MAX_FRM_NBR, model_file_name);
  if (status != eTIesrEngineSuccess) return status;
#endif

  /* Allocate memory in the memory pool above the ASR structure to search and
   frame processing variables */
  status = AllocateTIesrMemory( (short*)memoryPool, MAX_FRM_NBR, model_file_name );

  if (status != eTIesrEngineSuccess) 
    return status; 

  /* Set default values of ASR parameters */
  SetDefault(gv);

  return status;
}

/*--------------------------------*/
/* GMHMM_SI_API */ 
void CloseASR(gmhmm_type *gvv)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
/*  void free_models(gmhmm_type *);*/
  free_models(gv);  

#ifdef USE_SBC
  sbc_free( gv->sbcState );
#endif

#ifdef USE_NBEST
  if (gv->bUseNBest) nb_close(gv); 
#endif

#if defined(OFFLINE_CLS)
  rj_close(gv); 
#endif

#ifdef USE_SNR_SS
  ss_close(gv);
#endif

#ifdef USE_CONFIDENCE
  cm_close(gv);
#endif

#ifdef MODEL_LEVEL_PRUNE
  mlp_close(gv);
#endif

}

#ifdef _MONOPHONE_NET

void print_next_list(FILE* fp, ushort s, ushort *l_next, ushort nbr_next, char color[], short revert)
{
  ushort p, from_node, to_node, n_next;

  n_next = GET_N_NEXT(nbr_next); 

  for (p=0; p<n_next; p++) {
    if (revert) {
      from_node = l_next[p];
      to_node = s; 
    }
    else {
      from_node = s;
      to_node = l_next[p];
    }
    fprintf(fp,"edge: {thickness: 1 sourcename: \"%d\" targetname: \"%d\" color: %s}\n", 
	   from_node, to_node, color);
  }
}

/*
 * show a network of grammar
 */
 
void output_network(FILE *fp, short *base_net)
{
  ushort s;
  TransType *trans = (TransType *) base_net;
  SymType *p;
  char str[ MAX_STR ];
  
  fprintf(fp,"graph: {\nsplines: yes\ndisplay_edge_labels: yes\n");
  fprintf(fp,"/* list of nodes: */\n");

  /* list of nodes: */
  for (s = 0; s < trans->n_sym; s++) {
    p = (SymType *)(base_net + trans->symlist[s]);
    sprintf(str,"NODE: %2d PDF: %2d", s, p->hmm_code);
    fprintf(fp,"node: { title: \"%d\" label: \"%s\" }\n", s, str);
  } 
  fprintf(fp,"node: { title: \"%d\" label: \"START NODE\" color: red }\n", s++);
  fprintf(fp,"node: { title: \"%d\" label: \"STOP NODE\"  color: red }\n", s++);

  fprintf(fp,"/* list of edges: */\n");
  for (s=0; s< trans->n_sym; s++) {
    p = (SymType *)(base_net + trans->symlist[s]);
    fprintf(fp,"/* hmm code = %d:  */\n", p->hmm_code);
    print_next_list(fp, s,p->next,p->n_next, "black", FALSE);
  }
  fprintf(fp,"/* list of entry-exit nodes: */\n");
  p = (SymType *)(base_net + trans->start);
  print_next_list(fp, s++,p->next,p->n_next, "green", FALSE);
  p = (SymType *)(base_net + trans->stop);
  print_next_list(fp, s++,p->next,p->n_next, "green", FALSE);
  fprintf(fp,"}\n");
}

#endif
