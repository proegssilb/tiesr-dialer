/*=======================================================================
 dist.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 Functions to calculate Gaussian likelihood, and associated processing.
 
======================================================================*/

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "dist.h"
#include "hlr_dist.h"
*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif


#include "tiesr_config.h"
#include "dist_user.h"

/* prototypes */


/*================================================================
  hlr_gauss_obs_score_f 

  Fixed point gaussian log likelihood calculation.  This version
  supports Viterbi mixtures, full mixture log likelihood, and mixture
  component likelihood calculation.

  ----------------------------------------------------------------*/

TIESRENGINECOREAPI_API short /*Q6 */ hlr_gauss_obs_score_f(const short *feature, int pdf_idx,
				     ObsType type, short *comp, const short n_dim, const gmhmm_type *gv )

{
  short i;
  long  scr;
  short total_scr;
  short n_mix;
  short *p2wgt;
  short *mixture;
  short imin, imax;
#ifndef BIT8MEAN 
  short j;
  long  diff;
  short *mu, *invvar, gconst;
  const short *feat;
  short diff_s;
#endif

/*--------------------------------*/

  total_scr = BAD_SCR;

  mixture = GET_MIX(gv,pdf_idx);
  n_mix = MIX_SIZE(mixture);
  p2wgt = mixture + 1;

  if( type == COMPONENT )
  {
     imin = *comp;
     imax = imin + 1;
  }
  else
  {
     imin = 0;
     imax = n_mix;
  }

  for (i = imin; i < imax; i++) {      /* n mixtures */
#ifndef BIT8MEAN
    mu     = get_mean(gv->base_mu, mixture, n_dim, i);
    invvar = get_var(gv->base_var, mixture, n_dim, i);
    gconst = gv->base_gconst[ mixture[ i * 3 + 3 ] ];
    feat = feature;
    /* macro formed to make this reusable */
    GAUSSIAN_DIST(gv, scr, feat, j, n_dim, diff, diff_s, gconst, mu, invvar);
#else
    GAUSSIAN_DIST(gv, scr, n_dim, mixture, i, feature);
#endif
    scr += p2wgt[ i * 3 ];         /* mixture weight */

    if( type == FULL )
    {
       /* total_scr = log( exp( total_scr ) + exp( scr ) ); */
       total_scr = ( i == 0 ) ? LONG2SHORT((long)scr) : hlr_logsum( total_scr, LONG2SHORT((long)scr) );
    }
    else
    {
       if( scr > (long) total_scr )
       { 
	  total_scr = scr;
	  *comp = i;
       }
    }
  }

  return total_scr;
}


/*================================================================
  hlr_logsum

  This function accepts two short arguments as input, ln(a) and ln(b),
  in Q6 format.  It returns ln( a + b ) in Q6 format.  It uses the
  factorization ln( a + b ) = ln(a) + ln( 1 + exp( ln(b)/ln(a) ) ),
  where b < a.

  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API short hlr_logsum( short lna, short lnb )
{
   short s_temp;
   long temp;
   long diff;
   short result;
   unsigned short adb;

   /* lna holds larger value, lnb smaller */
   if( lna < lnb )
   {
      s_temp = lnb;
      lnb = lna;
      lna = s_temp;
   }
	 
   /* check for lnb small enough to be ignored, difference -7 or less */
   diff = (long)lnb;
   diff -= (long)lna;
   if( diff < -448 )
      return( lna );
      
   /* find exponential of diff in unsigned short Q15 */
   s_temp = (short)diff;
   adb = hlr_expn( s_temp, 6 );

   /* find ln( 1 + adb ) in short Q15 */
   result = hlr_log_one( adb );

   /* round and return result in Q6 */
   temp = (long)result;
   temp += lna << 9;
   temp += 1<<8;
   temp >>= 9;
   if( temp > 32767 )
      temp = 32767;

   return (short)temp;
}

/*================================================================
  hlr_expn

  This function returns exp(x) for x in the range of -limit to 0. Note
  that x is short, and you must supply the Q point, between 0 and
  15, with x.  The output is unsigned short Q15.
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API unsigned short hlr_expn( short x, short qpt )
{
   /* log2(e) in Q14 */
   short ln2e = 23637;

   /* order 4 polynomial for 2^x,  in Q14 */
   short coef_pow2[5] = { 158, 909, 3936, 11357, 16384 };

   long temp;
   long itemp;
   unsigned long ftemp;
   unsigned short result;
   short frac, s_temp;
   short coef;

   /* base two exponent Q qpt+14 */
   temp = ln2e*x;
   
   /* work with positive numbers */
   temp = -temp;

   /* 
      We want to find 2^(-temp) = 2^(-I)*2^(-frac). 
      Integer portion of base two exponent 
   */
   itemp =  temp >> (qpt+14);
   if( itemp >= 16 )
      return 0;
   else
   {
      s_temp = (short) itemp;
      result = ( 32768 >> s_temp );
   }

   /* fractional portion of base two exponent in Q15 */
   ftemp = itemp << (qpt+14);
   ftemp = temp & ~ftemp;
   if( qpt + 14 <= 15 )
   {
      ftemp <<= 15 - qpt - 14;
   }
   else
   {
      ftemp += ( 1 << ( qpt + 14 - 16 ) );
      ftemp >>= ( qpt + 14 - 15 );
   }

   /* find the fraction between -.5 and .5 */
   frac = (short)ftemp;
   if( frac >= 16384 )
   {
      result >>= 1;
      frac += -32768;;
   }      
   frac = -frac;

   /* calculate the fractional power of two by polynomial */
   s_temp = coef_pow2[0];
   for( coef = 1; coef < 5; coef++ )
   {
      /* Q14*Q15 */
      temp = s_temp*frac;
      temp += (long)( coef_pow2[coef] << 15 );

      /* Q14 */
      temp += (long)1<<14;
      s_temp = (short)( temp >>= 15 );
   }

   /* multiply fractional exponent by integer exponent, Q15 * Q14 */
   temp = result * (unsigned short)s_temp;

   /* result in unsigned short Q15 */
   temp += 1<<13;
   return (unsigned short)( temp >> 14 );
}

/* exponential operation
   @param x data in Q_qpt
   @param qpt Q value of data x
   @param rNorm norm of returned exponential
   return exponential in Q_rNorm
*/
unsigned short hlr_expn_q( short x, short qpt, short *rNorm)
{
  ushort tmpx; 
  short norm; 
  long ltmp, lnum = 1073741824 /* 1 in Q30 */;

  *rNorm = 15; 

  if (x == 0){
    return ONE_Q15; 
  }else{
    if (x < 0)
      return hlr_expn(x, qpt);
    else{
      tmpx = hlr_expn(-x, qpt);
      if (tmpx == 0){
	tmpx = ONE_Q15; norm = 0;
      }else{
	ltmp = lnum / tmpx ; /* Q30 - Q15 = Q15 */
	norm = 15; 
	while(ltmp > ONE_Q15){ ltmp >>= 1; norm--; }
	tmpx = LONG2SHORT(ltmp);
      }
      *rNorm = norm; 
      return tmpx;
    }
  }
}

/*================================================================
  hlr_log_one( x );

  This function calculates ln( 1 + x ), where x is between 0
  and 1 and is in unsigned short Q15.
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API short hlr_log_one( unsigned short x )
{
   /* order 4 polynomial for ln( 1 + x ) in Q15 */
   short coef_lnopx[5] = { -1801, 7104, -15220, 32623, 4 };
   
   long temp;
   short s_temp;
   short data;
   short coef;
   
   /* special case if x is 1.0 Q15, return ln(2) in Q15 */
   if( x == 32768 )
      return( 22713 );

   
   /* polynomial fit for ln( 1 + x ) */
   data = (short)x;
   s_temp = coef_lnopx[0];
   for( coef = 1; coef < 5; coef++ )
   {
      /* Q15*Q15 */
      temp = s_temp * data;
      temp += (long)( coef_lnopx[coef] << 15 );

      /* to Q15 */
      temp += 1<<14;
      s_temp = (short)( temp >> 15 );
   }
      
   return  s_temp;
}

/*--------------------------------
  Multiplication of a Q15 and a long; should be in math API.
  --------------------------------*/
TIESRENGINECOREAPI_API long q15_x(ushort a, long b)
{
  long tmp;
  short norm, tmp_s;

  if (b==0) return 0;
  else {
    tmp = b > 0 ? b : -b;
    GET_NORM(tmp,norm); 
    tmp_s = tmp >> 16; 
    tmp = a * tmp_s;  /* Q 15 * Q -16 = Q -1 */
    tmp <<= 1;           /* Q 0 */
    tmp >>= norm;
    return b > 0 ? tmp : -tmp;
  }
}

/* multiplication of a and b. 
   return long with the same Qnumber of b's.
   @param a, must be smaller than 1, can have minus value, in Q14
*/
TIESRENGINECOREAPI_API long q14_x(short a, long b)
{
  long tmp;
  short norm, tmp_s;

  if (b==0) return 0;
  else {
    tmp = b > 0 ? b : -b;
    GET_NORM(tmp,norm);  /* to the maximum precision */
    tmp_s = tmp >> 16; 
    tmp = a * tmp_s;  /* Q14 * (-Q16) = Q -2 */
    tmp <<= 2;           /* Q 0 */
    tmp >>= norm;
    return b > 0 ? tmp : -tmp;
  }
}

/*--------------------------------
  div 32 32 return specified Q; should be in math API.
  --------------------------------*/
TIESRENGINECOREAPI_API long div32_32_Q(long num, long den, ushort q_prec)
{
  long tmp1, tmp2, tmp3;
  short norm_n, norm_d, d;
  
  if (num == 0) return 0;
  else {
    if (num < 0) tmp1 = -num;
    else tmp1 = num;

    if (den < 0) tmp2 = -den;
    else tmp2 = den; 
    GET_NORM(tmp1,norm_n);
    tmp2 = MAX(1, tmp2);
    GET_NORM(tmp2, norm_d);
    tmp2 >>= q_prec; /* Q - q_prec */
    tmp3 = tmp1 / tmp2;
    d = norm_d - norm_n;
    if (d>0) tmp3 <<=d;
    else tmp3 >>= -d;
    if (num<0) tmp3 = -tmp3;
    if (den<0) tmp3 = -tmp3; 
    return tmp3;
  }
}

/*--------------------------------*/
/*
** div 32 32 return Q9
*/
long div32_32(long num, long den)
{
  long tmp1, tmp2, tmp3;
  short norm_n, norm_d, d;
  
  if (num == 0) return 0;
  else {
    if (num < 0) tmp1 = -num;
    else tmp1 = num;
    if (den < 0) tmp2 = -den; 
    else tmp2 = den; 
    tmp2 = MAX(1, tmp2); 

    GET_NORM(tmp1,norm_n);
    GET_NORM(tmp2, norm_d);
    tmp2 >>= 9; /* Q -9 */
    tmp3 = tmp1 / tmp2; 
    d = norm_d - norm_n;
    if (d>0) tmp3 <<=d;
    else tmp3 >>= -d;
    if (num<0) tmp3 = -tmp3;
    if (den<0) tmp3 = -tmp3;
    return tmp3;
  }
}

/*--------------------------------
  Multiplication of a Q15 and a long; should be in math API.
  --------------------------------*/
TIESRENGINECOREAPI_API long fx_sqrt(long x)
{
  long y0, y1, z;

  if (x <= 0) {
    y1 = 0;
  } else {
    y1 = x >> 1;
    if (y1 == 0) return x; 
    else {
      do {
	y0 = y1;
	z = x / y0;
	y1 = (y0 + z) >> 1;
      } while (abs(y0 - y1) > 1);
    }
  }
  return y1;
}

/* ---------------------------------------------------------------------------
   polynomial fit for log2() from 0.5 to 1.0, in Matlab:

   x=[0.5:0.001:1.0];
   y=log2(x);
   z=polyfit(x,y,8);
   z*347.8996
   ---------------------------------------------------------------------- */

const short coef_logpoly[ LOGPOLYORD + 1 ] = {
  -781, 5338, -16182, 28637, -32768, 25419, -13767, 5640, -1536};

/* ---------------------------------------------------------------------------
   polynomial fit to compute log2()
   ---------------------------------------------------------------------- */
short
log_polyfit(long value, short compensate)
{
  //  static short result = 0;
  short result = 0;
  short data;
  short norm;
  short i;
  long  tmp;

  if ( value <= 0 ) {
    PRT_ERR(printf("negative log_polyfit value %ld\n", value)); /* should never happen */
    // return result;     /* repeat neighboring value */
    value = 1; /* need a better solution  */
  }

  /* normalize to most precision */

  for (norm = 0; value < 0x40000000; norm++)
    value <<= 1;

  /* positive, between 0.5 and 1, Q 15 */

  data = (short) ( value >> 16 );

  /* compute the log2(data) polynomial */
  
  result = coef_logpoly[ 0 ];

  for (i = LOGPOLYORD - 1; i >= 0; i--) {

    tmp = result * data;                                      /* Q 15 */
    tmp <<= 1;                                                /* Q 16 */
    tmp += ( (long) coef_logpoly[ LOGPOLYORD - i ] << 16 );   /* Q 16 */
    result = (short)  LONG_RIGHT_SHIFT_N( tmp, 16 );          /* Q 0 */
  }

  /* result >= -348, -348 * 24103 * 256 / (2^16) = -32765, -1 in Q 15 */
  tmp = result * MAX_COEF_LOGPOLY;                    /* Q 15 * Q 8 = Q 23 */
  tmp <<= 8;                                          /* Q 31 */

  /* Compensate previous normalizations:
     Here we treat 2^30 as 0.5, so we need to compensate 31,
     31 - norm -compensate */
  tmp >>= 6;                                          /* Q 25 */
  tmp -= (long) norm << 25;                           /* log norm */
  tmp -= (long) compensate << 25;

  tmp += 1040187392;                                  /* 31 in Q 25 */

  result =  LONG_RIGHT_SHIFT_N( tmp , 16 );        /* Q 25 - 16 = Q 9 */

  return result;                                      /* log2(value) in Q 9 */
}

/* ---------------------------------------------------------------------------
   polynomial fit to compute pow2()
   ---------------------------------------------------------------------- */

TIESRENGINECOREAPI_API long pow_polyfit( short value )
{
  short result;
  short data;
  short norm;
  short i;
  long  tmp;

  /* rounding */
  if ( value < -512 ) return (long) 0;
  else if ( value <= 0) return (long) 1; 
  if ( value > 15870 ) return (long) 2147483647; /* overflow clip */

  /* normalization to between 0 and 1, value in Q 9 */

  norm = (value &  0xfe00) >> 9;
  data =  value & ~0xfe00;

  /* compute the pow2(data) polynomial */
  
  result = coef_powpoly[ 0 ];

  for (i = POWPOLYORD - 1; i >= 0; i--) {

    tmp = result * data;                                      /* Q 9 */
    tmp += ( (long) coef_powpoly[ POWPOLYORD - i ] << 9 );    /* Q 9 */
    result = (short)  LONG_RIGHT_SHIFT_N( tmp, 9 );                            /* Q 0 */
  }

  /* 2.0 == 32725 >= result >= 16384 == 1.0 */
  tmp = result; 
  if ( norm > 14 ) {
    tmp = (long) tmp << ( norm - 14 );
  }
  else{
    i = 14-norm-1; 
    if (i>=0)   /* rounding */
      tmp += (1 << i); 
    tmp =  tmp >> ( 14 - norm );
  }

  return tmp;
}

/* return 2^x
   $param l_in in Q9,
   return Q0 */
long  pow2x(long l_in)
{
  short norm_pow; 
  long tmp_l = l_in; 
  long tmp_l2 = l_in; 

  tmp_l -= 15870; /* max value w/o overflowing pow_poly */
  if (tmp_l < 0) norm_pow = 0;
  else if (tmp_l == 0) norm_pow =1;
  else norm_pow = (tmp_l >> 9) + 1; 

  tmp_l2 -= 512 * norm_pow; /* norm_pow is dim-specific */

  tmp_l2 = pow_polyfit((short)tmp_l2);

  if (norm_pow>=0) tmp_l2 <<= norm_pow; 
  else tmp_l2 >>= (-norm_pow); 

  return tmp_l2; 
}

 
/*----------------------------------------------------------------
  gauss_det_const

  This function calculates the log of the Gaussian constant based on
  the present inverse variance values.  The leading normalization term
  in a Gaussian distribution is given by 1/((2*pi)^N/2 * det(COV)^1/2
  ).  Since in TIESR the covariance is stored as inverse covariance
  terms in which covariances are scaled by dividing by the square of
  the mean vector component scales, and TIESR implements the
  multiplication by the gaussian constant by subtracting the term in
  the log domain and then multiplying by 1/2, the Gaussian constant
  term is calculated as: N*log(2*pi) - sum(log(1/Ci) ).  Correction
  terms are added according to the scaling of the covariance
  components.
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API short gauss_det_const(short *cov, ushort power_factor,
		      const short mu_scale_p2[],
		      const short dim) 
{
  long sum  = 0;
  long sumc = 0;
  ushort d;

  /* Calculate log2 sum of covariance terms.  Subtraction is due
     to using inverse covariance. Returns Q9 result. */
  for (d=0; d<dim; d++) {
    sum  -= log_polyfit((long)cov[d],9); 

    /* this could be pre-calculated */
    if (mu_scale_p2) sumc += mu_scale_p2[d]; 
  }


  /* Convert to Ln, Q21 */
  sum *= LOG2Q12;


  /* Add the leading constant term, and account for inv variance
   scaling */
  sum += dim * LOG_2_PI + ( (power_factor * LOG2Q12 * sumc) << 9);

  /* Q6  */
  sum >>= 15;

  return (short)sum;
}

/* This function conducts uniq operation on a vector. Each element may contain its probability.
   In such case, same elements are merged with their prabilities are summed. 
   @param imax_10 pointer to elements to be compared
   @param prob probability of the elements
          the updated probability of an element is the sum of probablities of same elements 
*/
short uniq(short * imax_10, short *prob , short inum)
{
  short i, j, k; 

  for (i=0;i<inum;i++){
    for (j=i+1;j<inum;){
      if ( imax_10[i] == imax_10[j] ){
	if (prob) 
	  prob[i] += prob[j]; 
	for (k=j;k<inum-1;k++){
	  imax_10[k] = imax_10[k+1];
	  if (prob) 
	    prob[k] = prob[k+1]; 
	}
	inum--; 
      }else j++;
    }
  }

  return inum;
}

