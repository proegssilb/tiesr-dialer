/*=======================================================================
 pmc_f.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 
 This header file contains constants and parameters that are local
 to the pmc_f.cpp file. 

======================================================================*/


#ifndef PMC_F_H
#define PMC_F_H

/* parameters to specify the log-add operation */

/* -10 in Q9, limit where the correction term is negligible */
#define PMCLIMIT  -5120

/* -5 in Q9, where to split the evaluation of the log polynomial function */
#define LOGSPLIT  -2560

/* midranges of polynomial evaluations */
/* -7.5, for lower segment */
#define LOOFFSET  -3840

/* -2.5 for upper segment */
#define HIOFFSET  -1280 

/* coefficients for two subdivisions of log2(1+2^x) */
/* first for -10<=x<-5 , then -5<=x<=0, Q19 */
/* fist segment is Q19 */
/* second segment is Q15 */
#define LOG_ORDER  3
static const short coef_log[2][4] = { { 265, 1223, 2826, 4014 },
	       		              { 211, 1503, 4955, 7665 } };

/* coefficients for 2^-x, Q15 for -1<x<=0 */
#define TWO_ORDER 2

static const short coef_twomx[3] = { 5615, -21873, 32702 };

#endif
