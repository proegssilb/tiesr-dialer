/*=======================================================================

 *
 * mfcc_f.cpp
 *
 * TIesr mfcc calculation.
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


 Functions dealing with mfcc calculation.
======================================================================*/


/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"

#include <stdlib.h>

#include "gmhmm_type.h"
#include "mfcc_f_user.h"
#include "mfcc_f.h"
#include "noise_sub_user.h"
#include "dist_user.h"


/* for FFT */
/*
const short w_real[ WINDOW_LEN / 2 ] =      
{
 32767,  32757,  32727,  32678,  32609, 
 32520,  32412,  32284,  32137,  31970, 
 31785,  31580,  31356,  31113,  30851, 
 30571,  30272,  29955,  29621,  29268, 
 28897,  28510,  28105,  27683,  27244, 
 26789,  26318,  25831,  25329,  24811, 
 24278,  23731,  23169,  22594,  22004, 
 21402,  20787,  20159,  19519,  18867, 
 18204,  17530,  16845,  16150,  15446, 
 14732,  14009,  13278,  12539,  11792, 
 11038,  10278,   9511,   8739,   7961, 
  7179,   6392,   5601,   4807,   4011, 
  3211,   2410,   1607,    804,      0, 
  -804,  -1607,  -2410,  -3211,  -4011, 
 -4807,  -5601,  -6392,  -7179,  -7961, 
 -8739,  -9511, -10278, -11038, -11792, 
-12539, -13278, -14009, -14732, -15446, 
-16150, -16845, -17530, -18204, -18867, 
-19519, -20159, -20787, -21402, -22004, 
-22594, -23169, -23731, -24278, -24811, 
-25329, -25831, -26318, -26789, -27244, 
-27683, -28105, -28510, -28897, -29268, 
-29621, -29955, -30272, -30571, -30851, 
-31113, -31356, -31580, -31785, -31970, 
-32137, -32284, -32412, -32520, -32609, 
-32678, -32727, -32757
};

const short w_imag[ WINDOW_LEN / 2 ] =
{
     0,   -804,  -1607,  -2410,  -3211, 
 -4011,  -4807,  -5601,  -6392,  -7179, 
 -7961,  -8739,  -9511, -10278, -11038, 
-11792, -12539, -13278, -14009, -14732, 
-15446, -16150, -16845, -17530, -18204, 
-18867, -19519, -20159, -20787, -21402, 
-22004, -22594, -23169, -23731, -24278, 
-24811, -25329, -25831, -26318, -26789, 
-27244, -27683, -28105, -28510, -28897, 
-29268, -29621, -29955, -30272, -30571, 
-30851, -31113, -31356, -31580, -31785, 
-31970, -32137, -32284, -32412, -32520, 
-32609, -32678, -32727, -32757, -32766, 
-32757, -32727, -32678, -32609, -32520, 
-32412, -32284, -32137, -31970, -31785, 
-31580, -31356, -31113, -30851, -30571, 
-30272, -29955, -29621, -29268, -28897, 
-28510, -28105, -27683, -27244, -26789, 
-26318, -25831, -25329, -24811, -24278, 
-23731, -23169, -22594, -22004, -21402, 
-20787, -20159, -19519, -18867, -18204, 
-17530, -16845, -16150, -15446, -14732, 
-14009, -13278, -12539, -11792, -11038, 
-10278,  -9511,  -8739,  -7961,  -7179, 
 -6392,  -5601,  -4807,  -4011,  -3211, 
 -2410,  -1607,   -804
};
*/

/* Hamming window */
/*
const short hw[ WINDOW_LEN ] = 
{
  2621,  2625,  2639,  2662,  2694,  2735,  2785,  2845, 
  2913,  2990,  3076,  3171,  3275,  3388,  3509,  3639, 
  3777,  3924,  4079,  4243,  4414,  4594,  4782,  4977, 
  5180,  5391,  5610,  5835,  6068,  6308,  6555,  6808, 
  7068,  7335,  7608,  7887,  8172,  8463,  8759,  9061, 
  9368,  9680,  9997, 10319, 10645, 10975, 11310, 11648, 
  11990, 12335, 12684, 13036, 13391, 13748, 14107, 14469, 
  14833, 15198, 15565, 15934, 16303, 16673, 17044, 17415, 
  17787, 18158, 18529, 18899, 19269, 19638, 20006, 20372, 
  20737, 21099, 21460, 21819, 22174, 22528, 22878, 23225, 
  23569, 23909, 24245, 24578, 24906, 25230, 25549, 25864, 
  26174, 26478, 26777, 27071, 27359, 27641, 27917, 28186, 
  28450, 28707, 28957, 29200, 29437, 29666, 29888, 30102, 
  30309, 30509, 30700, 30884, 31060, 31227, 31387, 31538, 
  31681, 31815, 31940, 32057, 32165, 32265, 32355, 32437, 
  32510, 32574, 32628, 32674, 32710, 32738, 32756, 32765, 
  32765, 32756, 32738, 32710, 32674, 32628, 32574, 32510, 
  32437, 32355, 32265, 32165, 32057, 31940, 31815, 31681, 
  31538, 31387, 31227, 31060, 30884, 30700, 30509, 30309, 
  30102, 29888, 29666, 29437, 29200, 28957, 28707, 28450, 
  28186, 27917, 27641, 27359, 27071, 26777, 26478, 26174, 
  25864, 25549, 25230, 24906, 24578, 24245, 23909, 23569, 
  23225, 22878, 22528, 22174, 21819, 21460, 21099, 20737, 
  20372, 20006, 19638, 19269, 18899, 18529, 18158, 17787, 
  17415, 17044, 16673, 16303, 15934, 15565, 15198, 14833, 
  14469, 14107, 13748, 13391, 13036, 12684, 12335, 11990, 
  11648, 11310, 10975, 10645, 10319,  9997,  9680,  9368, 
  9061,  8759,  8463,  8172,  7887,  7608,  7335,  7068, 
  6808,  6555,  6308,  6068,  5835,  5610,  5391,  5180, 
  4977,  4782,  4594,  4414,  4243,  4079,  3924,  3777, 
  3639,  3509,  3388,  3275,  3171,  3076,  2990,  2913, 
  2845,  2785,  2735,  2694,  2662,  2639,  2625,  2621
};
*/ 

/*
#define reg_factor 3276
*/

/* ---------------------------------------------------------------------------
   circular index on MFCC_BUF_SZ
   ---------------------------------------------------------------------- */

TIESRENGINECOREAPI_API short
circ_idx(short idx)
{
  while ( idx < 0 ) idx += MFCC_BUF_SZ;
  while ( idx >= MFCC_BUF_SZ ) idx -= MFCC_BUF_SZ;

  return idx;
}


/* ---------------------------------------------------------------------------
   regression
   ---------------------------------------------------------------------- */

TIESRENGINECOREAPI_API void
compute_regression(short buf_idx,     /* 0 <=  < MFCC_BUF_SZ */
		   short type,        /* 0: beg, 1: middle, 2: end */
		   short *reg_mfcc, short n_mfcc, const short *mu_scale_p2, short mfcc_buf[ /*MFCC_BUF_SZ*/ ][ MAX_DIM_MFCC16 ])
{
   short i, j;
   short i_adv, i_lag;
   long  sum;

   if ( type == 0 ) {                  /* beg, no lag, simple diff */

     i_adv = circ_idx( buf_idx + 1 );
     for (i = 0; i < n_mfcc; i++) {
       reg_mfcc[i] = mfcc_buf[ i_adv ][i] - mfcc_buf[ buf_idx ][i];
       reg_mfcc[i] <<= mu_scale_p2[ i ] - mu_scale_p2[ i + n_mfcc ];
     }

   } else if ( type == 1 ) {           /* both adv and lag, full regression */

     for (i = 0; i < n_mfcc; i++) {
       
       sum = 0;
       
       for (j = 1; j <= REG_SPAN; j++) {

	 i_adv = circ_idx( buf_idx + j );
	 i_lag = circ_idx( buf_idx - j );
	 sum += j * ( mfcc_buf[ i_adv ][i] - mfcc_buf[ i_lag ][i] ); 
       }
       /* compensate mu_scale_p2[] */
       sum <<= mu_scale_p2[ i ] - mu_scale_p2[ i + n_mfcc ];
       
       sum = reg_factor * sum;                   /* Q 15 * Q 11 = Q 26 */
       reg_mfcc[ i ] = (short)  LONG_RIGHT_SHIFT_N(sum, 15 );    /* Q 11 */
     }

   } else if ( type == 2 ) {           /* end, no adv, simple diff */

     i_lag = circ_idx( buf_idx - 1 );
     for (i = 0; i < n_mfcc; i++) {
       reg_mfcc[i] = mfcc_buf[ buf_idx ][i] - mfcc_buf[ i_lag ][i];
       reg_mfcc[i] <<= mu_scale_p2[ i ] - mu_scale_p2[ i + n_mfcc ];
     }

   } else {

     PRT_ERR(fprintf(stderr, "Error: type has to be 0 1 2\n"));
   }
   //   trouncate("D-MFCC", reg_mfcc, n_mfcc, n_mfcc);
}

#define accumulate_linear_mel_energy(tmp_l, tmp_s, mel_energy,  real_sig,  mel_filter) \
      tmp_l = mel_filter * real_sig; /* Q14 x Q14 */ \
      tmp_s = (short) LONG_RIGHT_SHIFT_N( tmp_l, 14 ); /* Q14  */      \
      tmp_l = tmp_s * tmp_s; /* Q 28 */ \
      tmp_l = LONG_RIGHT_SHIFT_N( tmp_l, 4 ); /* Q24  */ \
      mel_energy += tmp_l;


/* 
** apply mel scale to fft spectrum
*/

void
mel_scale_fft_spectrum(const short *real_sig, const short *imag_sig, 
		       long *mel_energy, const short n_filter, const mel_filter_type mel_filter[])
{
  short cur_filter, prev_filter;
  short i, tmp_s;
  long tmp_l;
  
  for (i = 0; i < n_filter; i++)
    mel_energy[i] = 0;

  for (i = 0; i < WINDOW_LEN / 2; i++) {

    cur_filter = mel_filter[i].num;
    prev_filter = cur_filter - 1;

    if ( prev_filter >= 0 ) {
      /* real */
      accumulate_linear_mel_energy(tmp_l, tmp_s, mel_energy[ prev_filter ],  real_sig[i],  mel_filter[i].x2);
      /* imag */
      accumulate_linear_mel_energy(tmp_l, tmp_s, mel_energy[ prev_filter ],  imag_sig[i],  mel_filter[i].x2);
    }
    if ( cur_filter < n_filter ) {
      /* real */
      accumulate_linear_mel_energy(tmp_l, tmp_s, mel_energy[ cur_filter ],  real_sig[i],  mel_filter[i].x1);
      /* imag */
      accumulate_linear_mel_energy(tmp_l, tmp_s, mel_energy[ cur_filter ],  imag_sig[i],  mel_filter[i].x1);
    }
  }
}


static void
mel_scale(const short *power_spectrum, long *mel_energy, const short n_filter, const mel_filter_type mel_filter[])
{
  short cur_filter, prev_filter;
  short i;

  for (i = 0; i < n_filter; i++)
    mel_energy[i] = 0;

  for (i = 0; i < WINDOW_LEN / 2; i++) {

    cur_filter = mel_filter[i].num;
    prev_filter = cur_filter - 1;

    if ( prev_filter >= 0 )
      mel_energy[ prev_filter ] += (long) 
	( mel_filter[i].x2 * power_spectrum[i] );

    if ( cur_filter < n_filter )
      mel_energy[ cur_filter ] += (long) 
	( mel_filter[i].x1 * power_spectrum[i] );
  }
}

#ifdef USE_FAST_DCT
/* ---------------------------------------------------------------------------
   cosine transform, notice the scaling and transpose of matrix
   ---------------------------------------------------------------------- */
/* faster cosine transform */
void
cos_transform(short *mfcc, short *mel_energy, const short *scale_p2, 
	      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  short i, j, mfcc0;
  long  tmp, sum[ N_FILTER26 ];

  mfcc0 = mfcc[0];
  mfcc[0] >>= 1;                                /* scale */

  for (i = 0; i < n_filter; i++) {
   
    sum[i] = 0;

    for (j = 0; j < n_mfcc; j++) {

      /* Q 15 * Q 11 = Q 26, >> 17 = Q 9 ... see above*/
      tmp = cosxfm[j][i] * mfcc[j];             /* transpose */
      tmp >>= ( 13 - scale_p2[j] );             /* scale back -- this scale doesn't allow simplification*/
      sum[i] += tmp;
    }
  }
  
  mfcc[0] = mfcc0;  /* restore the value */
  for (i = 0; i < n_filter; i++) {
    sum[i]=(( sum[i]*819)  + (sum[i]>>2)- (sum[i]>>4) ) >>17;  //0.05*2^14= 819.2 =>  819+0.25 -0.0625
    mel_energy[i] = (short) sum[i];
  }
}

/* ---------------------------------------------------------------------------
   inverse cosine transform
   ---------------------------------------------------------------------- */
/* faster version of inverse cosine transform */
void
inverse_cos_transform(short *log_mel_energy, short *mfcc, const short scale_p2[], 
		      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  short i, j, oe,  n2;
  long  tmp, sum;

  oe=0;
  n2=n_filter>>1;
  
  for (i = 0; i < n_mfcc; i++) {
    sum = 0;
   
    /* Q 15 * Q 9 = Q 24, >> 13 = Q 11 */
    if (oe==0)
      { for (j = 0; j < n2; j++) {
	tmp = (cosxfm[i][j]>>1) * (log_mel_energy[j]+ log_mel_energy[n_filter-1-j]) ;
	
	sum += tmp>>( 7 + scale_p2[i] );
      }
      oe=1;
      }  
    else
      {
	for (j = 0; j < n2; j++) {      	
	  tmp = cosxfm[i][j] * (log_mel_energy[j]- log_mel_energy[n_filter-1-j]) ;
	  
	  sum += tmp>>( 8 + scale_p2[i] );
	}
	oe=0;
      }
    mfcc[i] = (short) ((sum+ 16)>>5);
  }
}

/*very specific to 20*9 IDCT*/
void
inverse_cos_transform3(short *log_mel_energy, short *mfcc, const short scale_p2[], 
		      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  short i, j, oe,  n2;
  long  tmp, sum;

  oe=0;
  n2=n_filter>>1;
  
  for (i = 0; i < n_mfcc; i++) {
    sum = 0;
   
    switch (i)
      {
      case 1:
      case 3:
      case 7:
	for (j = 0; j < n2; j++) {      	
	  tmp = cosxfm[i][j] * (log_mel_energy[j]- log_mel_energy[n_filter-1-j]) ;
	  sum += tmp>>( 8 + scale_p2[i] );
	}
	break;
	
      case 2:
	for (j = 0; j < n2; j++) {
	  tmp = (cosxfm[i][j]) * (log_mel_energy[j]+ log_mel_energy[n_filter-1-j]) ;
	  sum += tmp>>( 8 + scale_p2[i] );
	}
	break;
   	
      case 0:
	for (j = 0; j < n_filter; j++)
          sum+=log_mel_energy[j];
	sum= (sum<<14) - (sum>>1); //*32767  then >>1  = *(32768-1) 
	sum>>=( 7 + scale_p2[i] );
	break;
	
      case 4:
      
	tmp=log_mel_energy[0]-log_mel_energy[4]-log_mel_energy[5]+log_mel_energy[9]+log_mel_energy[10]-log_mel_energy[14]-log_mel_energy[15]+log_mel_energy[19];
	sum+=tmp* cosxfm[i][0];
      
	tmp=log_mel_energy[1]-log_mel_energy[3]-log_mel_energy[6]+log_mel_energy[8]+log_mel_energy[11]-log_mel_energy[13]-log_mel_energy[16]+log_mel_energy[18];
	
	sum+=tmp* cosxfm[i][1];
	
	sum>>= ( 8 + scale_p2[i] );
	break;
	
      case 5:

	tmp=log_mel_energy[0]-log_mel_energy[3]-log_mel_energy[4]+log_mel_energy[7]+log_mel_energy[8]  
	  -log_mel_energy[11]-log_mel_energy[12]+log_mel_energy[15]+log_mel_energy[16]-log_mel_energy[19];
	sum+=tmp* cosxfm[i][0];
	
	tmp=log_mel_energy[1]-log_mel_energy[2]-log_mel_energy[5]+log_mel_energy[6]+log_mel_energy[9]  
	  -log_mel_energy[10]-log_mel_energy[13]+log_mel_energy[14]+log_mel_energy[17]-log_mel_energy[18];
	
	sum+=tmp* cosxfm[i][1];
	
	sum>>=( 8 + scale_p2[i] );
	break;
	
      case 6:
	tmp=log_mel_energy[0]-log_mel_energy[9]-log_mel_energy[10]+log_mel_energy[19];
	
	sum+=tmp* cosxfm[i][0];
	
	tmp=log_mel_energy[1]-log_mel_energy[8]-log_mel_energy[11]+log_mel_energy[18];
	
	sum+=tmp* cosxfm[i][1];
	
	tmp=log_mel_energy[2]-log_mel_energy[7]-log_mel_energy[12]+log_mel_energy[17];
	
	sum+=tmp* cosxfm[i][2];
      
	tmp=log_mel_energy[3]-log_mel_energy[6]-log_mel_energy[13]+log_mel_energy[16];
	
	sum+=tmp* cosxfm[i][3];
      
	tmp=log_mel_energy[4]-log_mel_energy[5]-log_mel_energy[14]+log_mel_energy[15];
	
	sum+=tmp* cosxfm[i][4];
       
	sum>>=( 8 + scale_p2[i] );
	break;
	
      case 8:

	tmp=log_mel_energy[0]+log_mel_energy[4]+log_mel_energy[5]+log_mel_energy[9]+log_mel_energy[10]+log_mel_energy[14]+log_mel_energy[15]+log_mel_energy[19];
	
	sum+=tmp* cosxfm[i][0];
	
	tmp=log_mel_energy[1]+log_mel_energy[3]+log_mel_energy[6]+log_mel_energy[8]+log_mel_energy[11]+log_mel_energy[13]+log_mel_energy[16]+log_mel_energy[18];
      
	sum+=tmp* cosxfm[i][1];
	
	tmp=log_mel_energy[2]+log_mel_energy[7]+log_mel_energy[12]+log_mel_energy[17];
	
	sum+= (tmp* cosxfm[i][2]); 
	
	sum>>=( 8 + scale_p2[i] );
	break;
	
      default:
	PRT_ERR(fprintf(stderr,"DCT dimension error"));
	/* Q 15 * Q 9 = Q 24, >> 13 = Q 11 */
	
      }
   
    //rounding by adding  1<< 2^4  before >>5
    mfcc[i] = (short) ((sum+ 16)>>5);
    
    
  }

/*just for verification
   for (i = 0; i < n_mfcc; i++) {

    sum = 0;

    for (j = 0; j < n_filter; j++) {

      
      tmp = cosxfm[i][j] * log_mel_energy[j];
      tmp += (long) 1 << ( 13 + scale_p2[i] - 1 );
      tmp >>= ( 13 + scale_p2[i] );
      sum += tmp;
    }
    printf("%d========%d, =======new%d\n", i, sum, mfcc[i] ); //= (short) sum;
  }
*/

}
#else
/*the generic one*/
void
cos_transform(short *mfcc, short *mel_energy, const short *scale_p2, 
	      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  short i, j, mfcc0;
  long  tmp, sum[ N_FILTER26 ];

  mfcc0 = mfcc[0];
  mfcc[0] >>= 1;                                /* scale */

  for (i = 0; i < n_filter; i++) {

    sum[i] = 0;

    for (j = 0; j < n_mfcc; j++) {

      /* Q 15 * Q 11 = Q 26, >> 17 = Q 9 */
      tmp = cosxfm[j][i] * mfcc[j];             /* transpose */
      tmp += (long) 1 << ( 17 - scale_p2[j] - 1 );
      tmp >>= ( 17 - scale_p2[j] );             /* scale back */
      sum[i] += tmp;
    }
  }
  mfcc[0] = mfcc0;  /* restore the value */
  for (i = 0; i < n_filter; i++) {
    sum[i] = sum[i] / n_filter;  /* this is a division ! need to remove */
    sum[i] <<= 1;
    mel_energy[i] = (short) sum[i];
  }

}

void
inverse_cos_transform(short *log_mel_energy, short *mfcc, const short scale_p2[], 
		      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  short i, j;
  long  tmp, sum;

  for (i = 0; i < n_mfcc; i++) {

    sum = 0;

    for (j = 0; j < n_filter; j++) {

      /* Q 15 * Q 9 = Q 24, >> 13 = Q 11 */
      tmp = cosxfm[i][j] * log_mel_energy[j];
      tmp += (long) 1 << ( 13 + scale_p2[i] - 1 );
      tmp >>= ( 13 + scale_p2[i] );
      sum += tmp;
    }
    mfcc[i] = (short) sum;
  }
}

#endif

/* used for generating MFCC from noise-subtracted spectra */
void
ss_power_spectrum_to_mfcc(const short *power_spectrum, 
			  short *mfcc,            /* Q 11 */
			  short *log_mel_energy,  /* Q 9 */
			  const short n_mfcc, const short n_filter, 
			  const mel_filter_type mel_filter[],
			  const short *mu_scale_p2, NormType *var_norm, 
			  const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  long  mel_energy[ N_FILTER26 ];          /* variable Q point, see below */
  short i;
  long  tmp;
  short compensate;

  /* power_spectrum : Q norm3,
     mel_filter : Q 14*/
  mel_scale(power_spectrum, mel_energy, n_filter, mel_filter);
  
  /* mel_energy : Q24 */
  
  /* log cannot handle 0, so substitute with min / 2 */
  tmp = LONG_MAX;    /* LONG_MAX */
  for (i = 0; i < n_filter; i++)
    if ( mel_energy[ i ] != 0 && mel_energy[ i ] < tmp ) 
      tmp = mel_energy[ i ];

  if( tmp == LONG_MAX )
     tmp = 0;

  tmp = tmp >= 2 ? ( tmp >> 1 ) : tmp;

  for (i = 0; i < n_filter; i++)
    if ( mel_energy[ i ] == 0 ) mel_energy[ i ] = tmp;    

  /* now, mel_energy has scale factor:
     (norm2 - 2) - 2 * (8 - norm0 - norm1) + 1 - 16 + 14 (x1,x2) = 
     2 * norm1 + 2 * norm0 + norm2 - 19 */

  /* compensate = 2 * var_norm->norm0 + 2 * var_norm->norm1 + var_norm->norm2 - 19; */
  compensate = 2*(var_norm->norm0 + var_norm->norm1 - WINDOW_PWR ) +
          var_norm->norm2 - 3;

  for (i = 0; i < n_filter; i++) 
    log_mel_energy[i] = log_polyfit( mel_energy[i], compensate);
  
  /* log2() to log10() */
  for (i = 0; i < n_filter; i++) {
    tmp = log_mel_energy[i] * LOG2TOLOG10;
    
    log_mel_energy[i] = (short) LONG_RIGHT_SHIFT_N(tmp,15);
  }
  //  prt_freq("mfcc", log_mel_energy, 9, n_filter);  
  inverse_cos_transform(log_mel_energy, mfcc, mu_scale_p2, n_mfcc, n_filter, cosxfm);
}

#ifdef REC

/* ---------------------------------------------------------------------------
   power_spectrum to mfcc
   ---------------------------------------------------------------------- */

extern void prt_freq(char *mesg, short x[], unsigned short q, unsigned short N);

extern FILE *pf_sp;
#include <assert.h>

void
power_spectrum_to_mfcc(const short *power_spectrum, 
		       short *mfcc,            /* Q 11 */
		       short *log_mel_energy,  /* Q 9 */
		       const short n_mfcc, const short n_filter, 
		       const mel_filter_type mel_filter[],
		       const short *mu_scale_p2, NormType *var_norm, const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  long  mel_energy[ N_FILTER26 ];          /* variable Q point, see below */
  short i;
  long  tmp;
  short compensate;

  /* power spectrum scaling: */
  short my_norm3 = ( var_norm->norm0 + var_norm->norm1 - 8 ) * 2 + ( var_norm->norm2 - 2 ) - 15;
  
  for (i=0; i< WINDOW_LEN/2; i++) {
    float spect_f;
    long tmp_l = (long)  power_spectrum[ i ];
    short signal_abs_log2;
    assert( tmp_l >=0);
    
    if (tmp_l > 0) /* log, with correction of  normalization factor */
      signal_abs_log2 = log_polyfit( tmp_l, my_norm3);   /* Q 9 */
    else 
      signal_abs_log2 =  /* 20 */ 1<<9 ; /* ~65 dB in Q9 */
    spect_f = (float)   signal_abs_log2/ (1<<9);
    my_fwrite(&spect_f, sizeof(float),1,pf_sp);
  }

  mel_scale(power_spectrum, mel_energy, n_filter, mel_filter);

  
  /* log cannot handle 0, so substitute with min / 2 */
  tmp = 2147483647;    /* LONG_MAX */
  for (i = 0; i < n_filter; i++)
    if ( mel_energy[ i ] != 0 && mel_energy[ i ] < tmp ) 
      tmp = mel_energy[ i ];

  tmp = tmp >= 2 ? ( tmp >> 1 ) : tmp;

  for (i = 0; i < n_filter; i++)
    if ( mel_energy[ i ] == 0 ) mel_energy[ i ] = tmp;    

  /* now, mel_energy has scale factor:
     (norm2 - 2) - 2 * (8 - norm0 - norm1) + 1 - 16 + 14 (x1,x2) = 
     2 * norm1 + 2 * norm0 + norm2 - 19 */

  compensate = 2 * var_norm->norm0 + 2 * var_norm->norm1 + var_norm->norm2 - 19;

  for (i = 0; i < n_filter; i++) 
    log_mel_energy[i] = log_polyfit( mel_energy[i], compensate);
  
  /* log2() to log10() */
  for (i = 0; i < n_filter; i++) {
    tmp = log_mel_energy[i] * LOG2TOLOG10;
    
    log_mel_energy[i] = (short) LONG_RIGHT_SHIFT_N(tmp,15);
  }

  inverse_cos_transform(log_mel_energy, mfcc, mu_scale_p2, n_mfcc, n_filter, cosxfm);
}
#endif

/*
** convert fft spectrum to mfcc coefficients
*/
void
fft_spectrum_to_mfcc(const short *real_sig,const short *imag_sig,
		     short *mfcc,            /* Q 11 */
		     short *log_mel_energy,  /* Q 9 */
		     const short n_mfcc, const short n_filter, 
		     const mel_filter_type mel_filter[],
		     const short *mu_scale_p2, NormType *var_norm, const short *cosxfm[  MAX_DIM_MFCC16 ])
{
  long  mel_energy[ N_FILTER26 ];          /* variable Q point, see below */
  short i;
  long  min, tmp;
  short compensate;

  /* power spectrum scaling: */
  
  mel_scale_fft_spectrum(real_sig, imag_sig, mel_energy, n_filter, mel_filter);
  /* mel_energy is Q24 */
  /* log cannot handle 0, so substitute with min / 2 */
  min = 2147483647;    /* LONG_MAX */
  for (i = 0; i < n_filter; i++)
    if ( mel_energy[ i ] != 0 && mel_energy[ i ] < min ) 
      min = mel_energy[ i ];

  min = min >= 2 ? ( min >> 1 ) : min;

  for (i = 0; i < n_filter; i++)
    if ( mel_energy[ i ] == 0 ) mel_energy[ i ] = min;    

  /* now, mel_energy has scale factor:
     (norm2 - 2) - 2 * (8 - norm0 - norm1) + 1 - 16 + 14 (x1,x2) = 
     2 * norm1 + 2 * norm0 + norm2 - 19 */

  /*  compensate = 2 * var_norm->norm0 + 2 * var_norm->norm1 + var_norm->norm2 - 19; */
  /* norm2 unused, norm2 - 2 + 1 -> -2  */
  compensate = 2 * var_norm->norm0 + 2 * var_norm->norm1 -1 - 19;

  for (i = 0; i < n_filter; i++) 
    log_mel_energy[i] = log_polyfit( mel_energy[i], compensate);
  
  /* log2() to log10() */
  for (i = 0; i < n_filter; i++) {
    tmp = log_mel_energy[i] * LOG2TOLOG10;
    
    log_mel_energy[i] = (short) LONG_RIGHT_SHIFT_N(tmp,15);
  }
  //  prt_freq("mfcc", log_mel_energy, 9, n_filter);  
  inverse_cos_transform(log_mel_energy, mfcc, mu_scale_p2, n_mfcc, n_filter, cosxfm);
}

/* ---------------------------------------------------------------------------
   bit reverse order, used in FFT
   ---------------------------------------------------------------------- */

#ifdef REC
static void
bit_rev(short *x, short n_fft, short n_stage)
{
  short i, j;
  short idx;
  short x_rev[ WINDOW_LEN ];

  for (i = 0; i < n_fft; i++) {

    idx = 0;

    for (j = 0; j < n_stage; j++)
      idx = ( idx << 1 ) | ( ( i >> j ) & 1 );    /* bit reverse */

    x_rev[ i ] = x[ idx ];
  }

  for ( i = 0; i < WINDOW_LEN; i++)
    x[i] = x_rev[i];
}

#endif
/* ---------------------------------------------------------------------------
   bit reverse order, used in FFT
   ---------------------------------------------------------------------- */

static void
bit_rev(short *x1, short *x2, short n_fft, short n_stage)
{
  short i, j, k, n_half, n_minus_one;
  short idx;
  short done[ WINDOW_LEN ];
  short tmp;

  for (i = 0; i < n_fft; i++) done[i] = 0;

  n_half = n_fft >> 1;
  n_minus_one = n_fft - 1;

  for (i = 1; i < n_half; i++) {

    if ( done[i] == 0 ) {

      idx = 0;

      for (j = 0; j < n_stage; j++)
        idx = ( idx << 1 ) | ( ( i >> j ) & 1 );    /* bit reverse */

      if ( i != idx ) {                             /* swap i and idx */

        tmp = x1[i];
        x1[i] = x1[idx];
        x1[idx] = tmp;
        tmp = x2[i];
        x2[i] = x2[idx];
        x2[idx] = tmp;

        done[i] = 1;                       /* do not repeat */
        done[idx] = 1;

        k = ( ~ i ) & n_minus_one;         /* do the same for 1's complement */
        idx = ( ~ idx ) & n_minus_one;

        if ( done[k] == 0 ) {

          tmp = x1[k];
          x1[k] = x1[idx];
          x1[idx] = tmp;
          tmp = x2[k];
          x2[k] = x2[idx];
          x2[idx] = tmp;

          done[k] = 1;                     /* do not repeat */
          done[idx] = 1;
        }
      }
    }
  }
}


/* ---------------------------------------------------------------------------
   decimation in time FFT
   ---------------------------------------------------------------------- */
    
TIESRENGINECOREAPI_API void
fft(short *real, short *imag)
{
  short n_fft;
  short n_stage;
  short group_cnt, group_size;
  short idx1, idx2, widx;
  short i, j;
  long  r1, i1, r2, i2;

  /* n_stage = log2( n_fft ) */

  n_fft = WINDOW_LEN;
  n_stage = 0;
  while ( n_fft >>= 1 ) n_stage++;

  /* decimation in time, bit reverse order input, normal order output */

  bit_rev( real, imag, (short) WINDOW_LEN, n_stage );    /* overwrite */
  /*   bit_rev( imag, (short) WINDOW_LEN, n_stage ); */

  /* n_stage butterfly */

  group_cnt = WINDOW_LEN >> 1;
  group_size = 2;

  while ( n_stage-- ) {

    for (i = 0; i < group_cnt; i++) {

      for (j = 0; j < group_size / 2; j++) {

	/* idx1 and idx2 are the butterfly pair, in-place overwrite,
	   widx is the W_n */

	idx1 = group_size * i + j;
	idx2 = idx1 + group_size / 2;
	widx = group_cnt * j;
	
	/* input data is assumed Q15 */
	r1 = (long) real[idx1] << 15; /* Q 30 */
	i1 = (long) imag[idx1] << 15; /* Q 30 */

	/* may need cast (long), depending on machine */
	/* w_real and w_imag are Q15 DFT twiddle factor coefficients */
	r2 = real[idx2] * w_real[widx] - imag[idx2] * w_imag[widx]; /* Q 30 */
	i2 = real[idx2] * w_imag[widx] + imag[idx2] * w_real[widx];

	real[idx1] = (short) LONG_RIGHT_SHIFT_N( (r1 + r2), 16);
	  
	imag[idx1] = (short) LONG_RIGHT_SHIFT_N( (i1 + i2), 16);
	 
	real[idx2] = (short) LONG_RIGHT_SHIFT_N( (r1 - r2), 16);
	imag[idx2] = (short) LONG_RIGHT_SHIFT_N( (i1 - i2), 16);  /* Q30 - Q16 = Q14 */
      }
    }

    group_cnt /= 2;
    group_size *= 2;
  }
}

/* ---------------------------------------------------------------------------
   apply Hamming window
   ---------------------------------------------------------------------- */

static short hamming_window(short *sig)
{
  short i;
  long  tmp, max;
  short norm1;
  max = 0;

  /* find max */

  for (i = 0; i < WINDOW_LEN; i++) {
    tmp = sig[i] * hw[i];             /* Q 15 * Q 15 = Q 30 */
    tmp = tmp > 0 ? tmp : (- tmp);
    if ( tmp > max ) max = tmp;
  }

  /* normalize to most precision */

  if ( max > 0 ) {

    for (norm1 = 0; max < 0x40000000; norm1++)
      max <<= 1;
    
    norm1 = norm1 > 0 ? norm1 - 1 : norm1;

  } else {

    norm1 = 0;
  }

  /* Hamming window */

  for (i = 0; i < WINDOW_LEN; i++) {
    tmp = sig[i] * hw[i];
    tmp <<= norm1;
    sig[i] = (short) LONG_RIGHT_SHIFT_N(tmp, 15 );
  }
  return norm1;
}

/* ---------------------------------------------------------------------------
   preemphasis
   ---------------------------------------------------------------------- */
static short preemphasis(short *sig_in, short *sig_out, short *last_sig_0)
{
  //  static short last_sig = 0; 
  short last_sig = *last_sig_0;
  short  i;
  long   tmp1, tmp2;
  short  tmp, max, norm0;

  max = last_sig > 0 ? last_sig : (- last_sig);

  /* find max */

  for (i = 0; i < WINDOW_LEN; i++) {
    tmp = sig_in[ i ] > 0 ? sig_in[ i ] : (- sig_in[ i ]);
    if ( tmp > max ) max = tmp;
  }

  /* normalize to most precision */

  if ( max > 0 ) {

    for (norm0 = 0; max < 0x4000; norm0++)
      max <<= 1;
    
    norm0 = norm0 > 0 ? norm0 - 1 : norm0;

  } else {

    norm0 = 0;
  }

  tmp1 = (last_sig << norm0) * PREEMPHASIS;          /* Q 15 * Q 15 = Q 30 */
  tmp2 = (long) (sig_in[0] << norm0) << 15;
  sig_out[0] =  LONG_RIGHT_SHIFT_N( tmp2 - tmp1, 15);  /* Q 15 */

  for (i = 1; i < WINDOW_LEN; i++) {
    tmp1 = (sig_in[ i - 1 ] << norm0) * PREEMPHASIS;
    tmp2 = (long) (sig_in[ i ] << norm0) << 15;
    sig_out[i] = LONG_RIGHT_SHIFT_N( tmp2 - tmp1, 15);
  }
  
  *last_sig_0 = sig_in[ FRAME_LEN - 1 ];
  return norm0;
}


/* ---------------------------------------------------------------------------
   complex fft to power spectrum
   ---------------------------------------------------------------------- */

static short fft_to_power_spectrum(const short *real, const short *imag, short *power_spectrum)
{
  short i, norm2;
  long  tmp, max;

  max = 0;

  /* find max */
  
  for (i = 0; i < WINDOW_LEN >> 1; i++) {
    tmp = real[i] * real[i] + imag[i] * imag[i];
    tmp <<= 1; /* Times 2 */
    if ( tmp > max ) max = tmp;
  }

  /* normalize to most precision */

  if ( max > 0 ) {

    for (norm2 = 0; max < 0x40000000; norm2++)
      max <<= 1;

  } else { /* max == 0 */

    norm2 = 2;
  }


  //  assert( ( norm2 - 2 ) < 0);
  
  /* power spectrum, normalize */

  for (i = 0; i < WINDOW_LEN >> 1 ; i++) {
    tmp = real[i] * real[i] + imag[i] * imag[i];
    tmp <<= 1;
    tmp <<= ( norm2 - 2 );
    power_spectrum[i] = (short)LONG_RIGHT_SHIFT_N( tmp, 16 );
    //    power_spectrum[i] = tmp;
  }
  return norm2;
  
}
  
/* ---------------------------------------------------------------------------
   speech signal to power_spectrum
   ---------------------------------------------------------------------- */

void
signal_to_power_spectrum(short *sig, short real_sig[], short imag_sig[], 
			 short *power_spectrum, NormType *nv, short *last_sig)
{
  short i;

  nv->norm0 = preemphasis(sig, real_sig, last_sig);   /* do not overwrite sig */

  nv->norm1 = hamming_window(real_sig);     /* overwrite real_sig */

  for (i = 0; i < WINDOW_LEN; i++)
    imag_sig[ i ] = 0;

  fft(real_sig, imag_sig);      /* in-place FFT, overwrite real and imag_sig */

  nv->norm2 = fft_to_power_spectrum(real_sig, imag_sig, power_spectrum);
}


/* ---------------------------------------------------------------------------
   MFCC is in Q11 
   ---------------------------------------------------------------------- */
TIESRENGINECOREAPI_API void
mfcc_a_window(short *sig, short *mfcc, short *log_mel_energy, const short n_mfcc, 
	      const short n_filter,
	      const mel_filter_type mel_filter[],
	      const short *cosxfm[  MAX_DIM_MFCC16 ],
              short power_spectrum[], const short *mu_scale_p2, NormType *var_norm, 
	      short *last_sig, short * ctrl)
{
  short real_sig[ WINDOW_LEN ], imag_sig[ WINDOW_LEN ], norm, signal_abs_log2;

  signal_to_power_spectrum(sig, real_sig, imag_sig, power_spectrum, var_norm, last_sig);

  norm = ( var_norm->norm0 + var_norm->norm1 - 8 ) * 2 + ( var_norm->norm2 - 2 ) - 15;

  if (ctrl){
    signal_abs_log2 = ss_est_frmEn(WINDOW_LEN >> 1, power_spectrum, norm);
    ss_update_meanEn(signal_abs_log2, ctrl);
  }

  fft_spectrum_to_mfcc(real_sig, imag_sig, mfcc, log_mel_energy, n_mfcc, n_filter, mel_filter, mu_scale_p2, var_norm, cosxfm );

  if (ctrl){
    ((NssType*)ctrl)->frm_count ++; 
  }
}

/*--------------------------------*/
TIESRENGINECOREAPI_API void dim_p2_init(const short nbr_mfcc, gmhmm_type *gvv)
{

  gmhmm_type *gv = (gmhmm_type *)gvv;
  int i;

  if (nbr_mfcc == MAX_DIM_MFCC16 || nbr_mfcc == MAX_DIM_MFCC13 ) {
    //    printf("Using MAX_DIM_MFCC16 x N_FILTER26\n");
    gv->n_filter = N_FILTER26 ;
    for (i=0; i<MAX_DIM_MFCC16; i++) gv->cosxfm[ i ] = cosxfm26[ i ];
    gv->mel_filter = mel_filter26;
  }
  else {
    //    printf("Using MAX_DIM_MFCC10 x N_FILTER20\n");
    gv->n_filter = N_FILTER20;
    for (i=0; i<MAX_DIM_MFCC10; i++) gv->cosxfm[ i ] = cosxfm20[ i ];
    gv->mel_filter = mel_filter20;
  }

  /* mfcc dimension related: */
  for (i=0; i<DIM6; i++)  {
    gv->muScaleP2[ i ] = mu_scale_p2_sta[ i ];
    gv->muScaleP2[ i+nbr_mfcc ] = mu_scale_p2_dyn[ i ];
  }
  for (i = DIM6; i< nbr_mfcc; i++)  {
    gv->muScaleP2[ i ] = 0;
    gv->muScaleP2[ i+nbr_mfcc ] = 0;
  }
#ifdef REC
  if (nbr_mfcc == 8)       return mu_scale_p2_dim8;
  else if (nbr_mfcc == 10) return mu_scale_p2_dim10;
  else { 
    PRT_ERR(printf("feature dimension %d not implemented\n", nbr_mfcc));
    PRT_ERR(exit(0));
  }
#endif
}

short ss_est_frmEn(short dim, short * pds_signal, short norm)
{
  short i; 
  long tmp_l = 0;
  for (i=0; i < dim ; i++) { /* for each frequency bin */
    tmp_l = MAX(tmp_l, (long) pds_signal[i]); 
  }

  if (tmp_l > 0) /* log, with correction of  normalization factor */
    tmp_l = log_polyfit( tmp_l, norm);   /* Q 9 */
  else 
    tmp_l = NSS_MIN_FRAME_ENR;
  return LONG2SHORT(tmp_l); 
}

/* return 1/16 (0.5 + 16 * log2(x)) */
static short ss_frameEn( short s_log )
{
  return 16 + s_log ;
}

short ss_update_meanEn(short frm_eng, short * pNss)
{
  short frmEn = ss_frameEn(frm_eng);
  NssType * ctrl = (NssType*) pNss; 

  if (ctrl->frm_count < MIN_FRAME)
    ctrl->lambdaLTE = coef_1downto1over10_in_Q15[ ctrl->frm_count] ; 
  else
    ctrl->lambdaLTE = LAMBDA_LTE; 

  if (frmEn - ctrl->meanEn < SNR_THRESHOLD_UPD_LTE || ctrl->frm_count < MIN_FRAME){
    if (frmEn < ctrl->meanEn || ctrl->frm_count < MIN_FRAME)
      ctrl->meanEn += q15_x(ctrl->lambdaLTE, frmEn - ctrl->meanEn); 
    else
      ctrl->meanEn += q15_x(ctrl->lambdaLTEhigherE, frmEn - ctrl->meanEn);
    if (ctrl->meanEn < ENERGY_FLOOR) 
      ctrl->meanEn = ENERGY_FLOOR; 
  }

  if (ctrl->frm_count == SS_NUM_FRM_FOR_NOISE_DET ){
    PRT_ERR(printf("Noise level (log2) = %d\n", ctrl->meanEn)); 
    ctrl->noiselvl = ctrl->meanEn; 
  } 

  return frmEn; 
}


//#define TABULATE_FILTER
#ifdef TABULATE_FILTER

/* ---------------------------------------------------------------------------
   create MEL filter
   ---------------------------------------------------------------------- */
static  mel_filter_type mel_filter[ WINDOW_LEN / 2 ];

void
create_mel_filter()
{
  float factor, freq;
  float mel_freq[ N_FILTER26 + 2 ];
  float x1, x2;
  short i, j;

  /* solve ( factor ^ 10 = 4 ) */
  factor = (float) pow( 10.0, 
			( log10( (float) SAM_FREQ / 2 / CUT_FREQ ) 
			  * (float) 2 / N_FILTER26 ) );

  /* under 1000 Hz, linear */
  for (i = 0; i <= N_FILTER26 / 2 + 1; i++)
    mel_freq[ i ] = (float) CUT_FREQ * i * 2 / ( N_FILTER26 + 2 );

  /* above 1000 Hz, factor ^ n */
  for (i = N_FILTER26 / 2 + 2; i <= N_FILTER26 + 1; i++)
    mel_freq[ i ] = factor * mel_freq[ i - 1 ];

  /* triangular square window */
  factor = (float) SAM_FREQ / 2 / ( WINDOW_LEN / 2 );

  for (i = 0; i < WINDOW_LEN / 2; i++) {

    freq = i * factor;

    for (j = 0; j < N_FILTER26 + 1; j++) {

      if ( freq >= mel_freq[ j ] && freq < mel_freq[ j + 1 ] ) {

	mel_filter[i].num = j;
	x1 = ( freq - mel_freq[j] ) / ( mel_freq[ j + 1 ] - mel_freq[ j ] );
	x2 = 1 - x1;

	/* square, power spectrum energy domain */
	//	x1 = x1 * x1;
	//	x2 = x2 * x2;

	/* Q 14 */
	mel_filter[i].x1 = (short) ( x1 * 16384 );
	mel_filter[i].x2 = (short) ( x2 * 16384 );
      }
    }
    printf("{ %5d, %5d, %5d },", mel_filter[i].num, mel_filter[i].x1,mel_filter[i].x2);
    if (i % 3 == 2) printf("\n");
  }
  printf("\n");
  exit(0);
  
}

main()
{
  create_mel_filter();
  
}

#endif

/*
** init constants: doesn't have to be called each time dimension changes
*/

#ifdef REC
void
const_init()
{

  short i, j;
  /* init Hamming window */
#ifdef REC
  for (i = 0; i < WINDOW_LEN; i++)
    hw[i] = (short) ( 32767 *                                       /* Q 15 */
      ( 0.54 - 0.46 * cos( (float) 2 * PI * i / ( WINDOW_LEN - 1 ) ) ) );

#endif
  /* create mel-scaled, triangular-square filter */

  create_mel_filter();

  /* create cosine transform matrix */
  for (i = 0; i < MAX_DIM_MFCC; i++) {
    printf("{\n");
    for (j = 0; j < N_FILTER; j++){
      cosxfm[i][j] = (short) 
	( 32767 * cos( (float) i * ( j + 0.5 ) * PI / N_FILTER ) );
      printf("%6d, ",   cosxfm[i][j]);
      if (j == (N_FILTER/2-1)) printf("\n"); 
    }
    printf("},\n");
  }
  
#ifdef DO_FFT
  /* create FFT constants */
  for (i = 0; i < WINDOW_LEN / 2; i++) {
    w_real[i] = (short) ( 32767 * cos( (float) 2 * PI * i / WINDOW_LEN ) );
    w_imag[i] = (short) ( 32767 * ( - sin( (float) 2 * PI * i / WINDOW_LEN )));
  }    

  /* compute regression factor */
  reg_factor = 0;
  for (i = 1; i <= REG_SPAN; i++)
    reg_factor += i * i;
  reg_factor = (short) ( (float) 32767 / ( 2 * reg_factor ) );
#endif
}

main()
{
  const_init();
}

#endif

