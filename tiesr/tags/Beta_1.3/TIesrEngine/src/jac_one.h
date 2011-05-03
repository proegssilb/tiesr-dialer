/*=======================================================================
 jac_one.h

 *
 * jac_one.h
 *
 * Header for implmentation of jac processing.
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


 This header was modified as part of the modifications to add
 sva to the live-mode TIesrSI engine API.  This header contains
 constants declarations of internal functions used by the jac_one
 code.  The jac_one code performs jac and sva compensation.

======================================================================*/

#ifndef _JAC_ONE_H
#define _JAC_ONE_H

#include "tiesr_config.h"

/*--------------------------------*/
/*  Constants */

/* The number of leading dimensions not to compensate via sva.
   Found that compensating mfcc parameter c[0] hurt
   performance */
const short START_DIMENSION = 1; 

// Now in tiesrcommonmacros.h
// static const short LOG2Q9 = 355;

/* NOISEWAIT defines the number of frames that will be used to estimate the
noise spectrum prior to JAC starting.  Values between 5 and 10 frames are typical. */
#define NOISEWAIT 10  

/*--------------------------------*/
/*  Helper functions */


/*--------------------------------
  sva_init_multipliers

  Calculates the linear multipliers for SVA compensation in linear_rho
  from the log multipliers in log_var_rho.  The linear multipliers 
  multiply the inverse variance vector elements in the Hmm model 
  Gaussians.
  --------------------------------*/
static void sva_init_multipliers( const short* log_var_rho, 
				  short* linear_rho,
				  short num_dimension );


/*--------------------------------
  expn_plus_q9

  Calculates exp(x) for x represented as Q9. Returns Q12 short.
  x may be positive or negative.  The user is responsible for
  ensuring no overflow.
  --------------------------------*/
static unsigned short expn_plus_q9( const short x);

/*----------------------------------------------------------------
  sva_compensate

  This function performs sva compensation on all mfcc variance vectors
  that are presently loaded.
  ----------------------------------------------------------------*/
static void sva_compensate( gmhmm_type* gv );


#ifdef USE_WSVA

/* poly[0] * x * x + poly[1] * x + poly[0] ,
   variance scaling of C[1-9] and delta C[0-9] 
   poly[0] in Q15,
   poly[1] in Q12, 
   poly[2] in Q2 */
static short wsva_polycoef[19][3] = { { -62 , 720, -556 } , {4, 89, 276 }, { 43, -208, 5945} , 
				      { 37, -145, 4678 }, {29, -120, 4515 }, {35, -157, 4963 }, 
				      { 43, -225, 5816 }, { 4, 0, 3345 }, { 33, -194, 5780 }, 
				      { -117, 809, -6600 }, { -95, 785, -7538 }, {-16, 202, 23 }, 
				      { 24, -74, 3305}, {7, 29, 2192 }, { -2, 55, 2250 }, 
				      { 15, -38, 3243 }, { 23, -101, 4264 }, { 19, -72, 4088} , 
				      { 30, -157, 5383 }}; 

#define UPPER_NOISE_LEVEL_CHANGE 4096 /* 8 in Q9 */

#endif
			
#endif
