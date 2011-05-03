/*=======================================================================

 *
 * rapidsearch.cpp
 *
 * Rapid likelihood calculation using clustering.
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

 Rapid likelihood calculation by clustered Gaussian representation.

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"
#include "search_user.h"
#include "dist_user.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "sbc_user.h"
#include "load_user.h"
#include "pmc_f_user.h"
#ifdef USE_NBEST
#include "nbest_user.h"
#endif
#include "rapidsearch_user.h"
#include "confidence_user.h"

#include "confidence.h"

#include "jac-estm.h"
#include "pack_user.h"

//#define DEV_CONFIDENCE
//#define DEV_NONSTATIONARY
//#define DEV_PRIOR_SHOW
//#define TEST_PU
//#define TEST_CM_LOGISTIC
//#define DBG_CONF
/* ------------------------------------------------- 
   for Gaussian selection 
   ------------------------------------------------- */
const ushort u_rj_perc_core = 16384; /* 50% core clusters */ 
const ushort u_rj_perc_inte = 0; /* 0% intermedium clusters */ 

/* --------------------------------------------------
   parameters for on-line reference modeling 
   ------------------------------------------------- */
#define GBG_NUM_IDS 10  /* the maximum number of begining frames for a garbage model */
const ushort GBG_num_ids = 9; /* number of frames to construct a garbage model */

/* --------------------------------------------------
   parameters for GMM-based VAD 
   ------------------------------------------------*/
const ushort GBG_HNOISE = 12000; /* Q9, threshold of noise level in log2, larger than the
				  number is considered as high noise level. */
const ushort GBG_VAD_VERIFY_TH_HNOISE = 80; 
/* Threshold of the log-likelihood ratio for high noise level */
const ushort GBG_VAD_VERIFY_TH_LNOISE = 150;
/* Threshold of the log-likelihood ratio for low noise level */

const ushort GBG_NUM_SIL = 20; 
/* minimum number of frames to declare long pause */
const ushort u_low_is_speech = 6; 
/* threshold of log-likelihood ratio to declare long pause */

const short GBG_NUM_AFTER_PEAK_TO_TRIGGER_END_SPEECH = 150; 
/* the maximum number of ending_cnt, which is increased after peak of LLR, 
   that is allowed for VAD. 
   above the number will trigger EOS */

/* parameters for end-of-speech detection */
const short GBG_END_SPEECH_START = 30; /* the minimum number of frame counter in decoder */
const short GBG_END_SPEECH_CNT_TH = 80; 
/* the number of frames that are continuously below the threshold */
const ushort GBG_END_SPEECH_TH_PERC = 3277; /* threshold of LLR to declare EOS, 
					       the threshold is computed as a certain
					       percentage of the maximum LLR. 
					       10% in Q15 */

/* parameters for updating reference model */
const short GBG_post_threshold = 6; /* 10% in Q6 */

const short  GS_LOW_SCORE = -8104; /* the default lowest score */

#ifdef RAPID_JAC
void rj_noise_compensation( gmhmm_type* gv)
{ 
  short j;
  short imeans, *sp;
  short mu16[MAX_DIM];
  OFFLINE_VQ_HMM_type* pOffline = (OFFLINE_VQ_HMM_type*)gv->offline_vq; 
  imeans = pOffline->n_cs; 
  Boolean b8bitMean = FALSE; 

  if (pOffline->bCompensated) return; 

  for (j=0;j<imeans;j++){
    sp = pOffline->vq_centroid + j * gv->nbr_dim; 
    pmc( sp, gv->log_N, mu16,
	 gv->log_H, 1, gv->n_mfcc, gv->n_filter, gv->muScaleP2, gv->scale_mu, gv->cosxfm, 
	 pOffline->vq_bias + j * gv->nbr_dim, b8bitMean);
  }
}


/*----------------------------------------------------------------
  rj_compensate

 compensate triphone mean with the cluster-dependent JAC bias vectors 
 ----------------------------------------------------------------*/
TIesrEngineJACStatusType rj_compensate(gmhmm_type * gv)
{
  OFFLINE_VQ_HMM_type *vqhmm = (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  short mixture[4], *mu, *mu8, *sp_bias; 

#ifdef BIT8MEAN 
  short mu16[MAX_DIM];
#endif

  short  mumu[MAX_DIM], k, i, j;

  if (vqhmm->bCompensated)
     return eTIesrEngineJACSuccess; 

#ifdef USE_16BITMEAN_DECOD
  {
     TIesrEngineStatusType loadStatus;
     loadStatus = load_mean_vec(gv->chpr_Model_Dir, gv, FALSE); 
     if( loadStatus != eTIesrEngineSuccess )
     {
	return eTIesrEngineJACRJLoadFail;
     }
  }
#endif

  for (i = 0; i < gv->n_mu; i++){
    mixture[2] = i; 
#ifndef BIT8MEAN
    mu = get_mean(gv->base_mu_orig, mixture, gv->nbr_dim, 0);
#else
    mu8 = get_mean(gv->base_mu_orig,mixture, gv->nbr_dim, 0);
    vector_unpacking((const unsigned short *)mu8, mu16, gv->scale_mu, gv->n_mfcc);
    mu = mu16;
#endif
    /* mean vectors are in Q11 */
    
    j = (short) vqhmm->pRAM2Cls[i];  /* locate ROM mean index to cluster index mapping */

    /* get the bias, obtained from function rs_noise_compensation for monophones  */
    sp_bias = vqhmm->vq_bias + j * gv->nbr_dim; 

    /* compensate bias */
    for (k=gv->nbr_dim-1;k>=0;k--) 
       mumu[k] = mu[k] + sp_bias[k];

    /* save the compensated mean  */
    mu8 = get_mean(gv->base_mu, mixture, gv->nbr_dim, 0);
#ifdef BIT8MEAN
    vector_packing(mumu, (ushort*) mu8, gv->scale_mu, gv->n_mfcc);
#else
    for (k=gv->nbr_dim-1;k>=0;k--) 
       mu8[k] = mumu[k]; 
#endif
  }
  vqhmm->bCompensated = TRUE; 

#ifdef USE_16BITMEAN_DECOD
  if( gv->base_mu_orig )
  {
     free(gv->base_mu_orig);
     gv->base_mu_orig = NULL;
  }
#endif

  return eTIesrEngineJACSuccess;
}

#endif /* #ifdef RAPID_JAC */



#ifdef OFFLINE_CLS
/*----------------------------------------------------------------------------
 rj_offline_inv_of_average_var

 Calculate the average variance vector over all variance vectors use in the
 grammar, and calculate the Gaussian pdf normalization constant based on the
 average variance. This defines the global variance over the set of Gaussians,
 and a global distribution.

 Arguments:
 gv: Pointer to ASR instance structure
 vqhmm: Pointer to Gaussian VQ structure

 -----------------------------------*/
static void rj_offline_inv_of_average_var(gmhmm_type * gv, OFFLINE_VQ_HMM_type *vqhmm)
{
  register short i, j;
  short mixture[4];
#ifdef BIT8VAR
  short invvar[ MAX_DIM ];
#endif
  short *inv;
  long ave_cov[MAX_DIM], ltmp;
  ushort stmp, Q15 = 32767;
  
  for (j=gv->nbr_dim-1;j>=0;j--) ave_cov[j] = 0;
  for (i=gv->n_var-1;i>=0;i--){
    mixture[2] = i; mixture[3] = i; 
    inv = get_var(gv->base_var_orig, mixture, gv->nbr_dim, 0);
#ifdef BIT8VAR
    vector_unpacking((unsigned short *)inv, invvar, gv->scale_var, gv->n_mfcc);
    inv = invvar; 
#endif

    for (j=gv->nbr_dim-1;j>=0;j--){
      if (inv[j] == 0) stmp = Q15; 
      else
	stmp = Q15/inv[j]; /* Q6 */
      ave_cov[j] += stmp; 
    }
  }
  for (j=gv->nbr_dim-1;j>=0;j--){
    //    ltmp = (ave_cov[j])/gv->n_var; /* Q6 */
    ltmp = (ave_cov[j])/vqhmm->n_cs; /* Q6, energy conservation */
    ltmp = (ltmp==0)?Q15:((long)Q15)/ltmp; /* Q9 */
    vqhmm->inv_ave_var[j] = LONG2SHORT(ltmp); 
  }

  vqhmm->gconst = 
    gauss_det_const( vqhmm->inv_ave_var, 2, gv->muScaleP2, gv->nbr_dim);
}


/*---------------------------------------------------------------------------
 rj_open_ROM

 Read the vq clustering information from files prepared offline. The clustering
 information includes the number of Gaussian vectors in the active grammar,
 the class associated with each Gaussian mean vector in the grammar, the total
 number of vq classes, and the Gaussian mean vector centroid associated with
 each vq class.

 Allocate space in the vq HMM structure to hold the data for rapid jac
 calculation, defining an online reference model and possibly updating it, and
 using vq distributions as part of voice activity detection.


 Arguments:
 gv: Pointer to ASR instance structure

 ---------------------------------*/
static TIesrEngineStatusType rj_open_ROM( gmhmm_type *gv )
{
  Boolean failed = FALSE; 

#ifdef OFFLINE_CLS
  char buf[MAX_STR], *dirname = gv->chpr_Model_Dir;
  unsigned char *pChr; 
  FILE *fp;
  OFFLINE_VQ_HMM_type * pOffline = (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  short nread, *p_short; 

  /* read table mapping RAM mean to cluster */
  strcpy( buf, dirname);
  strcat( buf, "/o2amidx.bin" );
  fp = my_fopen( buf, "rb" );
  failed |= ( fp == NULL );
  my_fread(&pOffline->sz_RamMean, sizeof(short), 1, fp);
  nread = ((pOffline->sz_RamMean + 1) >> 1) + 1;
  p_short = mem_alloc( pOffline->base_mem, &pOffline->mem_count, 
		       nread, VQHMM_SIZE, SHORTALIGN, "RAM2Cls");
  pOffline->pRAM2Cls = (unsigned char*) p_short; 
  pChr = (unsigned char*) pOffline->pRAM2Cls; 
  nread = my_fread(pChr, sizeof(unsigned char), pOffline->sz_RamMean, fp);
  failed |= (nread != pOffline->sz_RamMean); 
  my_fclose( fp );


  /* read cluster to ROM mean mapping */
  strcpy( buf, dirname);
  strcat( buf, "/vqcentr.bin"); 
  fp = my_fopen( buf, "rb" );
  failed |= ( fp == NULL );
  my_fread(&pOffline->n_cs, sizeof(short), 1, fp);
  my_fread(&pOffline->nbr_dim, sizeof(short), 1, fp); 
  nread = pOffline->n_cs * pOffline->nbr_dim;
  pOffline->vq_centroid = mem_alloc( pOffline->base_mem, &pOffline->mem_count, 
				     nread, VQHMM_SIZE, SHORTALIGN, "VQ Centroid");
  nread = my_fread(pOffline->vq_centroid, sizeof(short), pOffline->n_cs * pOffline->nbr_dim, fp);
  failed |= (nread != pOffline->n_cs * pOffline->nbr_dim);
  my_fclose( fp );
 
  pOffline->vq_bias = mem_alloc( pOffline->base_mem, &pOffline->mem_count, 
				 pOffline->n_cs * pOffline->nbr_dim, VQHMM_SIZE, 
				 SHORTALIGN, "VQ bias");
  failed |= (pOffline->vq_bias == NULL);

  pOffline->dist = mem_alloc( pOffline->base_mem, &pOffline->mem_count, 
			      pOffline->n_cs, VQHMM_SIZE, 
			      SHORTALIGN, "VQ dist");
  failed |= (pOffline->dist == NULL);

  pOffline->inv_ave_var = mem_alloc( pOffline->base_mem, &pOffline->mem_count, 
			      pOffline->nbr_dim, VQHMM_SIZE, 
			      SHORTALIGN, "VQ inv");
  failed |= (pOffline->inv_ave_var == NULL);

  pOffline->category = (char*)mem_alloc( pOffline->base_mem, &pOffline->mem_count,
				  (pOffline->n_cs + 1)>>SHORTSHIFT, VQHMM_SIZE, SHORTALIGN, "VQ category");
  failed |= (pOffline->category == NULL); 

  pOffline->bCompensated = FALSE; 

#ifdef USE_ONLINE_REF
  pOffline->imax_10 = (short*)mem_alloc(pOffline->base_mem, &pOffline->mem_count,
					GBG_NUM_IDS, VQHMM_SIZE, SHORTALIGN, "ORM index obtained from the current utterance");
  failed |= (pOffline->imax_10 == NULL); 

#endif

#ifdef USE_ORM_PU
  pOffline->iprior = (short*) mem_alloc(pOffline->base_mem, &pOffline->mem_count,
					3 * GBG_NUM_IDS, VQHMM_SIZE, SHORTALIGN, 
					"ORM index after posterior updating");
  failed |= (pOffline->iprior == NULL); 

  pOffline->pprior = (short*) mem_alloc(pOffline->base_mem, &pOffline->mem_count,
					3 * GBG_NUM_IDS, VQHMM_SIZE, SHORTALIGN, 
					"ORM index probabilities");
  failed |= (pOffline->pprior == NULL); 
#endif

#ifdef USE_ORM_VAD
  pOffline->mfcc_buf = (short*)mem_alloc( pOffline->base_mem, &pOffline->mem_count,
					  MFCC_BUF_SZ * MAX_DIM, VQHMM_SIZE, SHORTALIGN, "MFCC buffer");
  failed |= (pOffline->mfcc_buf == NULL); 
  
#endif

#endif	

  if (failed)
    return eTIesrEngineVQHMMMemorySize;
  else return  eTIesrEngineSuccess;
}


/*----------------------------------------------------------------------------
 rj_vq

 Initialize rapid JAC Gaussian vector quantization class information, allocate
 space for JAC, ORM and VAD calculations, and determine global variance of
 Gaussian components used in active grammar.

 Arguments:
 gv: Pointer to ASR instance structure

 -------------------------------------*/
static TIesrEngineStatusType rj_vq(gmhmm_type * gv)
{
  TIesrEngineStatusType bSucc = eTIesrEngineVQHMMMemorySize; 

  /* Read Gaussian vq class data and allocate space for calculations */
  if ((bSucc = rj_open_ROM(gv))!= eTIesrEngineVQHMMMemorySize)
    
     /* prepare VQ HMM. Determine global average variance and Gaussian const. */
    rj_offline_inv_of_average_var(gv, (OFFLINE_VQ_HMM_type*) gv->offline_vq);

  return bSucc; 
}

#endif /* #ifdef OFFLINE_CLS */



#if defined( USE_GAUSS_SELECT )

/* @param perc_core_clusters_q15 precentage of core clusters, value in Q15,
   @param perc_inter_clusters_q15 percentage of intermediate clusters, value in Q15 
   @param num_frms_gbg: number of begining frames to construct gabage model 
*/
Boolean rj_set_param( ushort perc_core_clusters_q15,
		      ushort perc_inter_clusters_q15, 
		      gmhmm_type * gv )
{

//#ifdef RAPID_JAC
  OFFLINE_VQ_HMM_type * vqhmm = (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  
  vqhmm->sVQ_CLS_TO_EVAL =  q15_x(perc_core_clusters_q15, vqhmm->n_cs);
  vqhmm->sVQ_NUM_INTER = q15_x(perc_inter_clusters_q15, vqhmm->n_cs) ;
//#endif

  return TRUE; 
}

#endif /* #if defined( USE_GAUSS_SELECT ) */


#ifdef OFFLINE_CLS
/* @param l_cnter pointer to a counter, each time, the function is evaluated, the counter will be added with one. If the pointer is NULL, no operation on the counter. 
   @param iargmix the mixture index corresponding to the largest likelihood score
   the function evaluates CI/GI-HMM score */
short rj_gauss_obs_score_f(short *feature, int pdf_idx, gmhmm_type*gv, long * l_cnter, short * iargmix)
{
   short total_scr = 0;

#ifdef RAPID_JAC
  register short i, l, k, m;
  short *mixture;
  short nbr_dim = gv->nbr_dim;
  short n_mix;
  short *p2wgt;
#ifndef BIT8MEAN
  register short *mu, *invvar, *feat, j;
  short gconst;
  short diff_s;
  long  diff;
#endif
  long  scr, min_scr = LZERO;
  OFFLINE_VQ_HMM_type * vqhmm = (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  
  total_scr = BAD_SCR;

  mixture = GET_MIX(gv,pdf_idx);
  n_mix = MIX_SIZE(mixture);
  p2wgt = mixture + 1;

  //  for (i = 0; i < n_mix; i++) {      /* n mixtures */
  for (i = n_mix-1; i >= 0; i--) {      /* n mixtures */
    k = mixture[i * 3 + 2];
    scr=(long) gv->gauss_scr[ k ] ;
    
    if (scr == LZERO) {
#ifdef  USE_GAUSS_SELECT
      l = (short)vqhmm->pRAM2Cls[k]; /* RAM mean index -> cluster index */
      if (vqhmm->category[l] == 0){
#endif
#ifndef BIT8MEAN
	mu = get_mean(gv->base_mu, mixture, nbr_dim, i);   
	invvar = get_var(gv->base_var, mixture, nbr_dim, i);
	gconst = gv->base_gconst[ mixture[ i * 3 + 3 ] ];
	feat = feature;
            
	GAUSSIAN_DIST(gv, scr, feat, j, nbr_dim, diff, diff_s, gconst, mu, invvar);
#else
	GAUSSIAN_DIST(gv, scr, nbr_dim, mixture, i, feature);
#endif

	if (l_cnter) (*l_cnter)++;
#if defined(USE_GAUSS_SELECT) || defined(USE_ONLINE_REF)
	vqhmm->low_score = MIN(vqhmm->low_score, scr);
#if defined(USE_GAUSS_SELECT)
      }
      else
	scr = (long) vqhmm->dist[l];
#endif
#endif

#if defined(USE_ONLINE_REF) || defined(DEV_CLS2PHONE)
      if (gv->frm_cnt > vqhmm->inum_frms_gbg){
#if defined(DEV_CLS2PHONE)
	if (vqhmm->bRefCloseToSilence == FALSE)
#endif

#ifdef USE_ORM_PU
	for (m=vqhmm->inum_prior-1;m>=0;m--){
	  if (l == vqhmm->iprior[m]){ /* belongs to the ORM model */
	    scr = MAX(scr, vqhmm->ga_score);
	    break; 
	  }
	}
#else
	for (m=vqhmm->inum_diff_cls-1;m>=0;m--){
	  if (l == vqhmm->imax_10[m]){ /* belongs to the ORM model */
	    scr = MAX(scr, vqhmm->ga_score);
	    break; 
	  }
	}
#endif
      }

#endif

      gv->gauss_scr[k] = LONG2SHORT(scr); 
    }
    
    if (iargmix && scr > min_scr){
      min_scr = scr; *iargmix = i; 
    }


    scr += p2wgt[ i * 3 ];         /* mixture weight */

    if ( scr > (long) total_scr ) total_scr = LONG2SHORT(scr);
  }

#endif

  return total_scr;
}

#endif  /* OFFLINE_CLS */



#if defined(RAPID_JAC) || defined(USE_CONFIDENCE)

/* got the approximated Gaussian score */
static short rj_dist(short *mfcc_feat, short* mean, short * inv, short gconst, short dm)
{
  register long td; 
  register long dist = 0; 
  register short i; 

  for (i=dm-1;i>=0;i--){
    td = mfcc_feat[i] - mean[i]; /* Q11 */
    td *= td; /* Q22 */
    td >>= 11; /* Q11 */
    td *= inv[i]; /* Q20 */
    td >>= 14 ; /* Q6 */
    dist -= td; 
  }
  dist -= gconst; 
  dist >>= 1; /* times 0.5 */
  
  return LONG2SHORT(dist); 
}

#endif /* defined(RAPID_JAC) || defined(USE_CONFIDENCE) */


#ifdef DEV_CLS2PHONE
/* check if the reference model contains silence model,
   if so, return TRUE,
   else return FALSE */
static Boolean rj_reference_close_to_silence_model(OFFLINE_VQ_HMM_type* vqhmm)
{
  short i, j, k, inum = vqhmm->inum_frms_gbg; 
  short m, bCount = 0; 
  Boolean bClose = FALSE; 

  for (i=vqhmm->inum_diff_cls-1;i>=0;i--){
    m = vqhmm->imax_10[i];
    j=(short)vqhmm->cls_nphone[m];
    //    printf("cls[%d] -> phone: ", m);
    for(--j;j>=0;j--){
      k = (short)vqhmm->cls_phone[PHONEPERCLS * m + j];
      //printf(" %d ", k);
      if (k==0){
	bCount ++; 
	if (bCount > GBG_NUM_SIL){ bClose = TRUE; break; }
      }
    }
    //    printf(" Close = %d\n", bClose);
  }
  return bClose; 
}

/* obtain cluster to phone mapping */
static void rj_cls2phone(gmhmm_type * gv)
{
  HmmType * hmm; 
  ushort hmmcode, hmm_dlt = 0, n_state, pdf_idx; 
  short n1, *mixture, n_mix, i, k, l, j, m, monophone_index;
  short * hmm2phone =  sbc_rtn_hmm2phone(gv->sbcState);
  Boolean bFound ; 
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;

  for (hmmcode = 0; hmmcode < gv->trans->n_hmm_set;hmmcode++){
    hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt);

    monophone_index = *( hmm2phone + hmmcode ); 
    monophone_index++; 

    n_state = NBR_STATES(hmm,gv->base_tran);
      
    n1 =  n_state - 1;

    for (j = 0; j < n1; j++){
      
      pdf_idx = hmm->pdf_idx[ j ];

      mixture = GET_MIX(gv,pdf_idx);
      n_mix = MIX_SIZE(mixture);
      
      for (i = n_mix-1; i >= 0; i--) {      /* n mixtures */
	k = mixture[i * 3 + 2]; /* mean index */
	l = (short)vqhmm->pRAM2Cls[k]; /* corresponding cluster index */
	bFound = FALSE; 
	for (m=((short)vqhmm->cls_nphone[l])-1;m>=0;m--){
	  if ((short)vqhmm->cls_phone[l*PHONEPERCLS + m] == monophone_index){ 
	    bFound = TRUE; break; 
	  }
	}
	if (bFound == FALSE){
	  m = (short)vqhmm->cls_nphone[l];
	  m++; 
	  if (m>=PHONEPERCLS){
	    printf("out of boundary \n");
	    m = PHONEPERCLS; 
	  }
	  vqhmm->cls_nphone[l] = (char)m;
	  vqhmm->cls_phone[l * PHONEPERCLS + m - 1] = (char)monophone_index; 
	}
      }
    }
  }
}
#endif


#ifdef USE_ONLINE_REF
/*----------------------------------------------------------------------------
 rj_load

 Load vq class indices and class probabilities for the vq classes currently
 comprising the ORM model.

 Arguments:
 gv: Pointer to ASR instance structure
 pf: File pointer to open JAC state file
 -------------------------------------*/
TIesrEngineStatusType rj_load(gmhmm_type * gv, FILE * pf)
{
#ifdef USE_ORM_PU
  size_t nread;
  OFFLINE_VQ_HMM_type* vqhmm = ( OFFLINE_VQ_HMM_type* ) gv->offline_vq;
  
  if (vqhmm && pf){
    my_fread(&(vqhmm->inum_prior), sizeof(short), 1, pf);
    if (vqhmm->inum_prior > 0){

      nread = my_fread(vqhmm->iprior, sizeof(short), vqhmm->inum_prior, pf);
      if( nread != (size_t)vqhmm->inum_prior )
	return eTIesrEngineVQHMMLoad;

      nread = my_fread(vqhmm->pprior, sizeof(short), vqhmm->inum_prior, pf);
      if( nread != (size_t)vqhmm->inum_prior )
	return eTIesrEngineVQHMMLoad; 

    }
  }
#endif
  return eTIesrEngineSuccess;
}

#endif /* USE_ONLINE_REF */

#ifdef USE_ONLINE_REF
TIesrEngineStatusType rj_save(gmhmm_type * gv, FILE * pf)
{
#ifdef USE_ORM_PU
  size_t nwrite;
  OFFLINE_VQ_HMM_type* vqhmm = ( OFFLINE_VQ_HMM_type* ) gv->offline_vq;
  if (vqhmm && pf){
    my_fwrite(&(vqhmm->inum_prior), sizeof(short), 1, pf);
    if (vqhmm->inum_prior > 0){
      nwrite = my_fwrite(vqhmm->iprior, sizeof(short), vqhmm->inum_prior, pf);
      if( nwrite != (size_t)vqhmm->inum_prior )
	return eTIesrEngineVQHMMSave;
      nwrite = my_fwrite(vqhmm->pprior, sizeof(short), vqhmm->inum_prior, pf);
      if( nwrite != (size_t)vqhmm->inum_prior )
	return eTIesrEngineVQHMMSave; 
    }
  }
#endif
  return eTIesrEngineSuccess;
}

#endif /* USE_ONLINE_REF */



#ifdef USE_ORM_PU
/* update prior probability of the reference model */
static void rj_update_prior(OFFLINE_VQ_HMM_type* vqhmm)
{
  short i, j, k, inum = vqhmm->inum_frms_gbg; 
  short imax[GBG_NUM_IDS], iprev_num = inum;
  short cnt[GBG_NUM_IDS]; 
  short npost = 0, prb_sum = 0;

  /* back up */
  for (i=inum-1;i>=0;i--) {
    imax[i] = vqhmm->imax_10[i]; 
    cnt[i] = 0; 
  }

  /* compact the reference model of this utterance */
  inum = uniq(vqhmm->imax_10, NULL, vqhmm->inum_frms_gbg);
#if defined(DEV_NONSTATIONARY) || defined(DEV_PRIOR_SHOW) 
  printf("current cluster: ");
  for (i=0;i<inum;i++) 
    printf("%d ", vqhmm->imax_10[i]);
  printf("\n");
#endif

  /* obtain prabilities of clusters in the reference model in this utterance */
  for (i=iprev_num-1;i>=0;i--){
    for (j=inum-1;j>=0;j--)
      if (imax[i] == vqhmm->imax_10[j])
	cnt[j] ++; 
  }
  for (i=inum-1;i>=0;i--){
    j = (cnt[i] << 6); 
    cnt[i] = j / iprev_num; /* the probability of cluster i */
  }

  /* prepare the posterior */
  j = vqhmm->inum_prior; 
#ifdef TEST_PU
  if (j==0){
    vqhmm->inum_prior = 14; 
    short fakeidx[14]={0, 45, 50, 62 , 30, 127, 32, 39, 107, 65,  124, 64, 59, 7 };
    for (i=0;i<vqhmm->inum_prior;i++){
      vqhmm->iprior[i] = fakeidx[i]; 
      vqhmm->pprior[i] = (2<<6)/14;
    }
    j = vqhmm->inum_prior; 
  }
#endif
  for (i=0;i<inum;i++){
    vqhmm->iprior[j + i] = vqhmm->imax_10[i]; 
    vqhmm->pprior[j + i] = cnt[i] ; 
  }


  npost = inum + vqhmm->inum_prior; 

  npost = uniq(vqhmm->iprior, vqhmm->pprior, npost); 

  for (i=npost-1;i>=0;i--)
    prb_sum += vqhmm->pprior[i]; 
  vqhmm->inum_prior = 0; 
  for (i=0;i<npost;i++){
    j = (vqhmm->pprior[i] << 6);
    vqhmm->pprior[i] = j/prb_sum; /* Q6 */
    /* the threshold is 10% in Q6 */
    if (vqhmm->pprior[i] <  GBG_post_threshold) {
      for (k=i;k<npost-1;k++){
	vqhmm->iprior[k] = vqhmm->iprior[k+1];
	vqhmm->pprior[k] = vqhmm->pprior[k+1]; 
      }
      npost--;
      i--;
      continue; 
    }else{
      if (vqhmm->inum_prior >= 3*GBG_NUM_IDS) 
	break; /* no more space for saving the reference clusters */
      vqhmm->inum_prior++;
    }
  }

#ifdef  DEV_PRIOR_SHOW
  printf("updated ");
  for (i=0;i<vqhmm->inum_prior;i++)
    printf("%d ", vqhmm->iprior[i]);
#endif

}

#endif /* USE_ORM_PU */



#ifdef USE_ORM_VAD

static Boolean rj_low_conf_seg(OFFLINE_VQ_HMM_type* vqhmm, gmhmm_type * gv)
{
  if (vqhmm->is_speech < u_low_is_speech){
    vqhmm->i_low_is_speech ++; 
  }else vqhmm->i_low_is_speech = 0; 

  if (vqhmm->i_low_is_speech > GBG_NUM_SIL) return TRUE; 
  else return FALSE; 
}

/* GMM-based VAD
   Speech event is triggered by a preliminary energy-based method. This function further
   verifies the event. 
   A speech event is verified if
   1) log-likeliood ratio of the best matched cluster relative to a reference model is
   above a certain threshold. 
   The threshold dependes on the background noise level. 
   After verification of speech event, a long pause may occur. This function thus detects
   the exisitence of long pause by looking at how many frames of speech falls below a threshold
   of log-likelihood ratio. If so, the VAD is checked as FALSE, and incoming frames are
   buffered untill a speech event is verified again by the function. 
   
*/
static Boolean rj_voice_begining(gmhmm_type * gv)
{
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  NssType* pNss = (NssType*) gv->pNss ; 
  short noiselvl = pNss->noiselvl;

  if (vqhmm->bVADVerified == FALSE ){
    /* noise-dependent threshold */
    if ((noiselvl >= vqhmm->th_noise_lvl) && (vqhmm->is_speech >  vqhmm->th_llr_in_high_noise))
      vqhmm->bVADVerified = TRUE; 
    else 
      if ((noiselvl < vqhmm->th_noise_lvl) && (vqhmm->is_speech >  vqhmm->th_llr_in_low_noise))
	vqhmm->bVADVerified = TRUE; 
  }else
    /* check if LLRs of the incomming frames continiously below a threshold, then trigger
       VAD again */
    if (rj_low_conf_seg(vqhmm, gv) == TRUE) 
      vqhmm->bVADVerified = FALSE; 

  /* could trigger end of speech because of such long segments of low confidence */
  if (vqhmm->is_speech < vqhmm->ending_llr_th){
    vqhmm->ending_low_llr_cnt++;
  }else vqhmm->ending_low_llr_cnt = 0;
  
  if (vqhmm->ending_low_llr_cnt > vqhmm->ending_cnt_th)
    gv->gbg_end_of_speech = TRUE; 

  if (vqhmm->ending_cnt > vqhmm->ending_cnt_after_peak)
    gv->gbg_end_of_speech = TRUE; 

  return vqhmm->bVADVerified; 
}

static void rj_voice_buffering(gmhmm_type* gv, short *mfcc_feat)
{
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  short idx = vqhmm->voice_idx, j; 

  if (vqhmm->bVADVerified == FALSE){

    idx = circ_idx( idx ); 
    for (j=gv->nbr_dim-1;j>=0;j--)
      vqhmm->mfcc_buf[idx * gv->nbr_dim + j] = mfcc_feat[j] ; 
    vqhmm->voice_idx++; 
    if (vqhmm->voice_idx <= MFCC_BUF_SZ)
      vqhmm->iPushed ++; 
    vqhmm->bSynced = FALSE; 
  }
}

static short* rj_vad_pop_buf(gmhmm_type *gv)
{
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  short idx = vqhmm->voice_idx; 

  if (vqhmm->bSynced) return NULL; 

  /* several frames has already passed detection, go back */
  if (vqhmm->iPushed > 0)
    idx = circ_idx( idx - vqhmm->iPushed ); 
  else idx = -1; 

  if (idx >= 0){
    vqhmm->iPushed--; 
    return &(vqhmm->mfcc_buf[idx * gv->nbr_dim]) ; 
  }
  else{
    vqhmm->bSynced = TRUE; 
    vqhmm->voice_idx = 0;
    return NULL; 
  }
}


/* GMM-based VAD
   if SAD is not verified, buffer the input speech and return NULL,
   if SAD is verified, then pop up buffered speech frames untill the buffer is clear. 
   the status is called synchronized to the current frame. After synchronization, 
   just forward the incoming frame as the output. 
   
*/
TIESRENGINECOREAPI_API short* rj_vad_proc(gmhmm_type * gv, short * mfcc_feat)
{
#ifdef USE_ORM_VAD
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  Boolean bSpeech = rj_voice_begining(gv);
  short * pMfcc;

  if (bSpeech == FALSE) {
    rj_voice_buffering(gv, mfcc_feat);
    return NULL; 
  }
  else{
    pMfcc = rj_vad_pop_buf(gv);
    if(pMfcc) {
      /* compute cluster scores for the popped frame */
      rj_clear_obs_scr(gv, pMfcc);
      
      return pMfcc;
    }
    else{
      if (vqhmm->b_orig_done == FALSE){
	vqhmm->b_orig_done = TRUE; 
	return mfcc_feat; 
      }else{
	vqhmm->b_orig_done = FALSE ; 
	return NULL;
      }
    }
  }
#else
  return NULL;
#endif
}

#endif /* USE_ORM_VAD */


#if defined(USE_GAUSS_SELECT) || defined(USE_ORM_VAD)
/* The function conducts two operations: 
   1) if the current frame number is smaller than a threshold,
   it records the best matched Gaussian cluster and put it to construct a reference model
   2) else, it updates score of the online reference model 
   Also, it computes the log-likelihood ratio of the best matched cluster w.r.t.
   the reference model
*/
TIESRENGINECOREAPI_API void rj_update_noise(gmhmm_type * gv, short * mfcc_feat, short imax)
{
#ifdef USE_ONLINE_REF
  register short j; 
  register short bkscore = LZERO;
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;

  if (vqhmm->fd_frms  >= vqhmm->inum_frms_gbg){
#ifdef USE_ORM_PU
    for (j=vqhmm->inum_prior-1;j>=0;j--){
      bkscore = MAX(bkscore, vqhmm->dist[vqhmm->iprior[j]]); 
    }
#else
    for (j=vqhmm->inum_diff_cls-1;j>=0;j--){
      bkscore = MAX(bkscore, vqhmm->dist[vqhmm->imax_10[j]]);
    }
#endif
  }

  if (vqhmm->fd_frms < vqhmm->inum_frms_gbg){
    vqhmm->imax_10[vqhmm->fd_frms] = imax; 
    bkscore = 0;
    vqhmm->ga_score = LZERO; 

    if (vqhmm->fd_frms == (vqhmm->inum_frms_gbg - 1)){
#ifdef USE_ORM_PU
      rj_update_prior(vqhmm); 
#else
      vqhmm->inum_diff_cls = uniq(vqhmm->imax_10, NULL, vqhmm->inum_frms_gbg);
#endif
#ifdef DEV_CLS2PHONE
      vqhmm->bRefCloseToSilence = rj_reference_close_to_silence_model(vqhmm);
#endif

#if defined(DEV_NONSTATIONARY) || defined(DEV_PRIOR_SHOW) 
      if (vqhmm->inum_diff_cls > 0){
	printf("current cluster: ");
	for (j=0;j<vqhmm->inum_diff_cls;j++) 
	  printf("%d ", vqhmm->imax_10[j]);
	printf("\n");
      }
#endif
    }
  }else{
    vqhmm->ga_score = bkscore; 
    bkscore = vqhmm->max_dist - bkscore; 
  }

  vqhmm->is_speech >>= 1; 
  vqhmm->is_speech += (bkscore >> 1);

#ifdef USE_ORM_VAD
  if ((vqhmm->is_speech < vqhmm->max_llr) && ( gv->frm_cnt > GBG_END_SPEECH_START )){
    vqhmm->ending_cnt ++; 
  }else{
    vqhmm->ending_cnt = 0; 
    vqhmm->i_low_is_speech = 0;
    vqhmm->ending_low_llr_cnt = 0; 
    vqhmm->max_llr = MAX(vqhmm->max_llr, vqhmm->is_speech); 
    vqhmm->ending_llr_th = q15_x(GBG_END_SPEECH_TH_PERC, vqhmm->max_llr);
  }
#endif

#ifdef DEV_NONSTATIONARY
  FILE *fp;
  short sp_log_ps[ N_FILTER26 ];

  /* get the S^-1 * C^-1 in log-spectral domain */
  cos_transform(mfcc_feat, sp_log_ps, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */

  fp = my_fopen("logs/tstnoisy.dat","ab");
  my_fwrite(sp_log_ps, sizeof(short), gv->n_filter, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstclean.dat","ab");
  my_fwrite(&vqhmm->ga_score, sizeof(short), 1, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstlog.dat","ab");
  my_fwrite(&vqhmm->is_speech, sizeof(short), 1, fp);
  my_fclose(fp);

#ifdef  USE_ORM_VAD
  fp = my_fopen("logs/tstcleaned.dat","ab");
  my_fwrite(&vqhmm->bVADVerified, sizeof(short), 1, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstnoise.dat","at");
  if (vqhmm->bVADVerified)
    fprintf(fp, "%d\n", imax); 
  else fprintf(fp, "%d\n", -1);
  my_fclose(fp);
#endif

#endif

  vqhmm->fd_frms ++; 

#endif
}

#endif



#ifdef DEV_ONLINE_GARBAGE
void SFB_rj_update_noise(gmhmm_type * gv, short * mfcc_feat)
{
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  short i, imin, u07 = 22938, j; 
  short mindist= LZERO, Q15 = 32767;
  short bkscore = LZERO;
  short *sp, *spbias, x[MAX_DIM], s_sqrt[MAX_DIM], po[MAX_DIM]; 
  short sp_log_inv[N_FILTER26], sp_log_ps[ N_FILTER26 ], x_log_ps[N_FILTER26],
    noisy_log[N_FILTER26], func_d[N_FILTER26];        /* log power spectrum */  
  long ltmp, accum, ltmp2, *accum_d ;//= vqhmm->lAccum; 
  short a01 = 6554, a10 = 3277, a11 = 32767 - a10, a00 = 32767 - a01; 
  short ifactor; 

  for (i=vqhmm->n_cs -1; i>=0; i--){
    if (vqhmm->dist[i] > mindist) {
      mindist = vqhmm->dist[i]; 
      imin = i; 
    }
    if (gv->frm_cnt > 10){
      for (j=9;j>=0;j--){
	bkscore = MAX(bkscore, vqhmm->dist[vqhmm->imax_10[j]]);
      }
    }
  }
  
  if (gv->frm_cnt <= 10){
    vqhmm->imax_10[gv->frm_cnt - 1] = imin; 
    bkscore = 0;
    vqhmm->ga_score = LZERO; 
  }else{
    vqhmm->ga_score = bkscore; 
    //    bkscore = MAX(0, mindist - bkscore); 
    bkscore = mindist - bkscore; 
  }
    
  /* get the compensated mean */
  sp = vqhmm->vq_centroid + imin * gv->nbr_dim; 
  spbias = vqhmm->vq_bias + imin * gv->nbr_dim; 
  for (i=vqhmm->nbr_dim-1;i>=0;i--){
    x[i] = sp[i] + spbias[i]; 
    x[i] = mfcc_feat[i] - x[i]; 
    ltmp = x[i] *  vqhmm->inv_ave_var[i]; /* Q11 * Q9 = Q20 */
    x[i] = LONG2SHORT(LONG_RIGHT_SHIFT_N(ltmp, 9));
    s_sqrt[i] = fx_sqrt(vqhmm->inv_ave_var[i]<< 1); /* Q5 */
    s_sqrt[i] <<= 6; /* Q11 */
  }

  /* get the (y-x)*S^-2*C^-1 in log-spectral domain */
  cos_transform(x, sp_log_ps, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */

  /* get the S^-1 * C^-1 in log-spectral domain */
  cos_transform(s_sqrt, sp_log_inv, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */

  /* get the S^-1 * C^-1 in log-spectral domain */
  cos_transform(sp, x_log_ps, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */

  /* obtain the first order differential of noisy speech w.r.t. to noise */
  log_spectral_compensation(x_log_ps, NULL, noisy_log, NULL, gv->log_N, gv->log_H, 1, func_d, gv->n_filter);

  for (i=gv->n_filter-1;i>=0;i--){
    ltmp = Q15 - func_d[i]; /*Q15, exp(n)/(exp(n) + exp(h + x)) */
    ltmp *= sp_log_ps[i]; /* Q24 */
    accum = LONG_RIGHT_SHIFT_N(ltmp, 10); /* Q14 */

    ltmp = sp_log_inv[i] * (Q15 - func_d[i]);   /* Q24 */
    ltmp = LONG_RIGHT_SHIFT_N(ltmp, 15); /*Q9*/
    ltmp *= ltmp; /* Q18 */
    ltmp = LONG_RIGHT_SHIFT_N(ltmp, 4); /* Q14 */

    //    ltmp2 = (accum >> 4) * func_d[i]; /* Q25 */
    //ltmp2 = LONG_RIGHT_SHIFT_N(ltmp2, 11); /* Q14 */
    //ltmp = ltmp - ltmp2; /* Q14 */

    accum_d[i] = (((accum_d[i] >> 1) + accum_d[i] )>>1)  + ltmp; 

    if (accum_d[i] > 0)
      ltmp = -div32_32_Q(accum, accum_d[i], 9);
    else ltmp = 0;
    ltmp >>= 1; /* discounting factor*/
    ltmp = MAX(0, ltmp);

    accum = ltmp * (Q15 - func_d[i]); /* Q24 */
    accum = LONG_RIGHT_SHIFT_N(accum, 15); /* Q9 */

    noisy_log[i] = LONG2SHORT(accum);

  }

  inverse_cos_transform(noisy_log, po, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm);

#ifdef DEV_NONSTATIONARY
  /* get the S^-1 * C^-1 in log-spectral domain */
  cos_transform(mfcc_feat, sp_log_inv, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */
#endif

  //  for (i=gv->n_mfcc-1;i>=0;i--) mfcc_feat[i] -= po[i];

  /* update VAD status */

  /*
  ltmp = gv->frm_cnt - 1; 
  ltmp <<= 15;
  ltmp /= gv->frm_cnt; 
    
  vqhmm->is_speech = q15_x(LONG2SHORT(ltmp), vqhmm->is_speech) + bkscore / gv->frm_cnt; 
  */
  vqhmm->is_speech >>= 1; 
  vqhmm->is_speech += (bkscore >> 1);

#ifdef DEV_NONSTATIONARY
  /* get the S^-1 * C^-1 in log-spectral domain */
  cos_transform(mfcc_feat, sp_log_ps, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */

  FILE *fp;
  fp = my_fopen("logs/tstnoise.dat","ab");
  my_fwrite(noisy_log, sizeof(short), gv->n_filter, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstclean.dat","ab");
  my_fwrite(x_log_ps, sizeof(short), gv->n_filter, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstnoisy.dat","ab");
  my_fwrite(sp_log_inv, sizeof(short), gv->n_filter, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstcleaned.dat","ab");
  fwrite(sp_log_ps, sizeof(short), gv->n_filter, fp);
  my_fclose(fp);

  fp = my_fopen("logs/tstlog.dat","ab");
  my_fwrite(&vqhmm->is_speech, sizeof(short), 1, fp);
  //fwrite(&vqhmm->viterbi_weight,sizeof(short), 1, fp);
  my_fclose(fp);
#endif
}
#endif


#if defined(USE_GAUSS_SELECT) || defined(USE_ONLINE_REF)
/* This function conducts scoring of Gaussian clusters,
   It also assigns Gaussian clusters into categories such as core, intermedium, and outmost. */
short rj_clear_obs_scr(gmhmm_type * gv, short *mfcc_feat)
{
   short imax = 0;

  /* find the closest clusters */
  OFFLINE_VQ_HMM_type *vqhmm =  (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  ushort uthre, uthre_exp;
  long  tmp; 
  short k, i, thre , thre_exp, gconst = vqhmm->gconst;
  short * sp_bias, tsp[MAX_DIM], *sp, maxdist = -32767, mindist = 32767; 

  for (i=vqhmm->n_cs-1;i>=0;i--){
    sp = vqhmm->vq_centroid + i * gv->nbr_dim; 
    sp_bias = vqhmm->vq_bias + i * gv->nbr_dim; 
    for (k=vqhmm->nbr_dim-1;k>=0;k--) tsp[k] = sp[k] + sp_bias[k]; 

    vqhmm->dist[i] = rj_dist(mfcc_feat, tsp, vqhmm->inv_ave_var, gconst, gv->nbr_dim); 
    if (vqhmm->dist[i] > maxdist){
      maxdist = vqhmm->dist[i]; 
      imax = i; 
    }
    mindist = MIN(vqhmm->dist[i], mindist);
  }

  /* caculate threshold for the intermediate clusters , above this threshold, 
   clusters are core. Below this threshold, they belong to either intermediate or outmost clusters. */

#if defined(USE_ONLINE_REF) || defined(USE_ORM_VAD)
  k = (vqhmm->fd_frms < 1)? vqhmm->n_cs : vqhmm->sVQ_CLS_TO_EVAL;
#else
  k = (gv->frm_cnt <= 1)? vqhmm->n_cs : vqhmm->sVQ_CLS_TO_EVAL;
#endif

  tmp = k; tmp <<= 5; tmp /= vqhmm->n_cs; 
  tmp *= (maxdist - mindist); tmp >>= 5; 
  uthre = (short) tmp; 
  thre = maxdist - uthre; 

  /* caculate threshold to select out-most clusters*/
  if (vqhmm->sVQ_NUM_INTER == 0) thre_exp = thre;
  else{
    k = MIN(k + vqhmm->sVQ_NUM_INTER, vqhmm->n_cs);
    tmp = k; tmp <<= 5; tmp /= vqhmm->n_cs; 
    tmp *= (maxdist - mindist); tmp >>= 5; 
    uthre_exp = (short) tmp; 
    thre_exp = maxdist - uthre_exp; 
  }

  for (i=vqhmm->n_cs-1;i>=0;i--) {
    if (vqhmm->dist[i] >= thre) vqhmm->category[i] = 0; /* core clusters */
    else if (vqhmm->dist[i] < thre_exp && vqhmm->low_score != -LZERO) {
      vqhmm->dist[i] = vqhmm->low_score ; /* outmost clusters */
      vqhmm->category[i] = 2; 
    }
    else vqhmm->category[i] = 1; /* intermediate clusters */
  }

  /* the correct lowest score is obtained during decoding. 
     if the decoding has not yet conducted, use the GS_LOW_SCORE instead 
     as the lowest score */
  if (gv->frm_cnt > 0)
    vqhmm->low_score = -LZERO; 
  else 
    vqhmm->low_score =  GS_LOW_SCORE;

  vqhmm->max_dist = maxdist; 


  return imax; 
}

#endif /* #if defined(USE_GAUSS_SELECT) || defined(USE_ONLINE_REF) */


#ifdef OFFLINE_CLS
/*----------------------------------------------------------------------------
 rj_init

 Initialize all vq-cluster distribution distances to observed feature vector
 to zero, and initialize parameters for ORM and ORM VAD in preparation for
 processing an utterance.

 Arguments:
 gv: Pointer to ASR instance structure.

 -----------------------------------*/
void rj_init(gmhmm_type *gv)
{
  short i; 
  OFFLINE_VQ_HMM_type * vqhmm = (OFFLINE_VQ_HMM_type*) gv->offline_vq;
  
  for (i = vqhmm->n_cs -1;i>=0;i--) {
    vqhmm->dist[i] = LZERO ; 
#ifdef DEV_CLS2PHONE
    vqhmm->cls_nphone[i] = 0; 
#endif
  }

#if defined(DEV_CLS2PHONE)
  vqhmm->bRefCloseToSilence = TRUE; 
#endif

  vqhmm->low_score = -LZERO;

#ifdef  USE_ONLINE_REF
  vqhmm->is_speech = 0; 
  vqhmm->i_low_is_speech = 0; 
  gv->gbg_end_of_speech = FALSE;

  vqhmm->fd_frms = 0; 

#ifdef USE_ORM_VAD
  vqhmm->bVADVerified = FALSE; 
  vqhmm->voice_idx = 0;
  vqhmm->bSynced= FALSE; 
  vqhmm->iPushed= 0; 
  vqhmm->b_orig_done = FALSE; 

  vqhmm->ending_low_llr_cnt = 0; 
  vqhmm->max_llr = LZERO; 
  vqhmm->ending_cnt = 0;
  vqhmm->ending_llr_th = LZERO; 

#endif

#endif

#ifdef DEV_CLS2PHONE
  rj_cls2phone(gv); 
#endif

#ifdef DEV_NONSTATIONARY
  FILE *fp;
  fp = my_fopen("logs/tstnoise.dat","wb");
  my_fclose(fp);

  fp = my_fopen("logs/tstclean.dat","wb");
  my_fclose(fp);

  fp = my_fopen("logs/tstnoisy.dat","wb");
  my_fclose(fp);

  fp = my_fopen("logs/tstcleaned.dat","wb");
  my_fclose(fp);

  fp = my_fopen("logs/tstlog.dat","wb");
  my_fclose(fp);
#endif
}


/*---------------------------------------------------------------------------
 rj_open
 
 Initialize the vq structure and set parameters in preparation for rapid
 JAC calculations for an utterance, using vector quantized classes of Gaussians
 to determine JAC adaptation parameters.
 
 Arguments:
 
 u_perc_cor: percentage of core clusters, in Q15
 u_perc_inte:percentage of intermediate clusters, in Q15
 gv: Pointer to ASR instance structure

 -----------------------------------*/
TIesrEngineStatusType rj_open(ushort u_perc_cor, 
			      ushort u_perc_inte, 
			      gmhmm_type *gv )
{
  TIesrEngineStatusType bSucc ; 
  OFFLINE_VQ_HMM_type * pOffline = (OFFLINE_VQ_HMM_type*) gv->offline_vq;

  pOffline->mem_count = 0;

  bSucc = rj_vq(gv);
  if (bSucc == eTIesrEngineVQHMMMemorySize) return bSucc;

#ifdef USE_GAUSS_SELECT
   if( rj_set_param( u_perc_cor, u_perc_inte, gv ) == FALSE )
      return eTIesrEngineVQHMMGarbage;
#endif

#ifdef USE_ORM_PU
  pOffline->inum_prior = 0; 
#endif

  return bSucc; 
}

#endif /* ifdef OFFLINE_CLS */


#if defined(USE_ONLINE_REF)
/* setup parameters for online reference modeling, VAD and EOS detection
   @param num_frms_gbg: number of begining frames to construct gabage model 
   @param noise_level : the noise level threshold to choose an LLR threshold
   @param th_llr_low_noise: the LLR threshold in low noise level
   @param th_llr_high_noise: the LLR threshold in high noise level 
   @param ctn_th : number of frames that continuously below a threshold
   @param llr_perc_th : the threshold is computed as $llr_perc_th * max_llr, where max_llr
   is the score of the best matched cluster 
   @param start_from : don't do EOS decision making untill $start_from of frames have
   passed the peak of LLR 
   @param th_nbr_frm_after_peak EOS will be triggered if more than the number of 
   frames have been processed
   return TRUE if sucess
*/
TIESRENGINECOREAPI_API Boolean rj_set_ORM(short num_frms_gbg,
		   short noise_level,
		   short th_llr_low_noise, short th_llr_high_noise,
		   short cnt_th, ushort llr_perc_th, short start_from, 
		   short th_nbr_frm_after_peak, 
		   gmhmm_type * gv)
{
  OFFLINE_VQ_HMM_type * pOffline = (OFFLINE_VQ_HMM_type*) gv->offline_vq;

  if (pOffline){
#ifdef USE_ONLINE_REF
    if ( GBG_NUM_IDS >= num_frms_gbg) {
      pOffline->inum_frms_gbg = num_frms_gbg; 
    }else{
      return FALSE; 
    }
#endif

#ifdef USE_ORM_VAD
    pOffline->th_noise_lvl = noise_level;
    pOffline->th_llr_in_low_noise = th_llr_low_noise; 
    pOffline->th_llr_in_high_noise = th_llr_high_noise; 

    pOffline->ending_cnt_th = cnt_th; 
    pOffline->ending_llr_th = 0; 
    pOffline->ending_llr_perc = llr_perc_th; 
    pOffline->ending_cnt_stt = start_from; 

    pOffline->ending_cnt_after_peak = th_nbr_frm_after_peak;
#endif
  }
  return TRUE; 
}

#endif /* defined(USE_ONLINE_REF) */


#ifdef OFFLINE_CLS
void rj_close(gmhmm_type* gv)
{

  gv->offline_vq = NULL; 

}

#endif /* ifdef OFFLINE_CLS */



#ifdef MODEL_LEVEL_PRUNE
void mlp_set_param(ushort uPercentage, gmhmm_type * gv)
{
  MODEL_PRUNE_type * pMLP = (MODEL_PRUNE_type*)gv->pPhoneLevelPrune; 
  pMLP->uPercentage = uPercentage; 
}

/* reset phone-level pruning */
void mlp_reset(gmhmm_type* gv)
{
  register short i; 
  MODEL_PRUNE_type * pMLP = (MODEL_PRUNE_type*)gv->pPhoneLevelPrune; 

  for (i=MLP_NUM_MONOPHONE-1;i>=0;i--) {
    pMLP->p_s_active[i] = 0;
    pMLP->p_s_cur_mono[i] = LZERO >> 1;
  }
}

#endif

/* update model level prunning */
void mlp_update(gmhmm_type* gv, short hmm_idx, long score)
{
#ifdef MODEL_LEVEL_PRUNE
  short monophone_index; 
  short * hmm2phone =  sbc_rtn_hmm2phone(gv->sbcState);
  short scr = LONG2SHORT(score);
  MODEL_PRUNE_type * pMLP = (MODEL_PRUNE_type*)gv->pPhoneLevelPrune; 

  monophone_index = *( hmm2phone + hmm_idx ); 
  monophone_index++; 

  pMLP->p_s_active[monophone_index] = 1; 

  pMLP->p_s_cur_mono[monophone_index] = MAX(scr, pMLP->p_s_cur_mono[monophone_index]);
#endif
}

/* open phone-level pruning */
void mlp_open(gmhmm_type* gv)
{
#ifdef MODEL_LEVEL_PRUNE
  MODEL_PRUNE_type * pMLP = (MODEL_PRUNE_type*)gv->pPhoneLevelPrune; 

  /* pMLP->mem_count = 0; */
  /* pMLP->p_s_active = mem_alloc( pMLP->base_mem, &pMLP->mem_count,  */
  /*   			MLP_NUM_MONOPHONE, MLP_SIZE, SHORTALIGN, "active phone idx"); */
  /* pMLP->p_s_cur_mono = mem_alloc( pMLP->base_mem, &pMLP->mem_count,  */
  /*   			  MLP_NUM_MONOPHONE, MLP_SIZE, SHORTALIGN, "phone score"); */
  pMLP->p_s_active = (short*) malloc(sizeof(short)*MLP_NUM_MONOPHONE);
  pMLP->p_s_cur_mono = (short*) malloc(sizeof(short)*MLP_NUM_MONOPHONE);  
  mlp_reset(gv); 
  mlp_set_param( MLP_uPercentage, gv);
#endif
}

void mlp_init(gmhmm_type *gv)
{
#ifdef MODEL_LEVEL_PRUNE
  MODEL_PRUNE_type * pMLP = (MODEL_PRUNE_type*)gv->pPhoneLevelPrune; 
  pMLP->prevBeam = 0;
#endif
}

/* close phone-level pruning */
void mlp_close(gmhmm_type * gv)
{
#ifdef MODEL_LEVEL_PRUNE
  gv->pPhoneLevelPrune = NULL; 
#endif
}

/* calculate pruning threshold 
   @param gv pointer to decoder
   @param th_default the pruning threshold by default beam-width pruning 
   @param beam_width beam width for pruning
   return pruning threshold
 */
short mlp_threshold(gmhmm_type *gv , long th_default , short beam_width)
{
#ifdef MODEL_LEVEL_PRUNE
  MODEL_PRUNE_type * pMLP = (MODEL_PRUNE_type*)gv->pPhoneLevelPrune; 
  register short max_v = LZERO >> 1, min_v = -LZERO >> 1, i , j = 0, threshold;

  for (i=MLP_NUM_MONOPHONE-1;i>=0;i--){
    if (pMLP->p_s_active[i]){
      max_v = MAX(pMLP->p_s_cur_mono[i], max_v);
      min_v = MIN(pMLP->p_s_cur_mono[i], min_v);
      j++;
    }
  }
  
  if (j==0){
    PRT_ERR(printf("%d", th_default));
    return LONG2SHORT(th_default); 
  }
  threshold = max_v - min_v; 
  threshold = q15_x(pMLP->uPercentage, threshold);
  threshold = min_v + threshold; 

  //  threshold = threshold + (MLP_PRUNE * (1 << 6)) * gv->nbr_dim;
  threshold = threshold + beam_width; 

  //  i = threshold - gv->best_prev_scr; /* the beam width to the best path */
  //pMLP->prevBeam = MIN(pMLP->prevBeam, i); 
  //threshold = gv->best_prev_scr + pMLP->prevBeam; 

  return threshold; 
#else
  return 0;
#endif
}

#ifdef _MONOPHONE_NET
gmhmm_type* rs_get_monophone_search_space(gmhmm_type * gvv){
  if (gvv->pRapidSearch){
    return (gmhmm_type*) gvv->pRapidSearch;
  }else return NULL;
}
#endif

/* Confidence measure functions */
#ifdef USE_CONFIDENCE

TIESRENGINECOREAPI_API void cm_freeze(ConfType* pconf)
{
   if( pconf )
      pconf->bNeedUpdate = FALSE; 
}

short * cm_antiscr(ConfType* pconf, ushort frm_cnt)
{
#ifdef USE_CONFIDENCE
    if (frm_cnt >= CM_MAX_FRM) return (short*) NULL; 
  return  & (pconf->cm_anti_scr[frm_cnt]); 
#else
  return NULL;
#endif
}

TIESRENGINECOREAPI_API TIesrEngineStatusType cm_open( ConfType* pconf,
        gmhmm_type * gv )
{
#ifdef USE_CONFIDENCE
  short i; 
  pconf->bCalced = FALSE ; 

  /* pconf->mem_count = 0 ;  */
  /* pconf->cm_anti_scr= (short*)mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*                               CM_MAX_FRM, CM_SIZE, SHORTALIGN, "anti score"); */
  pconf->cm_anti_scr= (short*)malloc(CM_MAX_FRM*sizeof(short));
  if (pconf->cm_anti_scr == NULL) return eTIesrEngineCMMemorySize;

  /* pconf->answers = (ushort*)mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*   			 CM_MAX_WORD, CM_SIZE, SHORTALIGN, "answers"); */
  pconf->answers = (ushort*)malloc(CM_MAX_WORD*sizeof(ushort));

  if (pconf->answers == NULL) return eTIesrEngineCMMemorySize;

  /* pconf->uStp = (ushort*) mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*   			    CM_MAX_WORD, CM_SIZE, SHORTALIGN, "stop at frame"); */
  pconf->uStp = (ushort*) malloc(CM_MAX_WORD*sizeof(ushort));

  if (pconf->uStp == NULL) return eTIesrEngineCMMemorySize;

  /* pconf->cm_scr = ( short *) mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*   		    CM_SCRS, CM_SIZE, SHORTALIGN, "stop at frame"); */
  pconf->cm_scr = ( short *) malloc( CM_SCRS * sizeof(short));

  if (pconf->cm_scr == NULL) return eTIesrEngineCMMemorySize;

  /* pconf->lgBeta =( short *) mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*   		    CM_NBR_BETA, CM_SIZE, SHORTALIGN, "stop at frame"); */
  pconf->lgBeta =( short *) malloc(CM_NBR_BETA*sizeof(short));

  if (pconf->lgBeta == NULL) return eTIesrEngineCMMemorySize;

  /* pconf->acc_dif2_beta = (long*)mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*   				  CM_NBR_BETA<<LONGSHIFT, CM_SIZE, LONGALIGN, "stop at frame"); */
  pconf->acc_dif2_beta = (long*)malloc(CM_NBR_BETA*sizeof(long));

  if (pconf->acc_dif2_beta == NULL) return eTIesrEngineCMMemorySize;

  for (i=CM_NBR_BETA-1;i>=0;i--){
    pconf->lgBeta[i] = CM_LOGIST_PARA[i];
    pconf->acc_dif2_beta[i] = 0;
  }

  gv->cm_score = 0;

#ifdef  USE_CONF_DRV_ADP
  /* pconf->sCH = ( short *)mem_alloc( pconf->base_mem, &pconf->mem_count,  */
  /*   		  N_FILTER26, CM_SIZE, SHORTALIGN, "channel"); */
  pconf->sCH = ( short *)malloc(N_FILTER26*sizeof(short));

  if (pconf->sCH == NULL) return eTIesrEngineCMMemorySize;

  /* pconf->lAccum = (long*)mem_alloc(pconf->base_mem, &pconf->mem_count,  */
  /*   		    (2*N_FILTER26)<<LONGSHIFT, CM_SIZE, LONGALIGN, "channel accum");  */
  pconf->lAccum = (long*)malloc((2*N_FILTER26)*sizeof(long));

  if (pconf->lAccum == NULL) return eTIesrEngineCMMemorySize;

#ifdef USE_SVA
  /* backup the SVA parameters */
  /* pconf->sLogVarRho = ( short *)mem_alloc(pconf->base_mem, &pconf->mem_count, */
  /*   			gv->nbr_dim , CM_SIZE, SHORTALIGN, "SVA rho");  */
  pconf->sLogVarRho = ( short *)malloc(gv->nbr_dim *sizeof(short)); 
  if (pconf->sLogVarRho == NULL) return eTIesrEngineCMMemorySize;

  /* backup the SVA fisher IM parameters */
  /* pconf->lVarFisherIM = (long*)mem_alloc(pconf->base_mem, &pconf->mem_count,  */
  /*   			  gv->nbr_dim << LONGSHIFT, CM_SIZE, LONGALIGN, "SVA fisher");  */
  pconf->lVarFisherIM = (long*)malloc(gv->nbr_dim*sizeof(long));                                       

  if (pconf->lVarFisherIM == NULL) return eTIesrEngineCMMemorySize;

#endif      

#endif

#endif
  return eTIesrEngineSuccess;
}

/* @param th_adp, the threshold for unsupervised adaptation 
   @param th_nbest, the threshold of number of N-bests to declare low 
		     confidence of an uttearnce 
*/
TIESRENGINECOREAPI_API void cm_set_param( short th_adp,
		   short th_nbest,
		   gmhmm_type * gv )
{
  ConfType * pconf = (ConfType*)gv->pConf; 
#ifdef USE_CONF_DRV_ADP
  pconf->adp_th  = th_adp; 
#endif
  pconf->th_nbest = th_nbest ; 
}

TIESRENGINECOREAPI_API void cm_backup_adp( gmhmm_type * gv)
{ 
#ifdef USE_CONF_DRV_ADP
  ConfType * pconf = (ConfType*)gv->pConf; 
  register short i; 
  
  for (i=N_FILTER26-1; i>=0;i--) {
    pconf->sCH[i] = gv->log_H[i]; 
    pconf->lAccum[2*i] = gv->accum[2*i]; 
    pconf->lAccum[2*i+1] = gv->accum[2*i + 1]; 
#ifdef USE_SVA
    pconf->sLogVarRho[i] = gv->log_var_rho[i]; 
    pconf->lVarFisherIM[i] = gv->var_fisher_IM[i]; 
#endif
  }
#ifdef USE_SBC
  if (gv->comp_type & SBC)
    sbc_backup(gv->sbcState);
#endif
#endif
}

TIESRENGINECOREAPI_API void cm_restore_adp(gmhmm_type * gv)
{
#ifdef USE_CONF_DRV_ADP
  ConfType * pconf = (ConfType*)gv->pConf; 
  register short i; 

#ifdef DEV_CONFIDENCE
  printf("SKIP ADAPTATION ");
#endif

  for (i=N_FILTER26-1; i>=0;i--){
    gv->log_H[i] = pconf->sCH[i];
    gv->accum[2*i] = pconf->lAccum[2*i] ;
    gv->accum[2*i + 1] = pconf->lAccum[2*i+1];
#ifdef USE_SVA
    gv->log_var_rho[i] = pconf->sLogVarRho[i] ;
    gv->var_fisher_IM[i] = pconf->lVarFisherIM[i] ;
#endif
  }
#ifdef USE_SBC
  if (gv->comp_type & SBC)
    sbc_restore(gv->sbcState);
#endif
#endif
}


/*-----------------------------------------------------------------------------
 cm_load

 Load statistical and update parameters for the confidence measure if the
 JAC state file pointer exists. Otherwise initialize the parameters to default
 values.

 Arguments:
 fp: File pointer to open JAC state file or NULL
 gv: Pointer to ASR instance structure

--------------------------------- */

TIESRENGINECOREAPI_API void cm_load(FILE* fp, gmhmm_type *gv)
{
#ifdef USE_CONFIDENCE
  ConfType * pconf = (ConfType*)gv->pConf; 
  short i; 

  if (fp){
    my_fread(&pconf->H0.mean, sizeof(short), 1, fp);
    my_fread(&pconf->H0.inv, sizeof(short), 1, fp);
    my_fread(&pconf->H0.gconst, sizeof(short), 1, fp);
    my_fread(&pconf->H1.mean, sizeof(short), 1, fp);
    my_fread(&pconf->H1.inv, sizeof(short), 1, fp);
    my_fread(&pconf->H1.gconst, sizeof(short), 1, fp);
    my_fread(&pconf->NSNR, sizeof(short), 1, fp);
    my_fread(&pconf->lgBeta[0], sizeof(short), CM_NBR_BETA, fp); 
    my_fread(&pconf->acc_dif2_beta[0], sizeof(long), CM_NBR_BETA, fp); 

  }else{
  
    pconf->H0.mean = CM_UV_ACC[0]; 
    pconf->H0.inv = CM_UV_ACC[1];
    pconf->H0.gconst = CM_UV_ACC[2]; 
    
    pconf->H1.mean = CM_UV_REJ[0]; 
    pconf->H1.inv = CM_UV_REJ[1];
    pconf->H1.gconst = CM_UV_REJ[2]; 

    for (i=CM_NBR_BETA-1;i>=0;i--){
      pconf->lgBeta[i] = CM_LOGIST_PARA[i];
      pconf->acc_dif2_beta[i] = 0; 
    }
  }
#endif
}

TIESRENGINECOREAPI_API void cm_save(FILE *fp,
	     gmhmm_type* gv)
{
#ifdef USE_CONFIDENCE
  ConfType * pconf = (ConfType*)gv->pConf; 

  if (fp){
    my_fwrite(&pconf->H0.mean, sizeof(short), 1, fp);
    my_fwrite(&pconf->H0.inv, sizeof(short), 1, fp);
    my_fwrite(&pconf->H0.gconst, sizeof(short), 1, fp);
    my_fwrite(&pconf->H1.mean, sizeof(short), 1, fp);
    my_fwrite(&pconf->H1.inv, sizeof(short), 1, fp);
    my_fwrite(&pconf->H1.gconst, sizeof(short), 1, fp);
    my_fwrite(&pconf->NSNR, sizeof(short), 1, fp);
    my_fwrite(&pconf->lgBeta[0], sizeof(short), CM_NBR_BETA, fp); 
    my_fwrite(&pconf->acc_dif2_beta[0], sizeof(long), CM_NBR_BETA, fp); 
  }

#endif
}

void cm_init( gmhmm_type * gv )
{
#ifdef USE_CONFIDENCE
  short i;
  ConfType * pconf = (ConfType*)gv->pConf; 
  pconf->bCalced = FALSE; 

  pconf->bNeedUpdate = TRUE; 
  for (i=CM_MAX_FRM-1;i>=0;i--)
    pconf->cm_anti_scr[i] = LZERO; 

  pconf->nS = 0;

  pconf->cm_bk_scr = 0; 
  pconf->cm_fg_scr = 0;

  pconf->speech_power = 0; 
  pconf->noise_power = 0;
  pconf->noise_power_sqr = 0;
  pconf->nSpeech = 0;
  pconf->nNoise = 0;

  gv->cm_score = 0; 
#endif
}

TIESRENGINECOREAPI_API void cm_close( gmhmm_type* gv )
{
}

TIESRENGINECOREAPI_API void cm_acc_seg_ratio_to_antimodel(gmhmm_type *gv,
        ushort hmm_code, short stt,
        short stp, long scaling)
{
#ifdef USE_CONFIDENCE
  ushort crt_vec; 
  short i, j, seg_len = stp - stt; 
  ConfType * pConf = (ConfType*) gv->pConf; 

#ifdef BIT8FEAT
   short mfcc_feature[ MAX_DIM ];
   int feat_size = gv->n_mfcc;
#else
   short *mfcc_feature;
   int feat_size = gv->nbr_dim;
#endif

  /* accumulate speech and noise power */
  for (j=0, crt_vec = stt * feat_size; j<seg_len; j++, crt_vec += feat_size) {
#ifdef BIT8FEAT
    vector_unpacking(gv->mem_feature + crt_vec, mfcc_feature, gv->scale_feat, gv->n_mfcc); 
#else
    mfcc_feature = (short *)gv->mem_feature + crt_vec;
#endif

    /* Q6 */
    i = (mfcc_feature[0] >> 5); /* Q11 -> Q6 */
    if (hmm_code == 0) {
      pConf->noise_power += i; 
      pConf->noise_power_sqr += (i * i);/* Q12 */
    }
    else {
      pConf->speech_power += i; 
      if (pConf->speech_power < 0){
	//	printf("find");
      }
    }
  }
  if (hmm_code == 0) pConf->nNoise += seg_len; 
  else pConf->nSpeech += seg_len; 

  if (hmm_code == 0) return; 
  if (stp >= CM_MAX_FRM) return; 

  for (i=stt;i<stp;i++){
    pConf->cm_bk_scr += pConf->cm_anti_scr[i];
    pConf->nS++;
  }

  pConf->cm_fg_scr += scaling; 

#ifdef DBG_CONF
  printf("%d -> %d : fg = %d, bg = %d\n", stt, stp, pConf->cm_fg_scr, pConf->cm_bk_scr);
#endif

#endif
}


/* return Q6 */
static short cm_logistic(short *x, gmhmm_type * gv)
{
   long ltmp = 0;

  ConfType * pConf = (ConfType*) gv->pConf; 
  long ltmp2; 
  short lsum, stmp;
  ushort Q15 = 32767, utmp; 

  for (lsum=1;lsum<=CM_NBR_BETA-1;lsum++){
    ltmp2 = pConf->lgBeta[lsum] * x[lsum-1]; /* Q12 */
    ltmp -= LONG_RIGHT_SHIFT_N(ltmp2 , 6); /* Q6 */
  }
  ltmp -= pConf->lgBeta[0]  ; 

  /* to get Q6 output, 
     15 - l + y = 6
     y = l - 9
     where l = lsum, y = stmp
  */
  utmp = hlr_expn_q(LONG2SHORT(ltmp), 6, &lsum); /* Q lsum */
  ltmp = (long) utmp + (1 << lsum); 
  stmp = lsum - 9; 
  if (stmp > 0)
    ltmp = Q15 / (ltmp >> stmp); /* to get Q6 output */
  else 
    ltmp = Q15 / (ltmp << (-stmp)); 


  return LONG2SHORT(ltmp);
}




/* online updating of parameters for hypothesis testing */
static void cm_update_hypo(short *feat, CM_hypo *hp)
{
  long mu, fea ; 

#ifdef CM_UPDATE_MEAN
  mu = hp->mean; 
  fea = (*feat); 
  fea <<= 5; 
  //  mu = q15_x(u098, mu) + q15_x(u002, fea);
  mu = mu*127 + fea; 
  mu >>= 7;

  hp->mean = LONG2SHORT(mu); 
#endif

#ifdef CM_UPDATE_VAR
  /* update variance */
  fea = (fea - mu);
  fea *= fea; 
  fea >>= 13; /* Q9 */
  
  mu = (1<<18) / hp->inv; /* Q9 */
  mu *= 255;

  fea += mu; /* Q9 */
  fea >>= 8; /* normalize */

  fea = (1<<18)/fea; 
  hp->inv = LONG2SHORT(fea); /* Q9 */

  hp->gconst = 
    gauss_det_const( &hp->inv, 2, NULL, 1);
#endif  

}




/* @param feat Q6 score
   return log-likelihood ratio between H0 and H1 hypothesis, Q6 */
static short cm_hypo_ratio(short *feat, CM_hypo *h0, CM_hypo *h1)
{
  short scr_h0, scr_h1, gconst, *mu, *invvar;
  short fea, *pfea;
  long diff;
  
  fea = *feat; 
  fea <<= 5; /* Q11 */
  pfea = &fea; 
  gconst = h0->gconst; 
  mu = &h0->mean; 
  invvar = &h0->inv; 
  scr_h0 = rj_dist(pfea, mu, invvar, gconst, 1);

  fea = *feat; 
  fea <<= 5; /* Q11 */
  pfea = &fea; 
  gconst = h1->gconst; 
  mu = &h1->mean; 
  invvar = &h1->inv; 
  scr_h1 = rj_dist(pfea, mu, invvar, gconst, 1);

  diff = scr_h0 - scr_h1; 

  return LONG2SHORT(diff);
}



/* calculate distance of
   p(H0|X)*(log(p(X|H0) - log(p(X|H1))
*/
static short cm_calc_proddiff(ConfType* pConf){
  long ltmp;

  ltmp = pConf->cm_scr[3] * pConf->cm_scr[2]; /* Q12 */

  return LONG2SHORT(ltmp >> 6);
}




/* return the confidence score due to N-best numbers. 
   works as a bias to the confidence score reported from else */
static short cm_score_nbest(gmhmm_type * gv)
{
  NB_words * nbest; 
  ConfType * pconf = (ConfType*) gv->pConf; 
  
  if (gv->bUseNBest){
    nbest = &gv->nbest; 
    if (nbest->n_bests >= pconf->th_nbest)
      return CM_NBEST_LOW_CONF;
    //    else if (nbest->n_bests >= (pconf->th_nbest >> 1))
    //return (CM_NBEST_LOW_CONF >> 1); 
    else return 0;
  }
  return 0; 
}




static void cm_calc_confidence( gmhmm_type * gv)
{
  ConfType * pConf = (ConfType*) gv->pConf; 
  long ltmp; 
  long sAve, nAve; 
  short lsum ;

#ifdef DBG_CONF
  printf("conf = ");
  for (ltmp = 0; ltmp < pConf->nSpeech + pConf->nNoise; ltmp ++)
    printf("%d ", pConf->cm_anti_scr[(short)ltmp]);
  printf("\n");
#endif

  ltmp = pConf->cm_fg_scr - pConf->cm_bk_scr;
  if (pConf->nS == 0) ltmp = LZERO; 
  else ltmp /= pConf->nS; 
  lsum = LONG2SHORT(ltmp);

  pConf->cm_scr[0] = lsum;


  /* calculate difference between the averaged log energies of speech and noise segments */
  sAve = (pConf->nSpeech==0)?0:(pConf->speech_power / pConf->nSpeech); 
  nAve = (pConf->nNoise ==0)?0:(pConf->noise_power / pConf->nNoise); 
  lsum = sAve - nAve; 

  /* calcualte non-stationarity */
  ltmp = sAve; /* Q6 */
  ltmp *= ltmp;  /* Q12 */
  ltmp -= ((sAve * nAve) << 1); /* Q12 */
  ltmp += (pConf->nNoise == 0)?0:(pConf->noise_power_sqr / pConf->nNoise);  /* Q12 */
  ltmp -= (lsum * lsum);/* Q12 */
  ltmp = fx_sqrt(ltmp); /* Q6 */
  pConf->NSNR = LONG2SHORT(ltmp); 

  pConf->cm_scr[1] = pConf->NSNR; 

  pConf->cm_scr[2] = cm_hypo_ratio(&pConf->cm_scr[0], &pConf->H0, &pConf->H1);
  pConf->cm_scr[3] = cm_logistic(&pConf->cm_scr[0], gv); 

  pConf->cm_scr[4] = cm_score_nbest(gv);

  gv->cm_score = cm_calc_proddiff(pConf); 
  
  gv->cm_score += pConf->cm_scr[4]; 

}

#endif /* USE_CONFIDENCE */


/* @param iTrue indicator of the acceptance or rejection, Q6 
   @param pCM output of the CM module, Q6
   @param pConf pointer to the CM module
*/
#ifdef TEST_CM_LOGISTIC
static void cm_update_logistic( short iTrue, short pCM, ConfType * pConf )
{
  short * x = pConf->cm_scr; 
  short i, dim = CM_NBR_BETA; 
  short diff, Q6 = 64;
  long first_diff, pp, W;
  ushort u098 = 32440, u002 = 32768 - u098;
  ushort u07 = 22938, u03 = 9830; 

  diff = iTrue - pCM; /* Q6 */
  diff = q15_x(u07, Q6 - pCM) - q15_x(u03, pCM); /* integrate the uncertainty of acceptance */

  pp = pCM*(Q6 - pCM); /* Q12 */
  for (i=dim-1;i>=0;i--) {
    if (i==0) {
      first_diff = Q6 * diff; 
      W = Q6 * Q6; 
    }else{
      first_diff = x[i] * diff; /* Q12 */
      W = x[i] * x[i]; /* Q12 */
    }
    W *= pp;   /* Q24 */
    if (pConf->acc_dif2_beta[i] == 0) pConf->acc_dif2_beta[i] = (W >> 12); 
    else 
      pConf->acc_dif2_beta[i] = q15_x(u098, pConf->acc_dif2_beta[i]) + 
	q15_x(u002, (W >> 12));  /* Q12 */
    if ((first_diff == 0) || (pConf->acc_dif2_beta[i] == 0)) W = 0;
    else W = (first_diff << 12) / pConf->acc_dif2_beta[i]; /* Q12 */
    pConf->lgBeta[i] += LONG2SHORT(W >> 6);
  }
    
}
#endif



#ifdef USE_CONFIDENCE
short cm_score(gmhmm_type *gv)
{
  ConfType * pConf = (ConfType*) (gv->pConf);
  if (pConf->bCalced == FALSE) cm_update_status(gv);
  return gv->cm_score; 
}

/* update confidence measure status */
TIESRENGINECOREAPI_API void cm_update_status(gmhmm_type *gv)
{
#ifdef USE_CONFIDENCE
  ConfType * pConf = (ConfType*) (gv->pConf);

  if (pConf == NULL) return ;

  if (pConf->bCalced == FALSE){/* not yet calculated */
    cm_calc_confidence(gv); 

#ifdef DEV_CONFIDENCE
    short cm; 
    for (cm=0;cm<CM_SCRS;cm++)
      printf("CM[%d] = %d ", cm, pConf->cm_scr[cm]); 
    printf("CM = %d ", gv->cm_score); 
#endif
    cm_update_hypo(&pConf->cm_scr[0], (pConf->cm_scr[2] >= 0)?&pConf->H0:&pConf->H1);

#ifdef TEST_CM_LOGISTIC
    cm_update_logistic( (pConf->cm_scr[1] >=0)?64:0, 
			pConf->cm_scr[3], pConf);
#endif

    pConf->bCalced = TRUE; 

  }
#endif
}

#endif /* USE_CONFIDENCE */



#ifdef USE_AVL_TREE

void 
avl_MakeEmpty( TreeSpaceType * pTree)
{
  if( pTree != NULL )
    {
      pTree->mem_count = 0; 
    }
}

Position
avl_Find( ElementType X, AvlTree T , TreeSpaceType * pTree)
{
  if( T == NULL )
    return NULL;
  if( X < T->Element )
    return avl_Find( X, (T->Left < 0)?NULL:(AvlTree) ((short*)pTree + T->Left), pTree);
  else
    if( X > T->Element )
      return avl_Find( X, (T->Right < 0)?NULL:(AvlTree)((short*)pTree + T->Right), pTree);
    else
      return T;
}

Position
avl_FindMin( AvlTree T , TreeSpaceType* pTree)
{
  if( T == NULL )
    return NULL;
  else
    if( T->Left == -1 )
      return T;
    else
      return avl_FindMin( (AvlTree) ((short*)pTree + T->Left), pTree);
}

Position
avl_FindMax( AvlTree T , TreeSpaceType * pTree)
{
  if( T != NULL )
    while( T->Right != -1 )
      T = (AvlTree) ((short*)pTree + T->Right);
  
  return T;
}

/* START: fig4_36.txt */
static short
avl_Height( Position P )
{
  if( P == NULL )
    return -1;
  else
    return P->Height;
}
/* END */

/* This function can be called only if K2 has a left child */
/* Perform a rotate between a node (K2) and its left child */
/* Update heights, then return new root */

static Position
avl_SingleRotateWithLeft( Position K2 , short * pTree)
{
  Position K1;

  K1 = (Position) (pTree + K2->Left);
  K2->Left = K1->Right;
  K1->Right = (short*)K2 - (short*)pTree;
  
  K2->Height = MAX( avl_Height( (AvlTree) (K2->Left + pTree)), avl_Height( (AvlTree) (K2->Right + pTree ) )) + 1;
  K1->Height = MAX( avl_Height( (AvlTree) (K1->Left + pTree)), K2->Height ) + 1;
  
  return K1;  /* New root */
}
/* END */

/* This function can be called only if K1 has a right child */
/* Perform a rotate between a node (K1) and its right child */
/* Update heights, then return new root */

static Position
avl_SingleRotateWithRight( Position K1 , short * pTree)
{
  Position K2;
  
  K2 = (Position) (K1->Right + pTree);
  K1->Right = K2->Left;
  K2->Left = (short*)K1 - pTree;
  
  K1->Height = MAX( avl_Height( (AvlTree) (K1->Left + pTree)), avl_Height( (AvlTree)(K1->Right + pTree)) ) + 1;
  K2->Height = MAX( avl_Height( (AvlTree) (K2->Right + pTree) ), K1->Height ) + 1;
  
  return K2;  /* New root */
}

/* This function can be called only if K3 has a left */
/* child and K3's left child has a right child */
/* Do the left-right double rotation */
/* Update heights, then return new root */

static Position
avl_DoubleRotateWithLeft( Position K3 , short *pTree)
{
  /* Rotate between K1 and K2 */
  K3->Left = (short*)avl_SingleRotateWithRight( (Position)(K3->Left + pTree), pTree ) - pTree;
  
  /* Rotate between K3 and K2 */
  return avl_SingleRotateWithLeft( K3 , pTree);
}
/* END */

/* This function can be called only if K1 has a right */
/* child and K1's right child has a left child */
/* Do the right-left double rotation */
/* Update heights, then return new root */

static Position
avl_DoubleRotateWithRight( Position K1 , short *pTree)
{
  /* Rotate between K3 and K2 */
  K1->Right = (short*) avl_SingleRotateWithLeft( (Position) (K1->Right + pTree), pTree) - pTree;
  
  /* Rotate between K1 and K2 */
  return avl_SingleRotateWithRight( K1 , pTree);
}


/* START: fig4_37.txt */
short 
avl_Insert( ElementType X, ElementType Val, short T_idx , 
	    short elem_size_in_short, TreeSpaceType * pTreeSpace,
	    TIesrEngineStatusType * status)
{
  short * p_short; 
  AvlTree T = (AvlTree)((T_idx == -1) ? NULL: ((short*)pTreeSpace) + T_idx); 

  if( T_idx == -1)
    {
      /* Create and return a one-node tree */
      p_short = mem_alloc( pTreeSpace->base_mem, &pTreeSpace->mem_count, 
			   elem_size_in_short, TREE_SPACE_SIZE, SHORTALIGN, "elem");
      T = (AvlTree) p_short; 
      if( T == NULL ){
	*status = eTIesrEngineVALTreeMemorySize; 
	return -1; 
      }
      else
	{
	  T->Element = X; T->Height = 0;
	  T->Left = T->Right = -1;
	  T->Value = Val; 
	  *status = eTIesrEngineSuccess;
	  return pTreeSpace->mem_count - elem_size_in_short;
	}
    }
  else
    if( X < T->Element )
      {
	T->Left = avl_Insert( X, Val, T->Left , elem_size_in_short, pTreeSpace, status);
	if( avl_Height( (T->Left < 0)?NULL:(AvlTree)(T->Left + (short*)pTreeSpace)) 
	    - avl_Height( (T->Right < 0)?NULL:(AvlTree)(T->Right + (short*)pTreeSpace)) == 2 )
	  if( X < ((AvlTree)(T->Left + (short*)pTreeSpace))->Element )
	    T = avl_SingleRotateWithLeft( T , (short*)pTreeSpace);
	  else
	    T = avl_DoubleRotateWithLeft( T , (short*)pTreeSpace);
      }
    else
      if( X > T->Element )
	{
	  T->Right = avl_Insert( X, Val, T->Right,  elem_size_in_short, pTreeSpace, status );
	  if( avl_Height( (T->Right<0)?NULL:(AvlTree)(T->Right + (short*)pTreeSpace)) 
	      - avl_Height( (T->Left<0)?NULL:(AvlTree)(T->Left + (short*)pTreeSpace)) == 2 )
	    if( X > ((AvlTree)(T->Right + (short*)pTreeSpace))->Element )
	      T = avl_SingleRotateWithRight( T , (short*)pTreeSpace);
	    else
	      T = avl_DoubleRotateWithRight( T , (short*)pTreeSpace);
	}
  /* Else X is in the tree already; we'll do nothing */
  
  T->Height = MAX( avl_Height( (T->Left<0)?NULL:(AvlTree)(T->Left + (short*)pTreeSpace)),
		   avl_Height( (T->Right<0)?NULL:(AvlTree)(T->Right + (short*)pTreeSpace)) ) + 1;

  if (T==NULL) return -1; 
  else return (short*)T - (short*)pTreeSpace;
}
/* END */

ElementType
avl_Retrieve( Position P )
{
  return P->Element;
}


#endif
