/*=======================================================================

 *
 * jac_one.cpp
 *
 * JAC processing.
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


 This source implements a portion of the jac processing.

======================================================================*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "jac_one.h"
#include "use_audio.h"
*/

#include "tiesr_config.h"

#include "tiesrcommonmacros.h"
#include "mfcc_f_user.h"
#include "pmc_f_user.h"
#include "jac_one_user.h"
#include "jac_one.h"
#include "pack_user.h"
#include "dist_user.h"
#include "status.h"

//#ifdef USE_SNR_SS
#include "noise_sub_user.h"
//#endif

#if defined(USE_RAPID_SEARCH) || defined(RAPID_JAC)
#include "rapidsearch_user.h"
#endif

#if defined(USE_CONFIDENCE)
#include "confidence_user.h"
#endif

/*
** JAC live mode
*/

/*
** 1. Estimate noise
** 2. Convert noise into log-spectral domain
** 3. Do JAC for a block of mean vectors
*/

/*----------------------------------------------------------------
  GetTIesrJacRate

  This function returns the number of model mfcc mean vectors per frame
  that should be JAC compensated per input frame.  This function is 
  maintained for compatibility, but GetTIesrJAC should be used in
  the future.
  ----------------------------------------------------------------*/
/* GMHMM_SI_API */ 
void GetTIesrJacRate(gmhmm_type *gvv, short *number_mu_per_frame)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  *number_mu_per_frame = gv->jac_vec_per_frame;
}

/*
** Channel: N_FILTER (20), in Q9. optimal for WAVES AKG hands-free
*/


/*----------------------------------------------------------------
  SetTIesrJacRate

  This function sets the number of model mfcc mean vectors per frame
  that should be JAC compensated per input frame.  This function is 
  maintained for compatibility, but SetTIesrJAC should be used in
  the future.
  ----------------------------------------------------------------*/
/* GMHMM_SI_API */ 
void SetTIesrJacRate(gmhmm_type *gvv, short number_mu_per_frame)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  gv->jac_vec_per_frame = number_mu_per_frame;
}

#ifdef RAPID_JAC
/* JAC compensation via cluster-dependent bias */
TIesrEngineJACStatusType rj_proc( gmhmm_type * gv)
{
   TIesrEngineJACStatusType status = eTIesrEngineJACSuccess;
   
   rj_noise_compensation(gv);

   status = rj_compensate(gv);

/*  Should not be needed here, since this is done in rj_compensate
#ifdef USE_16BITMEAN_DECOD
   if( gv->base_mu_orig )
   {
      free(gv->base_mu_orig);
      gv->base_mu_orig = NULL; 
   }
#endif
*/

   return status;
}
#endif

/*----------------------------------------------------------------
  per_frame_jac_compensate

  This function compensates the next number of model mfcc mean vectors 
  specified by the jac_vec_per_frame parameter.  It keeps track of
  the number of complete cycles of compensation of all of the loaded
  mean vectors in gv->nbr_cpy.

  SVA NOTE!  If SVA is being used, then this function also compensates
  all model mfcc variance vectors given the SVA parameters.  Note that
  this compensation only takes place on the first frame of the first
  JAC compensation cycle, since the variance vectors do not depend on
  the present file noise estimate.  
  ----------------------------------------------------------------*/
TIesrEngineJACStatusType per_frame_jac_compensate(short mfcc_buf[], gmhmm_type *gv)
{
   //  void play_beep();        
   unsigned short i;
   long  tmp; 

   /* current noise in mfcc  */
   short curt_noise[ MAX_DIM_MFCC16 ];

#ifdef USE_SVA
   /* changed to the 10th frame, as it takes at least 10 frames to obtain estimates of the noise
      level of the current frame. 
      Used for Weighted SVA,
   */
   if (gv->signal_cnt==10 || gv->speech_detected == 1)//   if (gv->speech_detected == 1)
   {
      if(gv->comp_type & SVA) sva_compensate( gv );
   }
#endif

   
   /* Smooth the current noise estimate by lowpass filtering */
#ifdef LIVEMODE
   if  ( gv->signal_cnt >=MFCC_BUF_SZ){  
     
      for ( i = 0; i < gv->n_mfcc; i++ ) {
	 tmp = 1 * mfcc_buf[ i ] + (long) (gv->mfcc_noise)[ i ] * 15;
	 gv->mfcc_noise[ i ] = (short) ( tmp >> 4 );
       
	 curt_noise[i] = gv->mfcc_noise[ i ]; 
      }
     
   }
   else if ( gv->signal_cnt <  MFCC_BUF_SZ ) 
   {      
       
      for ( i = 0; i < gv->n_mfcc; i++ ) {
         if (gv->signal_cnt==0) {
	    gv->mfcc_noise_init_acc[i]=gv->mfcc_buf[0][i];
	    gv->mfcc_noise[ i ]=gv->mfcc_noise_init_acc[i];
         }
         else 
	 {
	    gv->mfcc_noise_init_acc[i]+=gv->mfcc_buf[gv->signal_cnt][i] ;
	    gv->mfcc_noise[ i ]=gv->mfcc_noise_init_acc[i]/(gv->signal_cnt+1); 
	 }
         curt_noise[i]=gv->mfcc_noise[ i ];
      }
   }
#else  
   /*FILE mode*/
   if  ( gv->signal_cnt <= FILEMODEWAIT){ 
      for ( i = 0; i < gv->n_mfcc; i++ ) {
	 if (gv->signal_cnt==0) {
	    gv->mfcc_noise_init_acc[i]=gv->mfcc_buf[0][i];
	    gv->mfcc_noise[ i ]=gv->mfcc_noise_init_acc[i];
	 }
	 else 
	 {gv->mfcc_noise_init_acc[i]+=gv->mfcc_buf[gv->signal_cnt][i] ;
	 gv->mfcc_noise[ i ]=gv->mfcc_noise_init_acc[i]/(gv->signal_cnt+1); 
	 }
	 curt_noise[i]=gv->mfcc_noise[ i ];
      }
   }
   else
   {
      for ( i = 0; i < gv->n_mfcc; i++ )
	 curt_noise[i]=gv->mfcc_noise[ i ];
   }
#endif

   if ((gv->comp_type & JAC) == 0) 
      return eTIesrEngineJACSuccess; 
       
   /* cepstrum to log mel power spectrum, Q 11 to Q 9 
   ** verified that log-pow average gives equivalent vector as mfss average.
   */
   
   cos_transform( curt_noise, gv->log_N, gv->muScaleP2, gv->n_mfcc, gv->n_filter, gv->cosxfm );

#ifdef RAPID_JAC
   /* ready for speaker to speak utterances */
   if ((gv->signal_cnt >= NOISEWAIT) || (gv->speech_detected == 1)) 
      gv->nbr_cpy = 1;    

   /* compensate models when the time for recognition begins */
   if (gv->speech_detected ==1 || gv->signal_cnt >=NOISEWAIT) 
   {
      TIesrEngineJACStatusType jacStatus;
      jacStatus = rj_proc(gv);
      if( jacStatus != eTIesrEngineJACSuccess )
	 return jacStatus;
   }
#else

#ifndef LIVEMODE  /*file mode*/
   if ( (gv->signal_cnt ==FILEMODEWAIT || gv->speech_detected ==1) &&gv->nbr_cpy==0  ) //The last part is to prevent jac from doing the second round compensation, may not be needed.
      for (i = 0; i < gv->n_mu ; i++) {  
#else
   int j;

   /* Number of Gaussian mean vectors to update this frame. It does not need
    to exceed the number of vectors available. */
   j = (gv->jac_vec_per_frame > gv->n_mu ) ?  gv->n_mu : gv->jac_vec_per_frame;

   /* If speech detected and not all Gaussian mean vectors have been
    compensated, then compensate all remaining uncompensated mean vectors. */
   if ( gv->speech_detected == 1 && gv->nbr_cpy == 0 )
   {
      j = gv->n_mu - gv->index_mean;
      j = j>0 ? j : 0;
   } 
       
   /*one have to wait for 10 frame before any JAC is done, unless speech has been detected*/
   if ( (gv->signal_cnt >=NOISEWAIT || gv->speech_detected ==1)  )  
      for (i = 0; i < j ; i++) {  
#endif
	 /* Wrap to start at beginning of mean vectors if necessary */
	 if ((gv->index_mean) == gv->n_mu) {

	    gv->index_mean = 0;
	 
	    /* Check for end of first cycle of JAC adaptation */
	    if (gv->nbr_cpy == 0) {
	       PRT_ERR(printf("NOW SPEAK ...\n"));
	       /*		printf("%d (ALL) mean vectors adapted\n", n_mu); */
#ifdef USE_AUDIO 
	       //        play_beep();  /* beep ==> all mean has been adapted */ 
#endif
	    }
	 
	    /* Completed another cycle of JAC adaptation, track cycle number */
	    gv->nbr_cpy += 1;
	 }
	   
	 jac_one_mean( gv, gv->index_mean );
       
	 (gv->index_mean)++;
      }

#endif

   return eTIesrEngineJACSuccess;
}

/*----------------------------------------------------------------
  jac_one_mean

  This function compensates a single mfcc vector, specified by 
  by the index_mean argument, for estimated channel and noise.
  ----------------------------------------------------------------*/
void jac_one_mean(gmhmm_type *gv, ushort index_mean)
{
  int i = index_mean, k = gv->nbr_dim;
  Boolean b8bit = FALSE;
#ifdef BIT8MEAN
  b8bit = TRUE;
  k = gv->n_mfcc;
#endif

  pmc( gv->base_mu_orig + i * k, gv->log_N, gv->base_mu + i * k,
       gv->log_H, 1, gv->n_mfcc, gv->n_filter, gv->muScaleP2, gv->scale_mu, gv->cosxfm, 
       NULL, b8bit);

}

#ifdef USE_SVA

#ifdef USE_WSVA
			
/* use the above polynormial to obtain the variance scaling vector 
 @return the inverse variance scaling value for dimension i, in Q12 */ 
static short wsva_var_scale(short noiseEn /* in Q9 */ , short i, short n_mfcc )
{
  short x, j;
  ushort ux; 
  long y, z; 


  /* Safety check */
  if( i < START_DIMENSION )
     return 0;

  else if( i < n_mfcc && i <= MAX_DIM_WSVA_STATIC )
  {
     /* Static MFCC coefficient has polynomial */
     j = i - START_DIMENSION;
  }

  else if( i >= n_mfcc && i <= (n_mfcc + MAX_DIM_WSVA_DELTA) )
  {
     /* Delta MFCC coefficient has polynomial */
     j = i - n_mfcc + MAX_DIM_WSVA_STATIC - START_DIMENSION + 1;
  }
  else
  {
     /* Delta MFCC coefficient without polynomial */
     return 0;
  }


  x = noiseEn >> 9; /* Q0 */
  if (wsva_polycoef[j][0] > 0)
    y = q15_x(wsva_polycoef[j][0], noiseEn); /* Q9 */
  else{
    ux = -wsva_polycoef[j][0]; 
    y = q15_x(ux, noiseEn); /* Q9 */
    y = -y; 
  }

  y *= (noiseEn >> 6); 
  z = x * wsva_polycoef[j][1]; /* Q12 */
  y = y + z + wsva_polycoef[j][2]; 
  /* limit the scaling value */
  /* change to 1.5 since too large variance scaling will result overflow in SBC estimate */
  if (y > 6144) y = 6144 ; /* 1.5 in Q12 */
  else if (y < 1170) y = 1170; /* 1/3.5 in Q12 */
  return (short) y; 
}

/* weighted SVA
   @param log_var_rho variance scaling vector, in log, in Q9,
   @param linear_rho inverse variance scaling vector, in Q12
   @param num_dimension Dimension of MFCC + Delta MFCC vectors
*/
static void sva_weightedSVA(short * log_var_rho, short * linear_rho, short num_dimension,
			    gmhmm_type* gv)
{
  short dim;
  short mlogrho;
  short sInvScale; 
  short alpha, onemalpha; 
  ushort 
    u06 = 19661 /* 0.6 in Q15 */,u002 = 655;
  NssType * pSS = (NssType*) gv->pNss;
  short sEn = 0;
  if (pSS) {
    sEn = pSS->meanEn; /* sEn noise level of the current utterance, in Q9 */
    
    if (sEn - pSS->prevMeanEn > UPPER_NOISE_LEVEL_CHANGE || 
	pSS->prevMeanEn - sEn > UPPER_NOISE_LEVEL_CHANGE ) 
      {
	alpha = u06; 
      }else{
	//       alpha = u01; 
	alpha = u002; 
       }
  }else alpha = u002; 
  
  onemalpha = 32767 - alpha; 
  
  for (dim = START_DIMENSION; dim < num_dimension; dim++) 
   {
      /* Obtain prior estimate based on SNR */
      sInvScale = wsva_var_scale( sEn, dim, gv->n_mfcc );
      
      /* Update multiplier estimate if prior exists */
      if( sInvScale > 0 )
      {
         sInvScale = q15_x( alpha, sInvScale ) + q15_x( onemalpha, linear_rho[dim] );
         linear_rho[dim] = sInvScale;

         mlogrho = log_polyfit( linear_rho[dim], 12 );
         mlogrho = LONG_RIGHT_SHIFT_N( (long) mlogrho, 3 )* ( LOG2Q9 >> 6 );
         //      mlogrho = (mlogrho == -32768)? 32767 : -mlogrho;
         mlogrho = -mlogrho;
         log_var_rho[dim] = mlogrho;
      }
  }
}

#endif

/*----------------------------------------------------------------
  sva_compensate

  This function performs sva compensation on all mfcc variance vectors
  that are presently loaded.
  ----------------------------------------------------------------*/
static void sva_compensate( gmhmm_type* gv )
{
   short org_precis[ MAX_DIM ];
   short new_precis[ MAX_DIM ];
   short linear_rho[ MAX_DIM ];
   short vecindex;
   short dim;
   long prod;
   short *invvarorig;
   short *invvar;
         
   /* Calculate the linear multiplying factors in linear_rho */
   sva_init_multipliers( gv->log_var_rho, linear_rho, gv->nbr_dim );

#ifdef USE_WSVA
   sva_weightedSVA(gv->log_var_rho, linear_rho, gv->nbr_dim, gv); 
#endif
   
   /* Locations of original and compensated inverse variances */
   invvarorig = gv->base_var_orig;
   invvar = gv->base_var;

   /* Loop over all inverse variance vectors */
   for( vecindex = 0; vecindex < gv->n_var; vecindex++ )
   {

      /* Obtain original clean model inverse variance in short org_precis */
#ifdef BIT8VAR

      /* Unpack to obtain original clean model inverse variance in short */
      vector_unpacking((unsigned short *)invvarorig, org_precis, 
		       gv->scale_var, gv->n_mfcc);

#else
      /* Copy the original short inverse variance vector */
      for( dim=0; dim < gv->nbr_dim; dim++ )
	 org_precis[dim] = invvarorig[dim];
#endif


      /* Do not compensate the dimensions of the original 
	 short inverse variance vector before the start dimension */
      for( dim = 0; dim < START_DIMENSION; dim++ )
	 new_precis[dim] = org_precis[dim];
      

      /* Compensate all other dimensions */
      for( dim = START_DIMENSION; dim < gv->nbr_dim; dim++ )     /* make sure start from 0, look the .h file*/
      {
	 /* Q12 * Q9 = Q21 */
	 prod = linear_rho[ dim ] * org_precis[ dim ];
	 prod = LONG_RIGHT_SHIFT_N(prod, 12);      /* Q9 */
	 prod = MIN(prod, 32767); 
	 new_precis[ dim ] = (short) prod;
      }

      /* Output the compensated variance vector */
#ifdef BIT8VAR
      vector_packing(new_precis, (ushort *)invvar, 
		     gv->scale_var, gv->n_mfcc);

      /* Go to next original/compensated variance pair */
      invvarorig += gv->n_mfcc;
      invvar += gv->n_mfcc;

#else
      for( dim=0; dim < gv->nbr_dim; dim++ )
	 invvar[dim] = new_precis[dim];

      /* Go to next original/compensated variance pair */
      invvarorig += gv->nbr_dim;
      invvar += gv->nbr_dim;
#endif


      /* Recalculate the Gaussian constant based on the
	 compensated variance */
      //    printf(" %f ", gv->base_gconst[i] / (float)(1<<6));
      gv->base_gconst[vecindex] = 
	 gauss_det_const( new_precis, 2, gv->muScaleP2, gv->nbr_dim);
      //    printf(" %f\n", gv->base_gconst[i] / (float)(1<<6));
  }
}


/*----------------------------------------------------------------
  sva_init_multipliers

  This function initializes the multipliers for the variance terms
  from the SVA log_var_rho values.  This needs to be done once, prior
  to modifying variances for an utterance to be recognized.

  The input logarithmic log_var_rho values are Q9.  The output linear
  l_rho values are Q12, limited to a maximum of exp(2).
  ----------------------------------------------------------------*/
static void sva_init_multipliers( const short* log_var_rho, 
				  short* linear_rho,
				  short num_dimension )
{
   short dim;
   short mlogrho;

   for (dim = START_DIMENSION; dim < num_dimension; dim++) 
   {
      /* Change sign in log domain, since in linear domain the multiplier
	 is multiplying inverse variance. */
      mlogrho = ( log_var_rho[dim] == -32768 ) ? 32767 : -log_var_rho[dim]; 


      /* Limit to ensure that we never decrease the variance (increase the
	 inverse variance) by more than a factor of exp(2) */

      /* Do not do the limiting here!
         Instead the limiting should be put inside the loop. */

      /* Q12 linear multiplier for inverse variance of hmm model Gaussians */
      linear_rho[dim] = expn_plus_q9(mlogrho); 
   }
}


/*----------------------------------------------------------------
  expn_plus_q9

  This helper function calculates the linear sva multipliers given the
  log multiplier value.  Input is the log value in Q9.  The returned
  value is the linear value in Q12, representing exp(input).  The user
  is responsible for ensuring that the input is never large enough to
  overflow the Q12 output. 
  ----------------------------------------------------------------*/
static unsigned short expn_plus_q9(const short xQ9)
{
  short shift = 0, x = xQ9;
  unsigned short l;
  
  if( x > (2 << 9)) x = ( 2 << 9 ); 

  /* Find negative input for hlr_expn.  Not a very accurate method. */
  while (x > 0) {
    x -= LOG2Q9;
    shift++;
  }

  l = hlr_expn( x, 9 ); /* Q15 */
  shift -= 3; /* Q15 - Q3 = Q12 */
  if (shift >= 0) return (l << shift);
  else return (l >> -shift);
}  


#endif //#ifdef USE_SVA
