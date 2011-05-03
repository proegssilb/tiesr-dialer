/*=======================================================================
 pack.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 Model parameter packing functions for byte and 16-bit parameter representation.

======================================================================*/

/*
#include <stdio.h>
#include "load.h"
#include "pack.h"
*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include <stdio.h>
#include "tiesr_config.h"
#include "pack_user.h"

#ifdef REC
extern short  *scale_mu;

#define SHC 1
void trouncate(char *mesg, short *mfcc, int n_mfcc, int delta)
  
  { 
    int i;
    
    short x,y, scl;
    
    //    printf("%s\n", mesg);
    for (i=0; i<n_mfcc; i++) {
      //      printf("old = %d, ", mfcc[i]);
      y = mfcc[i];
      scl = scale_mu[delta + i] - SHC;
      if (scl<0) scl=0;
      
      mfcc[i] = nbr_coding(mfcc[i], scl, 0 );
      x = mfcc[i] >>= scl;
      
      //      printf("new = %d\n", mfcc[i]>>scale_mu[delta + i]);

      // printf("SC=%2d o=%5d (%5x) n=%5d (%5x) , %5d %9.5f\n", scale_mu[delta + i], 
      //    y,y,x, x, x - y,    y == 0? 0.: (x-y)/(float)(x>0? x:-x)*100.);
    }
  }

#endif


/*----------------------------------------------------------------
  nbr_coding

  This function accepts a short value o_mu, and codes it into an 8-bit
  representation with scaling according to the shift amount specified
  by max_mu, and rounding.  The resultant 8-bit value will be placed
  in the msbyte of the returned short.

  --------------------------------*/
TIESRENGINECOREAPI_API short nbr_coding(short o_mu, short max_mu, int prt )
{
  short a_mu = o_mu,  sign = 0;
  int tmp;
  short clipped = 0;
  
  if (a_mu<0) { /* make a positiver number */
    a_mu = -a_mu;
    sign = 1;
  }
  tmp = (a_mu << max_mu); /* shift */
  if (tmp > 32767) {
    a_mu = 32767;
    clipped = 1;
  }
  else a_mu = (short) tmp;
  if (a_mu < 0x7f00) a_mu += (1<<7);
  /*  else printf("a_mu = %x, no rounding\n", a_mu); */

  /* 8 bits precision coding, with 8 bits in the msbyte */
  a_mu &= 0xff00; 


/* This code does not appear to have any impact anymore and so it is
   commented out.  Its function would be to rescale the 8-bit value in
   the msbyte back to the original short scaling position, creating a
   short value with 8-bit reduced precision representing the original
   input short. Is not used anywhere. Removed the COMPACT definition. */

//#ifndef   COMPACT 
// a_mu >>= max_mu; /* restore */
//  if (sign) a_mu = - a_mu;
//  if (prt) */ /*  printf("ZZ=%d, %5d %5d, %5d %9.5f\n", max_mu, a_mu, o_mu, a_mu - o_mu, 
//              o_mu == 0? 0.: (a_mu - o_mu)/(float)(o_mu>0? o_mu:-o_mu)*100.) */;
//#else
//  if (sign) a_mu = - a_mu;
//#endif


  if (sign) a_mu = - a_mu;


  /*
 if (clipped) 
    fprintf(stderr,"clipping: original = %x (%d), actual = %x (%d) scale = %d)\n",
	    o_mu, o_mu, a_mu>> max_mu, a_mu>> max_mu, max_mu );
  */


  return a_mu;
}


/*----------------------------------------------------------------
  vector_packing

  This function implements packing of a non-interleaved vector
  consisting of static features followed by dynamic features in
  shorts, into a vector of shorts where each short contains the
  interleaved static/dynamic features in bytes.  It is assumed that a
  short is two bytes.  The max_scale argument is a vector of scales
  that determines how to scale each original dimension prior to
  truncating it to a byte feature.   If the vector is NULL, then
  no scaling is done prior to truncating. 

  TODO: This function should be changed so that it can accommodate
  static, delta, acc..., which means that the values should not be
  interleaved.

  --------------------------------*/
TIESRENGINECOREAPI_API void vector_packing(const short v_in[], unsigned short v_out[],  const short max_scale[], short D)
{
   short j;
   char tmp1, tmp2;
  
   if( max_scale == NULL )
   {
      for (j = 0; j<D; j++) 
      {
	 tmp1 = nbr_coding( v_in[j], 0, 0 ) >> 8;
	 tmp2 = nbr_coding( v_in[j + D], 0, 0 ) >> 8;
	 v_out[j] = (tmp1 << 8) + (tmp2 & 0xff);
      }
   }
   else
   {
      for (j = 0; j<D; j++) 
      {
	 tmp1 = nbr_coding( v_in[j], max_scale[j], 0 ) >> 8;
	 tmp2 = nbr_coding( v_in[j + D], max_scale[j+D], 0 ) >> 8;
	 v_out[j] = (tmp1 << 8) + (tmp2 & 0xff);
      }
   }
}


/*----------------------------------------------------------------
  vector_unpacking

  This function implements the inverse of vector_packing.  It unpacks
  an interleaved static/dynamic feature vector where each static or
  dynamic feature is a byte, into a non-interleaved vector of shorts
  where the static features come first in the vector followed by the
  dynamic features.  It is assumed that a short is two bytes. The
  argument scale is an array of scales that determines the amount to
  shift the resulting short value in order to obtain the original Q
  point of the short values.

  --------------------------------*/
TIESRENGINECOREAPI_API void vector_unpacking(const unsigned short v_in[], short v_out[], const short scale[], short D)
{
   short d;

   for (d = 0; d < D ; d++) {
      DECODE_STDY(v_in[d], v_out[d], v_out[d + D]);

      if( scale != NULL )
      {
	 v_out[d] >>= scale[d];
	 v_out[d+D] >>= scale[d+D];
      }
   }
}
