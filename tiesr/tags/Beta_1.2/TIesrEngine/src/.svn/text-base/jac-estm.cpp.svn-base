/*=======================================================================
 jac-estm.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This source implements JAC model compensation.  Includes improved
 JAC (IJAC) and stochastic bias compensation (SBC).

======================================================================*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif


#include "tiesr_config.h"
#include "tiesrcommonmacros.h"
#include "dist_user.h"
#include "search_user.h"
#include "mfcc_f_user.h"
#include "pack_user.h"
#include "pmc_f_user.h"
#include "obsprob_user.h"
#include "jac-estm_user.h"
#include "jac-estm.h"
#include "sbc_user.h"
#include "noise_sub_user.h"
//#ifdef USE_CONFIDENCE 
#include "confidence_user.h"
//#endif
#ifdef RAPID_JAC
#include "rapidsearch_user.h"
#endif
#include "load_user.h"

//#define DBG_CONF

//extern short test_convergence(short new_like, short last_like, ushort iter);

/*extern unsigned short hlr_expn( short x, short qpt );*/ /* Q15 */

/*
** cumulate per token statistics 
** - assume alpha and beta computed
*/

#ifndef BIT8FEAT
extern void  copy_feature(short feature[], ushort crt_vec, short n_mfcc);
#endif

/*
  IJAC update of channel estimates for noisy speech recognition

  @param jac_hmm pointer to HmmType
  @param pdfa pointer to log likelihood 
  @param ap pointer to alpha in EM estimates
  @param bp pointer to beta in EM estimates
  @param vec short pointer to MFCC feature
  @param log_vec short pointer to log-spectral observation vector
  @param T length of an utterance
  @param accum long pointer to the first- and second-order differentials of auxiliary function
  @param log_H short pointer to the estimated channel distortion in log-spectral domain
  @param log_N short pointer to the estimated noise in log-spectral domain
  @param n_mfcc MFCC dimension
  @param temp_freq short pointer to a memory space allocated by the upper function for temparary usage
  @param gv pointer to gmhmm_type
  @param p_transform pointer to tranformation applied to this model for this segment of feature
  @param s_normalization state normalization factor
 */
#ifdef USE_IJAC
static void ijac_update_t(HmmType *jac_hmm, short *pdfa, const short ap[], const short bp[], 
			  const short vec[], 
			  const short log_vec[], const ushort T, long accum[], const short log_H[], 
			  const short log_N[], const short n_mfcc,  short temp_freq[], gmhmm_type *gv,
			  short * p_transform, short s_normalization)
{
  short k, frame = 0, d, j;
  long indep_k, temp, *accum_d = GET_D_OBJ(accum, gv->n_filter);
  short pdf_i, *pdf, *pmix, *mu, *org_mu, s3, s2;
  ushort gamma_jk;
  short *func_d = temp_freq, Lr;
  short noisy_log[N_FILTER26];
  short *pmu, sp_log_ps[ N_FILTER26 ];        /* log power spectrum */  
  short *pnoisymu;
#ifdef BIT8MEAN 
  short org_mu16[ MAX_DIM ];
  short noisy_mu16[ MAX_DIM ];
#endif
  long  long_diff, longtemp; 
  short diff_s, weight;
  short invvar[ MAX_DIM ], *p_invvar ;
  
  FOR_EMS_STATES(j,jac_hmm, gv->base_tran) {
    
    indep_k = (long) ap[j] + bp[frame] - pdfa[frame] - s_normalization;
    pdf_i = GET_BJ_IDX(jac_hmm,j);
    pdf = GET_MIX(gv,pdf_i);
    FOR_EACH_MIXING(k,pmix,pdf) { 
#ifdef BIT8MEAN 
      mu = GET_MU(gv->base_mu, pmix, n_mfcc);
#else
      mu = GET_MU(gv->base_mu, pmix, gv->nbr_dim);
#endif
      org_mu = ADJ_POINTOR(mu, gv->base_mu,gv->base_mu_orig);
      temp = hlr_gauss_obs_score_f(vec, (int)pdf_i, COMPONENT, &k, gv->nbr_dim, gv) + indep_k;

      Lr = (temp > 0)? 0: LONG2SHORT(temp); /* Q6 */

      if (Lr > gv->jac_component_likelihood ) {
	gamma_jk = hlr_expn(Lr,6); /* Q15 */

#ifdef BIT8MEAN 
	vector_unpacking((ushort *)org_mu, org_mu16, gv->scale_mu,  n_mfcc);
	pmu = org_mu16;
	vector_unpacking((ushort *)mu, noisy_mu16, gv->scale_mu,  n_mfcc);
	pnoisymu = noisy_mu16;
	//	prt_fix("mu 16", noisy_mu16,gv->muScaleP2, gv->nbr_dim);	
	//	prt_fix("vec", (short*)vec, gv->muScaleP2, gv->nbr_dim);	
#else	
	pmu = org_mu;
	pnoisymu = mu;
#endif	  

	cos_transform(pmu, sp_log_ps, gv->muScaleP2, n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */
	log_spectral_compensation(sp_log_ps, NULL, noisy_log, NULL, log_N, log_H, 1, func_d, gv->n_filter);
	/* 	compensated_log_mean_fx(org_mu, log_N, noisy_log, func_d,log_H); */
	FOREACH(d,gv->n_filter) {
	  temp = gamma_jk * func_d[d]; /*Q30*/
	  s3 = LONG_RIGHT_SHIFT_N(temp, 15); /* Q15 */
	  s2 = noisy_log[d] - log_vec[d]; /* Q9 */

	  temp = s3 * s2; /* Q24 */
	  accum[d] += LONG_RIGHT_SHIFT_N(temp, 12); /* Q12 */

	  temp = s2 * func_d[d];   /* Q24 */
	  temp = LONG_RIGHT_SHIFT_N(temp, 9); /*Q15*/
	  s2 += (short) LONG_RIGHT_SHIFT_N((long)func_d[d] - temp, 6);   /*Q9*/
	  temp = s2 * s3; /* Q24 */
	  accum_d[d] += LONG_RIGHT_SHIFT_N(temp, 10); /* Q14 */
	}
	/*
	** accumulations of Fisher information matrix and   
	** score vector for sequential variance adaptation. 
	*/
#ifdef USE_SVA
	{  

#ifdef BIT8VAR
	   p_invvar = GET_SIGMA2(gv->base_var, pmix, n_mfcc);
	   vector_unpacking((unsigned short *)p_invvar, invvar, gv->scale_var, n_mfcc);
#else
	   p_invvar = GET_SIGMA2(gv->base_var, pmix, gv->nbr_dim);
	   for( d=0; d < gv->nbr_dim; d++ )
	      invvar[d] = *p_invvar++;
#endif

	   for ( d = 0 ; d < gv->nbr_dim ; d++ )  {
	    /* (1/sigma^2)(Q9) * gamma(Q15) to weight in Q9 */
	    longtemp = invvar[d] * gamma_jk; /* Q9 x Q15 = Q24 */
	    longtemp = LONG_RIGHT_SHIFT_N(longtemp, 15); /* Q9 */
	    weight = (short)longtemp;
	    long_diff = vec[d] - pnoisymu[d]; /* Q11 */
	    long_diff = MAX(long_diff, -32768); /* clip */ 
	    long_diff = MIN(long_diff, 32767); 
	    diff_s = (short) long_diff;
	    long_diff = diff_s * diff_s; /* Q 22 */ 
	    diff_s = (short)LONG_RIGHT_SHIFT_N(long_diff, 15);  /* Q7 */

	    longtemp = weight * diff_s; /* Q9 x Q7 = Q16 */
	    longtemp = LONG_RIGHT_SHIFT_N(longtemp, 7); /* 16 - 7 = Q9 */

	    gv->var_fisher_IM_per_utter[d] += longtemp; /* Q9  */
	  }
	}

#ifdef USE_SBC
	if (gv->comp_type & SBC)
	  sbc_acc_cep_bias(gv->nbr_dim, T, pnoisymu, invvar, vec, gamma_jk, 
			   p_transform, gv->sbcState);
#endif
#endif

      }
    }
    frame += T;
  }

}
#else
/*--------------------------------*/
static void update_t(HmmType *jac_hmm, short *pdfa, const short ap[], const short bp[], const short vec[],
	      const ushort T, long accum[], const short log_H[], const short log_N[], const short n_mfcc, short tmp_freq[], gmhmm_type *gv)
{
  short k, frame = 0, d, j;
  long indep_k, temp, *accum_d = GET_D_OBJ(accum, gv->n_filter);
  short pdf_i, *pdf, *pmix, *mu, *org_mu;
  ushort gamma_jk;
  short noisy_log[N_FILTER26], *func_d = tmp_freq, Lr;
  short *pmu, sp_log_ps[ N_FILTER26 ];        /* log power spectrum */  
  short *pnoisymu;
#ifdef BIT8MEAN 
  short org_mu16[ MAX_DIM ];
  short noisy_mu16[ MAX_DIM ];
#endif

  FOR_EMS_STATES(j,jac_hmm, gv->base_tran) {
    indep_k = (long) ap[j] + bp[frame] - pdfa[frame];
    pdf_i = GET_BJ_IDX(jac_hmm,j);
    pdf = GET_MIX(gv,pdf_i);
    FOR_EACH_MIXING(k,pmix,pdf) { 
#ifdef BIT8MEAN 
      mu = GET_MU(gv->base_mu,pmix, n_mfcc);
#else
      mu = GET_MU(gv->base_mu, pmix, gv->nbr_dim);
#endif
      org_mu = ADJ_POINTOR(mu, gv->base_mu,gv->base_mu_orig);
      temp = hlr_gauss_obs_score_f(vec, (int)pdf_i, COMPONENT, &k, gv->nbr_dim, gv) + indep_k;
      Lr = (temp > 0)? 0: LONG2SHORT(temp); /* Q6 */
      if (Lr > gv->jac_component_likelihood) {
	gamma_jk = hlr_expn(Lr,6); /* Q15 */
#ifdef BIT8MEAN 
	vector_unpacking((ushort *)org_mu, org_mu16, gv->scale_mu,  n_mfcc);
	pmu = org_mu16;
	vector_unpacking((ushort *)mu, noisy_mu16, gv->scale_mu,  n_mfcc);
	pnoisymu = noisy_mu16;
	//	prt_fix("mu 16", noisy_mu16,gv->muScaleP2, gv->nbr_dim);	
	//	prt_fix("vec", (short*)vec, gv->muScaleP2, gv->nbr_dim);	
#else	
	pmu = org_mu;
	pnoisymu = mu;
#endif
	cos_transform(pmu, sp_log_ps, gv->muScaleP2, n_mfcc, gv->n_filter, gv->cosxfm ); /* cepstrum to log mel power spectrum, Q 11 to Q 9 */
	log_spectral_compensation(sp_log_ps, NULL, noisy_log, NULL, log_N, log_H, 1, func_d, gv->n_filter);
	/* 	compensated_log_mean_fx(org_mu, log_N, noisy_log, func_d,log_H); */
	FOREACH(d,gv->n_filter) {
	  temp = gamma_jk * (long)noisy_log[d]; /* Q24 */
	  /* temp += (1<<13); */
	  accum  [d] += temp >> 14; /* Q10 */
	  temp = gamma_jk * (long) func_d[d];   /* Q30 */
	  /* temp += (1<<13); */
	  accum_d[d] += temp >> 14;            /* Q16 */
	}
	/*
	** accumulations of Fisher information matrix and   
	** score vector for sequential variance adaptation. 
	*/
#ifdef USE_SVA
	{  
	  long  long_diff, longtemp; 
	  short diff_s, weight;
	  short invvar[ MAX_DIM ];
	  short *p_invvar;

#ifdef BIT8VAR
	  p_invvar = GET_SIGMA2(gv->base_var, pmix, n_mfcc);
	  vector_unpacking((unsigned short *)p_invvar, invvar, gv->scale_var, n_mfcc);
#else
	  p_invvar = GET_SIGMA2(gv->base_var, pmix, gv->nbr_dim);
	  for( d=0; d < gv->nbr_dim; d++ )
	     invvar[d] = *p_invvar++;
#endif

	  for ( d = 0 ; d < gv->nbr_dim ; d++ )  {
	    /* (1/sigma^2)(Q9) * gamma(Q15) to weight in Q9 */
	    longtemp = invvar[d] * gamma_jk; /* Q9 x Q15 = Q24 */
	    longtemp = LONG_RIGHT_SHIFT_N(longtemp, 15); /* Q9 */
	    weight = (short)longtemp;
	    long_diff = vec[d] - pnoisymu[d]; /* Q11 */
	    long_diff = MAX(long_diff, -32768); /* clip */ 
	    long_diff = MIN(long_diff, 32767); 
	    diff_s = (short) long_diff;
	    long_diff = diff_s * diff_s; /* Q 22 */ 
	    diff_s = (short)LONG_RIGHT_SHIFT_N(long_diff, 15);  /* Q7 */

	    longtemp = weight * diff_s; /* Q9 x Q7 = Q16 */
	    longtemp = LONG_RIGHT_SHIFT_N(longtemp, 7); /* 16 - 7 = Q9 */

	    gv->var_fisher_IM_per_utter[d] += longtemp; /* Q9  */
	  }
	}
#endif
      }
    }
    frame += T;
  }
}
#endif

/*--------------------------------*/
/* update sufficient statistics 
 */
static Boolean update_statistics(ushort hmm_code, HmmType *jac_hmm,
				 short *pdfa, 
				 short *bp, ushort token, int T, 
				 short alpha[], 
				 short c_t_beta[], long accum[], 
				 short log_H[], 
				 short log_N[], 
				 short n_mfcc, gmhmm_type *gv)
{
  short tmp_freq[N_FILTER26];
  ushort t, seg;
  short log_e[N_FILTER26]; 
  short *alfa_t, *beta_t;
  short mfcc_feature[ MAX_DIM ]; /* share mfcc vector */
  short *p_fea; 
  Boolean bSucc = TRUE; 

#ifdef USE_SBC
  short obs_enh[MAX_DIM];
#endif

#if defined(USE_SBC) || defined(USE_IJAC)
  short sbc_bias[MAX_DIM]; 
#endif


  for (seg = token * n_mfcc, t=0; t<T; t++, seg += n_mfcc) {  
    alfa_t = EM_alpha_t(pdfa, alpha, t, T, jac_hmm, c_t_beta,gv);
    beta_t = bp + t;
#ifdef BIT8FEAT
    vector_unpacking(gv->mem_feature + seg, mfcc_feature, gv->scale_feat, n_mfcc);          
#else
    copy_feature(mfcc_feature, seg, n_mfcc);
#endif

    p_fea = mfcc_feature; 
#ifdef USE_SBC
    if (gv->comp_type & SBC){
      p_fea = obs_enh; 
      sbc_re_enhance( gv->sbcState, mfcc_feature, 
		      p_fea, hmm_code, gv->nbr_dim, sbc_bias); 
    }
#endif

#ifdef USE_IJAC
    cos_transform( p_fea, log_e, gv->muScaleP2, n_mfcc, gv->n_filter, gv->cosxfm);     /* Q 11 -> Q 9 */
    ijac_update_t(jac_hmm, pdfa + t, alfa_t, beta_t, p_fea, log_e, 
		  T, accum,log_H,log_N, n_mfcc, tmp_freq, gv, sbc_bias, 0); 
#else
    update_t(jac_hmm, pdfa + t, alfa_t, beta_t, p_fea, T, accum,log_H,log_N, n_mfcc, tmp_freq, gv); 
/* remove DFT of test frame (label: {expand:newton}.
** this can be raplced by reusing log spectrum in mfcc: 
*/
    cos_transform( p_fea , log_e, gv->muScaleP2, n_mfcc, gv->n_filter, gv->cosxfm);     /* Q 11 -> Q 9 */
    int d;
    for (d=0; d< gv->n_filter; d++) accum[d] -= log_e[d]<<1; /* Q10 */
#endif
  }

  return bSucc; 
}

/*--------------------------------*/
/* GMHMM_SI_API */ 
TIesrEngineJACStatusType JAC_clear( gmhmm_type *gv, 
				    const char* aModelDir,
				    const char* aTreeFile )
{
   int d;

#ifdef USE_SBC   
   TIesrEngineHLRStatusType hStatus;
#endif

   /* clear log spectrum of channel estimate */
   if (gv->comp_type& JAC) {
     FOREACH(d, gv->n_filter) gv->log_H[d] = 0;

     /* clear JAC channel estimate global correction accumulators */
     FOREACH(d, gv->n_filter*2) gv->accum[d] = 0; 
   }
   /* This forces the global correction accumulators to initialize
      with the next file channel estimates */
   gv->smooth_coef = ONE_Q15; 

#ifdef USE_SVA
   /* Clear SVA mfcc feature variance accumulators and rho estimate */
   if (gv->comp_type & SVA) {
     for (d = 0; d < gv->nbr_dim; d++) 
       {
	 gv->log_var_rho [ d ] = 0;
	 gv->var_fisher_IM[ d ] = 0;
	 gv->var_fisher_IM_per_utter[ d ] = 0;
       }
   }
#endif

#ifdef USE_SBC
   /* Try to initialize SBC structure */
   if (gv->comp_type & SBC) 
     hStatus = sbc_init_tree( gv->sbcState, aModelDir,
			      aTreeFile, FALSE,
			      gv->nbr_dim,
			      gv->trans->n_hmm_set , TRUE);
   else
   {

      if( gv->sbcState->pHMM2phone != NULL )
      {
	 free( gv->sbcState->pHMM2phone);
	 gv->sbcState->pHMM2phone = NULL;
      }

      hStatus = sbc_load_hmm2phone(aModelDir, gv->trans->n_hmm_set, 
				   gv->sbcState, TRUE); 

   }

   if( hStatus != eTIesrEngineHLRSuccess )
     return eTIesrEngineJACSBCInitFail;
        
#endif

   return eTIesrEngineJACSuccess;
}


/*----------------------------------------------------------------
  MP_JAC_load

  Read bias from a file, unless !exist, in which case bias is set to
  zero.  The file contains the channel and SVA information derived
  from past utterances.
  ----------------------------------------------------------------*/
static
TIesrEngineJACStatusType MP_JAC_load( const char *bias_file, gmhmm_type *gv,
				      const char *pNetDir, const char* p_tree_file,
				      Boolean bTriphone)
{
   ushort d;
   unsigned short numFilters;
   unsigned short featDim;
   size_t numRead;
   TIesrEngineJACStatusType eStatus = eTIesrEngineJACSuccess; 
   TIesrEngineJACStatusType eStatusClear;

#ifdef USE_SBC
   TIesrEngineHLRStatusType hStatus;
#endif


#ifdef USE_WSVA
   NssType* pNss = (NssType*) gv->pNss; 
#endif
  
   FILE *pf = fopen(bias_file,"rb");
  
   if( pf )
   {
      /* A channel estimate file exists, read all parameters, 
       checking to ensure they are appropriate. */

      /* Check number of filters in channel estimate file */
      numRead = fread( &numFilters, sizeof(short), 1, pf );
      if( numRead != 1 || numFilters != gv->n_filter )
	{
	  eStatus = eTIesrEngineJACLoadFail; 
	  goto Failure;
	}

      /* Check number of features in channel estimate file */
      numRead = fread( &featDim, sizeof(short), 1, pf );
      if( numRead != 1 || featDim != gv->nbr_dim )
	{
	eStatus = eTIesrEngineJACLoadFail;
	goto Failure;
	}

      /* Read the channel log spectral estimate */
      numRead = fread( gv->log_H, sizeof(short), gv->n_filter, pf );
      if( numRead != gv->n_filter )
	{
	  eStatus = eTIesrEngineJACLoadFail;
	  goto Failure;
	}
      
      /* Read the channel estimate global accumulators */
      numRead = fread( gv->accum, sizeof(long), 2*gv->n_filter, pf );
      if( numRead != (size_t)2*gv->n_filter )
	{
	  eStatus = eTIesrEngineJACLoadFail;
	  goto Failure;
	}
      /* A running global estimate exists, so continue to 
       update according to the accumulated estimates */
      gv->smooth_coef = gv->jac_smooth_coef; 

#ifdef USE_SVA
      /* Read the SVA log_var_rho parameter */
      numRead = fread( gv->log_var_rho, sizeof(short), gv->nbr_dim, pf );
      if( numRead != (size_t)gv->nbr_dim )
	{
	  eStatus = eTIesrEngineJACLoadFail;
	  goto Failure;
	}
      
      /* Read the SVA Fisher IM parameter */
      numRead = fread( gv->var_fisher_IM, sizeof(long), gv->nbr_dim, pf );
      if( numRead != (size_t)gv->nbr_dim )
	{
	  eStatus = eTIesrEngineJACLoadFail;
	  goto Failure;
	}
      
      /* Clear the temporary accumulators */
      for (d = 0; d < gv->nbr_dim; d++) 
	{
	  gv->var_fisher_IM_per_utter[ d ] = 0;
	}
#endif      

#ifdef USE_WSVA
      /* Read the noise level of the previous utterance */
      if (pNss){
	pNss->prevMeanEn = 0;
	numRead = fread(&(pNss->prevMeanEn), sizeof(short), 1, pf);
	if( numRead != 1 )
	  {
	    eStatus = eTIesrEngineJACLoadFail;
	    goto Failure;
	  }
      }
#endif      


      /* Initialization of SBC method */
#ifdef USE_SBC
      if (gv->comp_type & SBC){
	hStatus = sbc_init_tree( gv->sbcState, pNetDir, p_tree_file, FALSE,
				 gv->nbr_dim, 
				 gv->trans->n_hmm_set , bTriphone); 
	
	if( hStatus != eTIesrEngineHLRSuccess )
	  {
	    eStatus = eTIesrEngineJACSBCInitFail;
	    goto Failure;
	  }      
	
	hStatus = sbc_load( gv->sbcState, pf);
      }

      else
      {
	 if( gv->sbcState->pHMM2phone != NULL )
	 {
	    free( gv->sbcState->pHMM2phone);
	    gv->sbcState->pHMM2phone = NULL;
	 }

	 hStatus = sbc_load_hmm2phone(pNetDir, gv->trans->n_hmm_set, 
				     gv->sbcState, bTriphone); 
      }

      if( hStatus != eTIesrEngineHLRSuccess )
	{
	  eStatus = eTIesrEngineJACSBCLoadFail;
	  goto Failure;
	}


#endif

#ifdef USE_ONLINE_REF
      rj_load(gv, pf); 
#endif
      
   }

#ifdef USE_CONFIDENCE
   cm_load(pf, gv);
#endif

   /* Some failure in the channel estimate file or SBC.  Clear out 
      all accumulators and channel estimates, and start afresh */
  Failure:

   if (pf) fclose( pf );
   /* Clear JAC channel estimates, SVA estimates,
      and accumulators */
   eStatusClear = eTIesrEngineJACSuccess;
   /* 
      return the error status will results in Recognizer failure. 
      Instead, reset the recognizer, so comment the error status */
   if (eStatus == eTIesrEngineJACLoadFail) pf = NULL; 
   
   if (eStatus != eTIesrEngineJACSuccess || pf == NULL) 
      eStatusClear = JAC_clear( gv, pNetDir, p_tree_file );

   if( eStatusClear != eTIesrEngineJACSuccess )
      return eStatusClear;
   else{
     switch(eStatus){
     case eTIesrEngineJACLoadFail: 
       return eTIesrEngineJACReset; 
     default:
       return eStatus; 
     }
   }
}

/* GMHMM_SI_API */ 
TIesrEngineJACStatusType JAC_load( const char *bias_file, gmhmm_type *gv,
				   const char *pNetDir, const char* p_tree_file)
{
   TIesrEngineJACStatusType eStatus = eTIesrEngineJACSuccess; 

#ifdef _MONOPHONE_NET
   gmhmm_type * gvv = (gmhmm_type*) gv->pAux; 
   if (gvv)
     eStatus = MP_JAC_load( bias_file, gvv, pNetDir, p_tree_file, FALSE); 
   if (eStatus != eTIesrEngineJACSuccess) return eStatus; 
#endif
   eStatus = MP_JAC_load( bias_file, gv, pNetDir, p_tree_file, TRUE); 
   return eStatus; 
}

/*----------------------------------------------------------------
  JAC_save
  
  Write channel spectral estimate, accumulators, and SVA data
  to a channel estimate file
  ----------------------------------------------------------------*/
/* GMHMM_SI_API */ 
TIesrEngineJACStatusType JAC_save(const char *bias_file, gmhmm_type *gv)
{ 
   size_t numWritten;
#ifdef USE_WSVA
   NssType * pNss = (NssType*) gv->pNss; 
#endif

   FILE *pf = fopen(bias_file,"wb");

   if ( !pf ) 
   {
      PRT_ERR(fprintf(stderr,"fail to open %s\n", bias_file));
      return eTIesrEngineJACSaveFail;
   }

   else
   {
      numWritten = fwrite( &gv->n_filter, sizeof(ushort), 1, pf );
      if( numWritten != 1 )
	 goto Failure;

      numWritten = fwrite( &gv->nbr_dim, sizeof(short), 1, pf );
      if( numWritten != 1 )
	 goto Failure;

      numWritten = fwrite(gv->log_H, sizeof(short), gv->n_filter, pf);
      if( numWritten != gv->n_filter )
	goto Failure;
      
      numWritten = fwrite( gv->accum, sizeof(long), 2*gv->n_filter, pf );
      if( numWritten != (size_t)2*gv->n_filter )
	goto Failure;

#ifdef USE_SVA
      /* Write the SVA log_var_rho parameter */
      numWritten = fwrite( gv->log_var_rho, sizeof(short), gv->nbr_dim, pf );
      if( numWritten != (size_t)gv->nbr_dim )
	goto Failure;
      
      /* Read the SVA Fisher IM parameter */
      numWritten = fwrite( gv->var_fisher_IM, sizeof(long), gv->nbr_dim, pf );
      if( numWritten != (size_t)gv->nbr_dim )
	goto Failure;
#endif      

#ifdef USE_WSVA
      /* Write the noise level of the current utterance, will be used as the noise 
	 level of the previous utterance in the next iteration */
      if (pNss) {
	numWritten = fwrite(&pNss->noiselvl, sizeof(short), 1, pf);
	if( numWritten != 1)
	  goto Failure;
      }
#endif      

#ifdef USE_SBC
      if (gv->comp_type & SBC) sbc_save(gv->sbcState, pf);
#endif

#ifdef USE_ONLINE_REF
      rj_save(gv, pf); 
#endif

#ifdef USE_CONFIDENCE
      cm_save(pf, gv);
#endif

      /* Everything written ok, close file and return success */
      fclose( pf );
      PRT_ERR(printf("Channel saved to %s\n", bias_file));
      return  eTIesrEngineJACSuccess;
   }

  Failure:
   fclose(pf);
   return eTIesrEngineJACSaveFail;
}

/* ------------------------------ for force alignment --------------------------------------  */

/*--------------------------------*/
static void JAC_backup_net(gmhmm_type * gv)
{
  short i; 
  gv->base_net_bk = gv->base_net; 
  gv->trans_bk = gv->trans; 

  for (i=0;i<gv->nbr_ans;i++)
    gv->answer_bk[i] = gv->answer[i]; 
  gv->nbr_ans_bk = gv->nbr_ans; 
}

/*--------------------------------*/
static void JAC_recover_backuped_net(gmhmm_type * gv)
{
  short i; 

  gv->base_net = gv->base_net_bk; 
  gv->trans = gv->trans_bk; 

  gv->nbr_ans = gv->nbr_ans_bk; 
  for (i=0;i<gv->nbr_ans;i++)
    gv->answer[i] = gv->answer_bk[i]; 
}


/* return 0 if fail, 1 if pass */
static short cap_in_check(char *mesg, ushort value, ushort max_mem)
{
#ifdef SHOW_MEMORY
  printf("%3d words (MAX = %3d) %s\n", value, max_mem, mesg); 
#endif
  if (value > max_mem ){
#ifdef SHOW_MEMORY
    fprintf(stderr,"maximum size capacity exceeded at \"%s\"\n", mesg);
    exit(1);
#endif
    return 0;
  }
  else return 1;
}

/*
** make a network for name decoding.
** n is the number of HMMs including 1 SIL (there are two)
** silence uses the last code
*/
static TIesrEngineStatusType JAC_make_net(gmhmm_type *gv, ushort n, ushort *mem_count, 
					  short *mem_base, ushort max_mem)
{
  short cnt /* in number of shorts */,
    i;
  ushort *p_cnt, s, total_words;
  SymType *p_sym, *p_sym0; /* the full network (including entry&Exit states) */

  /* point to new network */
  gv->base_net = mem_base + *mem_count;
  gv->trans = (TransType *) gv->base_net;

  gv->trans->n_sym = n + 1; /* plus 1, for h#  */

  total_words = TOTAL_NET_SIZE(gv->trans->n_sym);
  *mem_count += total_words;
  /*
  ** print total memory usage: 
  */
  if (!cap_in_check("total network memory", *mem_count, max_mem)) return eTIesrEngineNetworkMemorySize;
  
//printf("======%d %d  %d\n", gv->trans->n_hmm,  gv->trans->n_hmm_set, gv->trans->n_set );
  gv->trans->n_hmm = gv->trans_bk->n_hmm; //n;     /* including one h# */
  gv->trans->n_word = 0;    /* no word information needed for name dialing */
  gv->trans->n_set = gv->trans_bk->n_set;//1;
  gv->trans->n_hmm_set =  gv->trans_bk->n_hmm_set; //gv->trans->n_hmm / gv->trans->n_set;

  /* start and atop offset: */
  cnt = 5 +  /* the 5 previous numbers */
        2 +  /* offset pointer to start and stop */
        gv->trans->n_sym; /* -> 10 */
  gv->trans->start = cnt;
  /* entry & exit symbols: */

  cnt += NBR_FIELDS + 1; /* start.n_next */ /* -> 13 */
  gv->trans->stop  = cnt;    /* stop offset  */
  cnt += NBR_FIELDS + 1; /* stop.n_next  */  /* -> 16 */

  for (p_cnt = &(gv->trans->symlist[0]), s = 0; s < gv->trans->n_sym; s++, p_cnt++) {
    *p_cnt = cnt;  /* n_sym offsets */
    cnt += NBR_FIELDS + 1; /* trans.symlist[ i ].n_next; */
  }
  
  /* entry & exit symbols: */
  p_sym0 = p_sym = (SymType *) p_cnt;
  /* output_sym( &trans.start, fp); */
  p_sym->hmm_code = 0; /* non-emitting */
  p_sym->n_next = 1;
  p_sym->next[0] = 0;
  p_sym++;
  /* output_sym( &trans.stop, fp); */
  p_sym->hmm_code = 0; /* non-emitting */
  p_sym->n_next = 1;
  p_sym->next[0] = gv->trans->n_sym - 1;
  p_sym++;

  /* HMM code for entry and exit silences: */
  i = gv->nbr_seg - 1; 
  for (s=0; s<gv->trans->n_sym ; s++) {
    p_sym[s].hmm_code = gv->hmm_code[i--];
    
    p_sym[s].n_next = 1; /* always one next symbol */
    p_sym[s].next[0] = s + 1;
  }
  p_sym[gv->trans->n_sym - 1].n_next = 0;
#ifdef OUTPUT_NET
  //  output_network(base_net); /* write network in vcg format */
  prt_spot_network(gv->base_net);
#endif
  return eTIesrEngineSuccess;
}

static TIesrEngineStatusType process_a_file_1( gmhmm_type *gv, 
					       ushort frm_cnt, ushort start_frame)
{
  short mfcc_feature[ MAX_DIM ];   
  ushort *feat = (ushort *)gv->mem_feature + start_frame * gv->n_mfcc; 
  ushort fm, status;

  for (fm = 0; fm < frm_cnt; fm++) {
#ifdef BIT8FEAT
    vector_unpacking(feat, mfcc_feature, gv->scale_feat, gv->n_mfcc);
#else
    copy_feature(mfcc_feature, fm, n_mfcc);
#endif

    /* search entry */
//printf(" %d  wbt %d\n", fm, gv->word_backtrace);    
    status=search_a_frame(mfcc_feature, 1, fm, gv);
    if (status!=eTIesrEngineSuccess) 
     { PRT_ERR(printf (" BT fails at frame %d !!! \n\n", fm));
       return eTIesrEngineAlignmentFail;
     }

    feat += gv->n_mfcc;
  }
  return eTIesrEngineSuccess;
}

static TIesrEngineStatusType jac_force_alignment(gmhmm_type * gv)
{
  ushort mem_count = 0, total_frm = gv->frm_cnt; //gv->stp[0]; 
  short mem_base[MEM_SIZE_FORCE_ALIGNMENT] ;
  TIesrEngineStatusType status;

  /* backup original network */
  JAC_backup_net(gv);

  status = 
    JAC_make_net(gv, gv->nbr_seg - 1, 
		 &mem_count, mem_base, MEM_SIZE_FORCE_ALIGNMENT);
  if ( status != eTIesrEngineSuccess ) return status; 

  status = process_a_file_1(gv, total_frm, 0);

  if ( status == eTIesrEngineSuccess ) search_a_frame(NULL, 0, total_frm - 1, gv ); 

  if ( status == eTIesrEngineSuccess && gv->best_sym != USHRT_MAX ) {
    
    status = back_trace_beam(gv->best_sym, total_frm - 1,  
			     gv->hmm_code ,  gv->stt ,  gv->stp , 
			     &(gv->nbr_seg), &(gv->hmm_dlt), gv);
  }
  else {
    if (status == eTIesrEngineSuccess) status = eTIesrEngineAlignmentFail;
    PRT_ERR(fprintf(stderr,"\nalignment not found, error code = %d\n", status)); 
  }
  
  PRT_ERR(printf("R:\n"););

  JAC_recover_backuped_net(gv);

  return status;
}




/* GMHMM_SI_API */ 
/* HMM-level JAC_update w/wo force alignment */
TIesrEngineJACStatusType JAC_update(gmhmm_type *gvv)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  short jac_memory_base[ MAX_EM_MEMORY ];
  short last_lhd = BAD_SCR, curt_lhd;
  ushort em_iter, iter, d, coeff_1,  max_seg_len = 0; 
  long total_like, lld_forward;    /* total likelihood in Q6 */
  long *glb_accum = gv->accum;
  long *glb_accum_d = GET_D_OBJ(glb_accum, gv->n_filter);
  long *loc_accum = glb_accum + 2 * gv->n_filter;
  HmmType *hmm;
  ushort  frame_cnt[NBR_TRANSFO] /* total number of frames */;
  long temp[3*N_FILTER26];

  
  //IF word based backtrace is used, do a forced alignment here.
  if (gvv->word_backtrace==WORDBT)
  { 
   //printf("before FA %d %d %d %d %d %d!\n", gvv->stt[0], gvv->stp[0], gvv->stt[1], gvv->stp[1], gvv->stt[2], gvv->stp[2]);
   gvv->word_backtrace=HMMFA;

   /* freeze background scores obtained in the first pass */
   cm_freeze((ConfType*)gv->pConf); 

   if (jac_force_alignment(gv) != eTIesrEngineSuccess){
     gvv->word_backtrace=WORDBT;
     return  eTIesrEngineJACNoAlignment ;
   }

   gvv->word_backtrace=WORDBT;

   //printf("after FA %d %d %d %d %d %d\n", gvv->stt[0], gvv->stp[0], gvv->stt[1], gvv->stp[1], gvv->stt[2], gvv->stp[2]);
  }


//test code for HMM BT!!!
 /*  //printf("before FA %d %d %d %d %d %d!\n", gvv->stt[0], gvv->stp[0], gvv->stt[1], gvv->stp[1], gvv->stt[2], gvv->stp[2]);
   puts("Before FA====");
   for (int ii=0 ; ii< gvv->nbr_seg; ii++)
     printf (" %d, hmm: %d, st: %d, ed: %d    ", ii, gvv->hmm_code[ii], gvv->stt[ii], gvv->stp[ii]);
   puts("");
      
    gvv->stt[0]= gvv->stp[0]= gvv->stt[1]= gvv->stp[1]= gvv->stt[2]= gvv->stp[2]=0;
   gvv->word_backtrace=HMMFA;
   jac_force_alignment(gv);
   gvv->word_backtrace=HMMBT; //---- HMM BT !!
   
   //printf("after FA %d %d %d %d %d %d\n", gvv->stt[0], gvv->stp[0], gvv->stt[1], gvv->stp[1], gvv->stt[2], gvv->stp[2]);
     puts("after FA ===");
   for (int ii=0 ; ii< gvv->nbr_seg; ii++)
     printf (" %d, st %d, ed: %d    ", ii, gvv->stt[ii], gvv->stp[ii]);
   puts("");
 */
   


#ifdef USE_SBC
  sbc_rst_SBC_stat( gv->sbcState );
#endif

#ifdef USE_16BITMEAN_DECOD
  if (load_mean_vec(gv->chpr_Model_Dir, gv, FALSE) != eTIesrEngineSuccess)
    return eTIesrEngineJACMemoryFail;
#endif

#if defined(USE_CONFIDENCE) && defined(USE_CONF_DRV_ADP)
  cm_backup_adp(gv); 
#endif

  for (em_iter = 0; em_iter < MAX_EM_ITR; em_iter++) {  
    frame_cnt[0] = 0; 
    total_like = 0;
    
#ifdef USE_IJAC
    FOREACH(d, gv->n_filter) {
       /* backup previous estimates of channel distortion */
       temp[d] = gv->log_H[d];

       gv->log_H[d] = q15_x(gv->jac_discount_factor, gv->log_H[d]); 
    }
#else
    FOREACH(d, gv->n_filter) gv->log_H[d] = 0;
#endif

    for (iter = 0; iter<MAX_NEWTON_ITR; iter++) {
      ushort s, seg_len, em_mem; /* active memory size */
      short *state_lld, *bp, *alpha, *c_t_beta;

      FOREACH(d, gv->n_filter*2) loc_accum[d] = 0;
      for (s = 0; s < gv->nbr_seg; s++) {  /* for each segment */

	/* is useful for N-best, since there are multiple
	   alignments, which are saved in the vector gv->stp and gv->stt */ 
	if (s && (gv->stp[s] > gv->stp[s-1])) break; /* only use the top */

	seg_len = gv->stp[s] - gv->stt[s];
	//	printf("stt = %d, stp = %d\n", gv->stt[s], gv->stp[s]);
	
        if (seg_len > gv->jac_max_segment_size) {
	  // fprintf(stderr,"segment %d size limited to %d\n", seg_len,MAX_SEG_SIZE);
	  seg_len  = gv->jac_max_segment_size;
	}
	max_seg_len = MAX(max_seg_len, seg_len);
	hmm = GET_HMM(gv->base_hmms, gv->hmm_code[s], gv->hmm_dlt);
	em_mem = 0; /* active memory size */
	state_lld = jac_memory_base + em_mem;     
	em_mem += (NBR_STATES(hmm, gv->base_tran)-1) * seg_len;
	bp = jac_memory_base + em_mem;                
	em_mem += NBR_STATES(hmm,gv->base_tran) * seg_len; /* bwd prob */
	alpha = jac_memory_base + em_mem ;       
	em_mem += NBR_STATES(hmm, gv->base_tran) * 2; /* fwd prob, two colums t, t-1 */
	c_t_beta = jac_memory_base + em_mem;       
	em_mem += seg_len+1;
	/* total EM memory == em_mem */
	
	if (em_mem> MAX_EM_MEMORY) {
	  PRT_ERR(fprintf(stderr,"maximum size capacity exceeded in JAC\n"));
	  return eTIesrEngineJACMemoryFail;  /* lack of memory */
	}
#ifdef USE_SBC
	sbc_rst_seg_stat( gv->sbcState );
	sbc_observation_likelihood(gv->stt[s],seg_len,gv->hmm_code[s], 
				   hmm,state_lld, gv->n_mfcc,gv ); 
#else
	observation_likelihood(gv->stt[s],seg_len,hmm,state_lld, gv->n_mfcc,gv); /* maybe calc twice if shared */
#endif
	lld_forward = EM_beta(state_lld,bp,seg_len,hmm, c_t_beta,gv);

	if (lld_forward > -512000000) /* -1e6 in Q9 */ 
	  {
	    if( update_statistics(gv->hmm_code[s], hmm,state_lld, bp, gv->stt[s],
				  seg_len,alpha, c_t_beta,loc_accum, gv->log_H, 
				  gv->log_N, gv->n_mfcc, gv))
	      { 
		total_like += lld_forward; 
		/* info for frame average likelihood */
		frame_cnt[0] += seg_len;
		
#ifdef USE_SBC
		if (gv->comp_type & SBC) 
		  if (sbc_acc_SBC(gv->hmm_code[s], gv->nbr_dim, seg_len,
				  gv->sbcState , gv->sbc_fgt) == FALSE) 
		    return eTIesrEngineJACSVAFail;
#endif
	      }
	    
#ifdef USE_CONFIDENCE
	    cm_acc_seg_ratio_to_antimodel( gv, gv->hmm_code[s], gv->stt[s], gv->stp[s], lld_forward); 

#ifdef DBG_CONF
	    printf("sym[%d], code[%d], %d->%d, lk=%d\n", s, gv->hmm_code[s], gv->stt[s], gv->stp[s], lld_forward); 
#endif

#endif
	  }
	else PRT_ERR(printf("skip low prob segment ")); 
      }// for seg

#ifdef USE_CONFIDENCE
      cm_update_status(gv); 
#endif

#if defined(USE_CONFIDENCE) && defined(USE_CONF_DRV_ADP)
      if (gv->cm_score > ((ConfType*)gv->pConf)->adp_th ){
#endif

#ifdef USE_SBC
	sbc_update_SBC(gv); 
#endif

	/* garantee: glb_accum is self normalized: */
	coeff_1 = ONE_Q15 - gv->smooth_coef;
#ifndef USE_AUDIO // MSC++ GUI  doesn't support stderr 
	//      if (frm_cnt !=  frame_cnt[0]) 
	//	fprintf(stderr,"%d frames ignored\n", frm_cnt - frame_cnt[0]);
#endif
	
	FOREACH(d, gv->n_filter*2) { 
	  
	  long top = (gv->frm_cnt == 0)?0:(loc_accum[d] / gv->frm_cnt); /* each utterance has a same weight */
#ifdef USE_IJAC
	  glb_accum[d] = q15_x(gv->smooth_coef, glb_accum[d]) + top; 
#else
	  glb_accum[d] = q15_x(coeff_1, glb_accum [d]) + q15_x(gv->smooth_coef, top);
#endif
	}

	FOREACH(d,gv->n_filter) { 
#ifdef USE_IJAC
	  /*  In Q14, avoid div. by zero */
	  if (glb_accum_d[d] > MIN_ACCUM) 
	    {
	      temp[d + gv->n_filter] = glb_accum[d];
	      temp[d + 1 + gv->n_filter] = glb_accum_d[d] >> 2; /* Q 12 */
	      if (gv->jac_bayesian_weight != 0)
		{
		  /* use Bayesian IJAC */
		  lld_forward = q15_x(gv->jac_bayesian_weight, pPriorChannelInvvar[d]); /* Q9 */
		  temp[d] = lld_forward * (gv->log_H[d] - temp[d]);  /* Q18 */
		  temp[d] = LONG_RIGHT_SHIFT_N(temp[d], 6);  /* Q18 -> Q12 */
		  temp[d + gv->n_filter] += temp[d]; /* Q12 */
		  
		  temp[d + 1 + gv->n_filter] += (lld_forward << 3); /* Q12 */
		}
	      
	      temp[d] = div32_32(temp[d + gv->n_filter],
				 temp[d + 1 + gv->n_filter]);
	      temp[d] = q15_x(gv->jac_deweight_factor, temp[d]); 
	      gv->log_H[d] -= (short)temp[d]; 
	      
	      /* index_obj += loc_accum[d] * loc_accum[d]; */
	    }
#else
	  
	  /*  In Q16, avoid div. by zero */
	  if (glb_accum_d[d] > MIN_ACCUM) 
	    {
	      gv->log_H[d] -= (short)div32_32(glb_accum[d],(glb_accum_d[d]>>6)); 
	      
	      /* index_obj += loc_accum[d] * loc_accum[d]; */
	    }
#endif
	  
	}// for filter 
	
	
	gv->smooth_coef = gv->jac_smooth_coef;
	
	/* if (index_obj/N_FILTER < 0.02 ) break;    find max objective function: */
	
#ifdef USE_SVA
	/*
	** update scaling factor for sequential variance adaptation:
	*/
	for ( d = 0; d < gv->nbr_dim; d++ )  {
#ifdef USE_WSVA
	  gv->var_fisher_IM[d] = q15_x(8192, /* 0.25 in Q15 */
				       gv->var_fisher_IM[d]) 
	    + q15_x(24576, gv->var_fisher_IM_per_utter[d]);
#else
	  gv->var_fisher_IM[d] += gv->var_fisher_IM_per_utter[d];
#endif
	  
	  /* Sanity check, should not have divisor go to zero */
	  if ( gv->var_fisher_IM[d] == 0 ) {  
	    PRT_ERR(printf("dividing by zero error occurs during sequential variance adaptation!\n"));
	    return eTIesrEngineJACSVAFail;
	  }
	  
	  temp[d] = gv->var_fisher_IM_per_utter[d] - (((long) frame_cnt[0] ) << 9);
	  gv->log_var_rho[d] += (short) div32_32(temp[d], gv->var_fisher_IM[d]);  /* Q9 */   
	  
#ifndef USE_WSVA
	  /* Limit rho so variance does not get too small. */
	  if( gv->log_var_rho[d] < MIN_SVA_RHO )
	    {
	      gv->log_var_rho[d] = MIN_SVA_RHO;
	    }
#endif
	  
#ifdef ISVA
	  /*  APPLYING CONSTRAINTS   */
	  
	  // comments: 15200 14000 is also fine.. or 15360.  jac_prm->ceps.noise_mean[0] > 120)  
	  //under high noise, variance should decrease 
	  if (gv->mfcc_noise[ 0 ] > ISVA_NOISE_TH)    
	    {  

	      /* If noise is high, don't let log_var_rho go positive */
	      gv->log_var_rho[d]= gv->log_var_rho[d]<0? gv->log_var_rho[d]:0 ;
	      
	      // add constraint to C0 , which is too sensitive by itself
	      if (d == 0) 
		gv->log_var_rho[d] =  gv->log_var_rho[d]> -C0LIMIT? gv->log_var_rho[d]: -C0LIMIT;
	    }
	  
	  else
	    {  
	      if (d==0) 
		gv->log_var_rho[d]=0;
	      
	      /* If noise is low, don't let log_var_rho go negative */
	      gv->log_var_rho[d]= gv->log_var_rho[d]>0? gv->log_var_rho[d]:0 ;
	    } 
	  
	  /*nogoing testing code, not working yet. Setting limits to the rho adaptation*/
	  //gv->log_var_rho[d]= gv->log_var_rho[d]>256? 256: gv->log_var_rho[d];
	  //gv->log_var_rho[d]= gv->log_var_rho[d]< -? -256: gv->log_var_rho[d];	
	  
	  //NO adapt on C0! use the init value ONLY
	  //if (gv->mfcc_noise[ 0 ] > 16700 ) gv->log_var_rho[0]=-100;else gv->log_var_rho[0]=0;
#endif		
	}// for dim
	
	for ( d = 0; d < gv->nbr_dim; d++ )  
	  {
#ifndef USE_WSVA
	    /* multiply by the forgetting factor */
	    gv->var_fisher_IM[d] >>= gv->sva_var_forget_factor;
#endif
	    gv->var_fisher_IM_per_utter[d] = 0;
	  }
#endif

#if defined(USE_CONFIDENCE) && defined(USE_CONF_DRV_ADP)
      }else 
        cm_restore_adp(gv); 
#endif
      
    }// for inner-iter
    /* printf("max seg lenghth = %d\n", max_seg_len); */
    curt_lhd = (frame_cnt[0] == 0)?0:(short) (total_like / (long) frame_cnt[0]);
    //    test_convergence(curt_lhd, last_lhd, em_iter);  
    last_lhd = curt_lhd;

  }// for outer-iter

#ifdef USE_16BITMEAN_DECOD
  free(gv->base_mu_orig);
  gv->base_mu_orig = NULL;
#endif

  return eTIesrEngineJACSuccess;
}


/*----------------------------------------------------------------
  JAC_init_params

  Initialize JAC parameters in the gmhmm_type structure.
  ----------------------------------------------------------------*/
TIesrEngineJACStatusType JAC_set_default_params( gmhmm_type* gv )
{
   gv->jac_vec_per_frame = VEC_PER_FRAME_DEFAULT;
   gv->jac_component_likelihood = COMPONENT_LIKELIHOOD_DEFAULT;
   gv->jac_max_segment_size = MAX_SEG_SIZE_DEFAULT;
   gv->jac_smooth_coef = SMOOTH_COEF_DEFAULT;
   gv->jac_discount_factor = DISCOUNT_FACTOR_DEFAULT;
   gv->jac_deweight_factor = DEWEIGHT_FACTOR_DEFAULT;
   gv->sva_var_forget_factor = VAR_FORGET_FACTOR_DEFAULT;
   
   gv->jac_bayesian_weight = BAYESIAN_WEIGHT_DEFAULT;

   return eTIesrEngineJACSuccess;
}
