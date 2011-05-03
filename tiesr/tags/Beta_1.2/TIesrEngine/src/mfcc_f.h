/*=======================================================================
 mfcc_f.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 
 Parameters for calculating mfcc vectors and transforms.
======================================================================*/


#ifndef  MFCC_F_H
#define  MFCC_F_H

#include  <stdio.h>
#include  <math.h>

#include "winlen.h"

#include "tiesrcommonmacros.h"
#include "mfcc_f_user.h"

/*--------------------------------*/
/* These are now in mfcc_f_def_struct_user.h */
// size of buffer holding 2-byte audio samples that user must allocate

/* window len = FFT len */
/*
#define   WINDOW_LEN   256                  
*/

/* overlap between two frames */
/*
#define   OVERLAP  (WINDOW_LEN - FRAME_LEN) 
*/

/*
#define   N_FILTER20     20
*/

/*
#define   N_FILTER26     26
*/

/* maximum number of mfcc */
/*
#define   MAX_DIM_MFCC10 10         
*/

/* maximum number of mfcc */
/*
#define   MAX_DIM_MFCC16 16          
*/

/* regression span */
/*
#define   REG_SPAN     2                  
*/

/* #define   MFCC_BUF_SZ  (2 * REG_SPAN + 1) */
/*
#define   MFCC_BUF_SZ  (2 * REG_SPAN + 10)
*/

/*--------------------------------*/
/* Local constants for mfcc processing */

#define   PI           3.1415926

// Cingular 2125 Windows audio capture already seems to be doing
// preemphasis
#if defined (WIN32) || defined (WINCE)
#define   PREEMPHASIS  33                   /* 0.001 in Q 15 */
#else
  #define   PREEMPHASIS  31457              /* 0.96 in Q 15 */
#endif


#define   SAM_FREQ     8000
#define   CUT_FREQ     1000

#define   LOGPOLYORD   8
#define   MAX_COEF_LOGPOLY  24103         /* 94.15, Q8 */

/* log10(2) = 0.30103 in Q 15 */
/*
#define   LOG2TOLOG10  9864               
*/

/* log2(10) = 3.3219 in Q 13 */
/*
#define   LOG10TOLOG2  27213              
*/

/* 0.6 in Q 15, 0.6 for wvdig, 1 for wvcmd */
/* Presently this is not used anywhere */
#define   PMC_GAIN     19661              

/* for JAC */
/*
#define ACC_MEMORY (N_FILTER26 * 2 * 2)  
*/


/*  log2(10)/ln(10) in Q13 */
/*
#define LC   11818  
*/

/* ln 10 in Q13 */
/*
#define LN10 18863 
*/


#define reg_factor 3276


/* These were originally in set_dim.cpp */
#ifdef REC
/* DIM MFCC == 8 */
static const short mu_scale_p2_dim8 [ ] = { 4, 2, 2, 1, 1, 1, 0, 0,
                                      2, 1, 1, 0, 0, 0, 0, 0};

/* DIM MFCC == 10 */
static const short mu_scale_p2_dim10[ ] = { 4, 2, 2, 1, 1, 1, 0, 0, 0, 0,
                                      2, 1, 1, 0, 0, 0, 0, 0, 0, 0};

#endif

#define DIM6 6
static const short mu_scale_p2_sta[ ] = { 4, 2, 2, 1, 1, 1, };
static const short mu_scale_p2_dyn[ ] = { 2, 1, 1, 0, 0, 0, };


/*--------------------------------*/
/* These were originally in frontend_const.h */

/* filter with 20 frequency bins (to be used with 8, 10 dimension mfcc): */

static const mel_filter_type mel_filter20[ WINDOW_LEN / 2 ] =
  {
{     0,     0, 16384 },{     0,  5632, 10752 },{     0, 11264,  5120 },
{     1,   512, 15872 },{     1,  6144, 10240 },{     1, 11776,  4607 },
{     2,  1024, 15359 },{     2,  6656,  9727 },{     2, 12288,  4095 },
{     3,  1536, 14847 },{     3,  7168,  9215 },{     3, 12800,  3583 },
{     4,  2048, 14335 },{     4,  7680,  8703 },{     4, 13312,  3071 },
{     5,  2560, 13823 },{     5,  8192,  8191 },{     5, 13824,  2559 },
{     6,  3072, 13311 },{     6,  8704,  7680 },{     6, 14335,  2048 },
{     7,  3583, 12800 },{     7,  9216,  7167 },{     7, 14848,  1535 },
{     8,  4096, 12287 },{     8,  9728,  6655 },{     8, 15359,  1024 },
{     9,  4608, 11776 },{     9, 10240,  6143 },{     9, 15872,   511 },
{    10,  5120, 11263 },{    10, 10752,  5631 },{    11,     0, 16384 },
{    11,  3443, 12940 },{    11,  6886,  9497 },{    11, 10329,  6054 },
{    11, 13772,  2611 },{    12,   724, 15659 },{    12,  3721, 12662 },
{    12,  6719,  9664 },{    12,  9716,  6667 },{    12, 12714,  3669 },
{    12, 15711,   672 },{    13,  2024, 14359 },{    13,  4633, 11750 },
{    13,  7243,  9140 },{    13,  9852,  6531 },{    13, 12462,  3921 },
{    13, 15071,  1312 },{    14,  1129, 15254 },{    14,  3400, 12983 },
{    14,  5672, 10711 },{    14,  7944,  8439 },{    14, 10215,  6168 },
{    14, 12487,  3896 },{    14, 14759,  1624 },{    15,   563, 15820 },
{    15,  2540, 13843 },{    15,  4518, 11865 },{    15,  6495,  9888 },
{    15,  8473,  7910 },{    15, 10451,  5932 },{    15, 12428,  3955 },
{    15, 14406,  1977 },{    16,     0, 16383 },{    16,  1721, 14662 },
{    16,  3443, 12940 },{    16,  5164, 11219 },{    16,  6886,  9497 },
{    16,  8608,  7775 },{    16, 10329,  6054 },{    16, 12051,  4332 },
{    16, 13772,  2611 },{    16, 15494,   889 },{    17,   724, 15659 },
{    17,  2223, 14160 },{    17,  3721, 12662 },{    17,  5220, 11163 },
{    17,  6719,  9664 },{    17,  8218,  8165 },{    17,  9716,  6667 },
{    17, 11215,  5168 },{    17, 12714,  3669 },{    17, 14213,  2170 },
{    17, 15711,   672 },{    18,   719, 15664 },{    18,  2024, 14359 },
{    18,  3329, 13054 },{    18,  4633, 11750 },{    18,  5938, 10445 },
{    18,  7243,  9140 },{    18,  8547,  7836 },{    18,  9852,  6531 },
{    18, 11157,  5226 },{    18, 12462,  3921 },{    18, 13766,  2617 },
{    18, 15071,  1312 },{    18, 16376,     7 },{    19,  1129, 15254 },
{    19,  2265, 14118 },{    19,  3400, 12983 },{    19,  4536, 11847 },
{    19,  5672, 10711 },{    19,  6808,  9575 },{    19,  7944,  8439 },
{    19,  9080,  7303 },{    19, 10215,  6168 },{    19, 11351,  5032 },
{    19, 12487,  3896 },{    19, 13623,  2760 },{    19, 14759,  1624 },
{    19, 15895,   488 },{    20,   563, 15820 },{    20,  1551, 14832 },
{    20,  2540, 13843 },{    20,  3529, 12854 },{    20,  4518, 11865 },
{    20,  5507, 10876 },{    20,  6495,  9888 },{    20,  7484,  8899 },
{    20,  8473,  7910 },{    20,  9462,  6921 },{    20, 10451,  5932 },
{    20, 11439,  4944 },{    20, 12428,  3955 },{    20, 13417,  2966 },
{    20, 14406,  1977 },{    20, 15395,   988 }
};

/*

    {    0,     0, 16384}, {    0,  1936,  7056}, {    0,  7744,  1600}, 
    {    1,    16, 15376}, {    1,  2304,  6400}, {    1,  8464,  1295}, 
    {    2,    64, 14399}, {    2,  2704,  5775}, {    2,  9216,  1023}, 
    {    3,   144, 13455}, {    3,  3136,  5183}, {    3, 10000,   783}, 
    {    4,   256, 12543}, {    4,  3600,  4623}, {    4, 10816,   575}, 
    {    5,   400, 11663}, {    5,  4096,  4095}, {    5, 11664,   399}, 
    {    6,   576, 10815}, {    6,  4624,  3600}, {    6, 12543,   256}, 
    {    7,   783, 10000}, {    7,  5184,  3135}, {    7, 13456,   143}, 
    {    8,  1024,  9215}, {    8,  5776,  2703}, {    8, 14399,    64}, 
    {    9,  1296,  8464}, {    9,  6400,  2303}, {    9, 15376,    15}, 
    {   10,  1600,  7743}, {   10,  7056,  1935}, {   11,     0, 16384}, 
    {   11,   723, 10221}, {   11,  2894,  5505}, {   11,  6512,  2237}, 
    {   11, 11577,   416}, {   12,    32, 14967}, {   12,   845,  9785}, 
    {   12,  2755,  5701}, {   12,  5762,  2713}, {   12,  9866,   821}, 
    {   12, 15067,    27}, {   13,   250, 12585}, {   13,  1310,  8427}, 
    {   13,  3202,  5099}, {   13,  5925,  2603}, {   13,  9479,   938}, 
    {   13, 13864,   105}, {   14,    77, 14203}, {   14,   705, 10288}, 
    {   14,  1963,  7002}, {   14,  3851,  4347}, {   14,  6369,  2322}, 
    {   14,  9517,   926}, {   14, 13295,   161}, {   15,    19, 15277}, 
    {   15,   394, 11696}, {   15,  1246,  8593}, {   15,  2575,  5967}, 
    {   15,  4382,  3819}, {   15,  6666,  2148}, {   15,  9428,   954}, 
    {   15, 12667,   238}, {   16,     0, 16383}, {   16,   180, 13121}, 
    {   16,   723, 10221}, {   16,  1628,  7682}, {   16,  2894,  5505}, 
    {   16,  4522,  3690}, {   16,  6512,  2237}, {   16,  8864,  1145}, 
    {   16, 11577,   416}, {   16, 14653,    48}, {   17,    32, 14967}, 
    {   17,   301, 12239}, {   17,   845,  9785}, {   17,  1663,  7606}, 
    {   17,  2755,  5701}, {   17,  4122,  4069}, {   17,  5762,  2713}, 
    {   17,  7677,  1630}, {   17,  9866,   821}, {   17, 12329,   287}, 
    {   17, 15067,    27}, {   18,    31, 14976}, {   18,   250, 12585}, 
    {   18,   676, 10402}, {   18,  1310,  8427}, {   18,  2152,  6659}, 
    {   18,  3202,  5099}, {   18,  4459,  3747}, {   18,  5925,  2603}, 
    {   18,  7598,  1667}, {   18,  9479,   938}, {   18, 11567,   418}, 
    {   18, 13864,   105}, {   18, 16368,     0}, {   19,    77, 14203}, 
    {   19,   313, 12167}, {   19,   705, 10288}, {   19,  1256,  8566}, 
    {   19,  1963,  7002}, {   19,  2829,  5596}, {   19,  3851,  4347}, 
    {   19,  5032,  3256}, {   19,  6369,  2322}, {   19,  7865,  1545}, 
    {   19,  9517,   926}, {   19, 11327,   465}, {   19, 13295,   161}, 
    {   19, 15420,    14}, {   20,    19, 15277}, {   20,   147, 13427}, 
    {   20,   394, 11696}, {   20,   760, 10085}, {   20,  1246,  8593}, 
    {   20,  1851,  7220}, {   20,  2575,  5967}, {   20,  3419,  4833}, 
    {   20,  4382,  3819}, {   20,  5464,  2924}, {   20,  6666,  2148}, 
    {   20,  7987,  1491}, {   20,  9428,   954}, {   20, 10988,   537}, 
    {   20, 12667,   238}, {   20, 14466,    59}
  };
*/
/* filter with 26 frequency bins (to be used with 16 dimension mfcc) : */

static const mel_filter_type mel_filter26[ WINDOW_LEN / 2 ] = {

{     0,     0, 16384 },{     0,  7168,  9216 },{     0, 14336,  2048 },
{     1,  5119, 11264 },{     1, 12287,  4096 },{     2,  3071, 13312 },
{     2, 10239,  6144 },{     3,  1023, 15360 },{     3,  8191,  8192 },
{     3, 15359,  1024 },{     4,  6143, 10240 },{     4, 13312,  3072 },
{     5,  4096, 12288 },{     5, 11263,  5120 },{     6,  2047, 14336 },
{     6,  9215,  7168 },{     7,     0, 16384 },{     7,  7167,  9216 },
{     7, 14335,  2048 },{     8,  5119, 11264 },{     8, 12288,  4095 },
{     9,  3072, 13311 },{     9, 10240,  6143 },{    10,  1024, 15359 },
{    10,  8192,  8192 },{    10, 15359,  1024 },{    11,  6143, 10240 },
{    11, 13311,  3072 },{    12,  4095, 12288 },{    12, 11264,  5119 },
{    13,  2048, 14335 },{    13,  9216,  7167 },{    14,     0, 16384 },
{    14,  4549, 11834 },{    14,  9099,  7284 },{    14, 13649,  2734 },
{    15,  1631, 14752 },{    15,  5721, 10662 },{    15,  9811,  6572 },
{    15, 13900,  2483 },{    16,  1443, 14940 },{    16,  5119, 11264 },
{    16,  8795,  7588 },{    16, 12471,  3912 },{    16, 16147,   236 },
{    17,  3091, 13292 },{    17,  6395,  9988 },{    17,  9700,  6683 },
{    17, 13004,  3379 },{    17, 16308,    75 },{    18,  2901, 13482 },
{    18,  5871, 10512 },{    18,  8841,  7542 },{    18, 11811,  4572 },
{    18, 14781,  1602 },{    19,  1229, 15154 },{    19,  3898, 12485 },
{    19,  6568,  9815 },{    19,  9237,  7146 },{    19, 11907,  4476 },
{    19, 14576,  1807 },{    20,   775, 15608 },{    20,  3174, 13209 },
{    20,  5574, 10809 },{    20,  7973,  8410 },{    20, 10373,  6010 },
{    20, 12772,  3611 },{    20, 15172,  1211 },{    21,  1067, 15316 },
{    21,  3224, 13159 },{    21,  5381, 11002 },{    21,  7538,  8845 },
{    21,  9694,  6689 },{    21, 11851,  4532 },{    21, 14008,  2375 },
{    21, 16165,   218 },{    22,  1741, 14642 },{    22,  3680, 12703 },
{    22,  5619, 10764 },{    22,  7557,  8826 },{    22,  9496,  6887 },
{    22, 11435,  4948 },{    22, 13373,  3010 },{    22, 15312,  1071 },
{    23,   779, 15604 },{    23,  2521, 13862 },{    23,  4264, 12119 },
{    23,  6007, 10376 },{    23,  7749,  8634 },{    23,  9492,  6891 },
{    23, 11234,  5149 },{    23, 12977,  3406 },{    23, 14719,  1664 },
{    24,    70, 16313 },{    24,  1636, 14747 },{    24,  3203, 13180 },
{    24,  4769, 11614 },{    24,  6335, 10048 },{    24,  7901,  8482 },
{    24,  9468,  6915 },{    24, 11034,  5349 },{    24, 12600,  3783 },
{    24, 14167,  2216 },{    24, 15733,   650 },{    25,   823, 15560 },
{    25,  2230, 14153 },{    25,  3638, 12745 },{    25,  5046, 11337 },
{    25,  6454,  9929 },{    25,  7862,  8521 },{    25,  9270,  7113 },
{    25, 10678,  5705 },{    25, 12085,  4298 },{    25, 13493,  2890 },
{    25, 14901,  1482 },{    25, 16309,    74 },{    26,  1198, 15185 },
{    26,  2464, 13919 },{    26,  3729, 12654 },{    26,  4994, 11389 },
{    26,  6260, 10123 },{    26,  7525,  8858 },{    26,  8791,  7592 },
{    26, 10056,  6327 },{    26, 11322,  5061 },{    26, 12587,  3796 },
{    26, 13853,  2530 },{    26, 15118,  1265 }
};

  /*

{     0,     0, 16384 },{     0,  3136,  5184 },{     0, 12544,   256 },
{     1,  1599,  7744 },{     1,  9215,  1024 },{     2,   575, 10816 },
{     2,  6399,  2304 },{     3,    63, 14400 },{     3,  4095,  4096 },
{     3, 14399,    64 },{     4,  2303,  6400 },{     4, 10816,   576 },
{     5,  1024,  9216 },{     5,  7743,  1600 },{     6,   255, 12544 },
{     6,  5183,  3136 },{     7,     0, 16384 },{     7,  3135,  5184 },
{     7, 12543,   256 },{     8,  1599,  7744 },{     8,  9216,  1023 },
{     9,   576, 10815 },{     9,  6400,  2303 },{    10,    64, 14399 },
{    10,  4096,  4096 },{    10, 14399,    64 },{    11,  2303,  6400 },
{    11, 10815,   576 },{    12,  1023,  9216 },{    12,  7744,  1599 },
{    13,   256, 12543 },{    13,  5184,  3135 },{    14,     0, 16384 },
{    14,  1263,  8547 },{    14,  5053,  3238 },{    14, 11371,   456 },
{    15,   162, 13283 },{    15,  1997,  6939 },{    15,  5874,  2636 },
{    15, 11793,   376 },{    16,   127, 13623 },{    16,  1599,  7744 },
{    16,  4721,  3514 },{    16,  9493,   934 },{    16, 15914,     3 },
{    17,   583, 10783 },{    17,  2496,  6089 },{    17,  5742,  2726 },
{    17, 10321,   697 },{    17, 16232,     0 },{    18,   513, 11094 },
{    18,  2104,  6744 },{    18,  4771,  3472 },{    18,  8515,  1275 },
{    18, 13335,   156 },{    19,    92, 14017 },{    19,   927,  9514 },
{    19,  2633,  5880 },{    19,  5208,  3116 },{    19,  8653,  1223 },
{    19, 12969,   199 },{    20,    36, 14870 },{    20,   615, 10649 },
{    20,  1896,  7132 },{    20,  3880,  4317 },{    20,  6567,  2205 },
{    20,  9957,   795 },{    20, 14050,    89 },{    21,    69, 14318 },
{    21,   634, 10569 },{    21,  1767,  7388 },{    21,  3468,  4776 },
{    21,  5736,  2731 },{    21,  8573,  1253 },{    21, 11977,   344 },
{    21, 15949,     2 },{    22,   185, 13085 },{    22,   826,  9849 },
{    22,  1927,  7072 },{    22,  3486,  4754 },{    22,  5504,  2895 },
{    22,  7981,  1494 },{    22, 10916,   553 },{    22, 14311,    70 },
{    23,    37, 14862 },{    23,   388, 11728 },{    23,  1109,  8964 },
{    23,  2202,  6572 },{    23,  3665,  4550 },{    23,  5499,  2898 },
{    23,  7703,  1618 },{    23, 10278,   708 },{    23, 13224,   169 },
{    24,     0, 16243 },{    24,   163, 13274 },{    24,   626, 10604 },
{    24,  1388,  8233 },{    24,  2449,  6162 },{    24,  3811,  4391 },
{    24,  5471,  2919 },{    24,  7431,  1746 },{    24,  9691,   873 },
{    24, 12250,   299 },{    24, 15108,    25 },{    25,    41, 14779 },
{    25,   303, 12225 },{    25,   808,  9914 },{    25,  1554,  7845 },
{    25,  2542,  6017 },{    25,  3773,  4432 },{    25,  5245,  3088 },
{    25,  6959,  1987 },{    25,  8915,  1127 },{    25, 11113,   509 },
{    25, 13553,   134 },{    25, 16235,     0 },{    26,    87, 14074 },
{    26,   370, 11826 },{    26,   848,  9773 },{    26,  1522,  7916 },
{    26,  2392,  6255 },{    26,  3456,  4789 },{    26,  4717,  3518 },
{    26,  6173,  2443 },{    26,  7824,  1563 },{    26,  9671,   879 },
{    26, 11713,   390 },{    26, 13951,    97 }, };

*/

/*
** COS transformation, to be used with (8, 10) x 20 transformations
*/

static const short cosxfm20[ MAX_DIM_MFCC10 ][ N_FILTER20 ] = /* cosine transform */
{

{32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 
 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,},


{32665, 31861, 30272, 27938, 24916, 21280, 17120, 12539, 7649, 2570, 
 -2570, -7649, -12539, -17120, -21280, -24916, -27938, -30272, -31861, -32665, },


{32363, 29195, 23169, 14875, 5125, -5125, -14875, -23169, -29195, -32363, 
 -32363, -29195, -23169, -14875, -5125, 5125, 14875, 23169, 29195, 32363,} ,

{31861, 24916, 12539, -2570, -17120, -27938, -32665, -30272, -21280, -7649, 
 7649, 21280, 30272, 32665, 27938, 17120, 2570, -12539, -24916, -31861, },

{31163, 19259, 0, -19259, -31163, -31163, -19259, 0, 19259, 31163, 
 31163, 19259, 0, -19259, -31163, -31163, -19259, 0, 19259, 31163, },

{30272, 12539, -12539, -30272, -30272, -12539, 12539, 30272, 30272, 12539, 
 -12539, -30272, -30272, -12539, 12539, 30272, 30272, 12539, -12539, -30272, },

{29195, 5125, -23169, -32363, -14875, 14875, 32363, 23169, -5125, -29195, 
 -29195, -5125, 23169, 32363, 14875, -14875, -32363, -23169, 5125, 29195, },

{27938, -2570, -30272, -24916, 7649, 31861, 21280, -12539, -32665, -17120, 
 17120, 32665, 12539, -21280, -31861, -7649, 24916, 30272, 2570, -27938, },

{26509, -10125, -32766, -10125, 26509, 26509, -10125, -32766, -10125, 26509, 
 26509, -10125, -32766, -10125, 26509, 26509, -10125, -32766, -10125, 26509, },

{24916, -17120, -30272, 7649, 32665, 2570, -31861, -12539, 27938, 21280, 
 -21280, -27938, 12539, 31861, -2570, -32665, -7649, 30272, 17120, -24916}
};


/*
** COS transformation, to be used with 16 x 26 transformations
*/
static const short cosxfm26[ MAX_DIM_MFCC16][ N_FILTER26 ] =  /* cosine transform */
{
{
 32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767, 
 32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767, },
{
 32707,  32230,  31283,  29880,  28041,  25793,  23169,  20208,  16951,  13447,   9748,   5906,   1978, 
 -1978,  -5906,  -9748, -13447, -16951, -20208, -23169, -25793, -28041, -29880, -31283, -32230, -32707, },
{
 32528,  30637,  26966,  21728,  15227,   7841,      0,  -7841, -15227, -21728, -26966, -30637, -32528, 
-32528, -30637, -26966, -21728, -15227,  -7841,      0,   7841,  15227,  21728,  26966,  30637,  32528, },
{
 32230,  28041,  20208,   9748,  -1978, -13447, -23169, -29880, -32707, -31283, -25793, -16951,  -5906, 
  5906,  16951,  25793,  31283,  32707,  29880,  23169,  13448,   1978,  -9748, -20208, -28041, -32230, },
{
 31814,  24526,  11619,  -3949, -18613, -29013, -32766, -29013, -18613,  -3949,  11619,  24526,  31814, 
 31814,  24526,  11619,  -3949, -18613, -29013, -32766, -29013, -18613,  -3949,  11619,  24526,  31814, },
{
 31283,  20208,   1978, -16951, -29880, -32230, -23169,  -5906,  13447,  28041,  32707,  25793,   9748, 
 -9748, -25793, -32707, -28041, -13448,   5906,  23169,  32230,  29880,  16951,  -1978, -20208, -31283, },
{
 30637,  15227,  -7841, -26966, -32528, -21728,      0,  21728,  32528,  26966,   7841, -15227, -30637, 
-30637, -15227,   7841,  26966,  32528,  21728,      0, -21728, -32528, -26966,  -7841,  15227,  30637, },
{
 29880,   9748, -16951, -32230, -25793,  -1978,  23169,  32707,  20208,  -5906, -28041, -31283, -13448, 
 13447,  31283,  28041,   5906, -20208, -32707, -23169,   1978,  25793,  32230,  16951,  -9748, -29880, },
{
 29013,   3949, -24526, -31814, -11619,  18613,  32766,  18613, -11619, -31814, -24526,   3949,  29013, 
 29013,   3949, -24526, -31814, -11619,  18613,  32766,  18613, -11619, -31814, -24526,   3949,  29013, },
{
 28041,  -1978, -29880, -25793,   5906,  31283,  23169,  -9748, -32230, -20208,  13447,  32707,  16951, 
-16951, -32707, -13448,  20208,  32230,   9748, -23169, -31283,  -5906,  25793,  29880,   1978, -28041, },
{
 26966,  -7841, -32528, -15227,  21728,  30637,      0, -30637, -21728,  15227,  32528,   7841, -26966, 
-26966,   7841,  32528,  15227, -21728, -30637,      0,  30637,  21728, -15227, -32528,  -7841,  26966, },
{
 25793, -13447, -32230,  -1978,  31283,  16951, -23169, -28041,   9748,  32707,   5906, -29880, -20208, 
 20208,  29880,  -5906, -32707,  -9748,  28041,  23169, -16951, -31283,   1978,  32230,  13448, -25793, },
{
 24526, -18613, -29013,  11619,  31814,  -3949, -32766,  -3949,  31814,  11619, -29013, -18613,  24526, 
 24526, -18613, -29013,  11619,  31814,  -3949, -32766,  -3949,  31814,  11619, -29013, -18613,  24526, },
{
 23169, -23169, -23169,  23169,  23169, -23169, -23169,  23169,  23169, -23169, -23169,  23169,  23169, 
-23169, -23169,  23169,  23169, -23169, -23169,  23169,  23169, -23169, -23169,  23169,  23169, -23169, },
{
 21728, -26966, -15227,  30637,   7841, -32528,      0,  32528,  -7841, -30637,  15227,  26966, -21728, 
-21728,  26966,  15227, -30637,  -7841,  32528,      0, -32528,   7841,  30637, -15227, -26966,  21728, },
{
 20208, -29880,  -5906,  32707,  -9748, -28041,  23169,  16951, -31283,  -1978,  32230, -13447, -25793, 
 25793,  13448, -32230,   1978,  31283, -16951, -23169,  28041,   9748, -32707,   5906,  29880, -20208, },

};



const short w_real[ WINDOW_LEN / 2 ] =      /* for FFT */
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

const short hw[ WINDOW_LEN ] =       /* Hamming window */
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
 






/*--------------------------------*/
/* These are now moved to mfcc_f_user.h */

//typedef struct 
//{
//  short norm0;  /* preemphasis */
//  short norm1;  /* Hamming */
//  short norm2;  /* power spectrum */
//} NormType;

/* mel filter, mel scale, triangular square */
/*
typedef struct {   
  short num;
  short x1;
  short x2;
} mel_filter_type;
*/

/*--------------------------------*/
/* Now in tiesrcommonmacros.h */
/* right shift with rounding: */
/*
#define LONG_RIGHT_SHIFT_N(_val_,_n_) (((_val_) + ((long)1<<((_n_)-1))) >>(_n_))
*/

/*--------------------------------*/
/* Now in mfcc_f_user.h */
/*
extern short log_polyfit(long value, short compensate);
// extern long pow_polyfit( short value );


extern void mfcc_a_window(short *sig, short *mfcc, short *log_mel_energy, const short n_mfcc, const short n_filter, 
			  const mel_filter_type mel_filter[], const short *cosxfm[  MAX_DIM_MFCC16 ],
                          short [], const short [], NormType *, short * );

extern void 
cos_transform(short *mfcc, short *mel_energy, const short *scale_p2, 
	      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ]);
extern void
inverse_cos_transform(short *log_mel_energy, short *mfcc, const short scale_p2[], 
		      const short n_mfcc, const short n_filter,  const short *cosxfm[  MAX_DIM_MFCC16 ]);
*/
//extern void 
//pmc(short *sp_mfcc_o,     /* original non-PMC MFCC mean */
//    short *noise_log_ps, /* noise log mel energy */
//    short *pmc_mfcc,     /* PMC-compensated MFCC mean */
//    short *chn,          /* channel estimate  */
//    short jac,           /* jac/pmc switch  */
//    short n_mfcc, short n_filter, const short *mu_scale_p2, short *scale_mu,
//    const short *cosxfm[  MAX_DIM_MFCC16 ]);

/*
extern void 
log_spectral_compensation(short sp_log_ps[], short reg_sp_log_ps[],  short pmc_log_ps[],  short reg_pmc_log_ps[], 
			  const short noise_log_ps[], const short chn[], short jac, short *log_df, short n_filter);
*/

#endif
