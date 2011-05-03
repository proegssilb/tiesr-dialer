/*=======================================================================

 *
 * uttdet.h
 *
 * Header for implementation parameters of utterance detection.
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

 This header file contains the utterance detector parameters.
 
======================================================================*/

#ifndef _UTTDET_H
#define _UTTDET_H

#include "winlen.h"
#include "tiesrcommonmacros.h"

/*--------------------------------
  Utterance detector default parameters
  --------------------------------*/
static const int TH_NOISE_FLR_DEFAULT = 0;

static const int TH_SPEECH_DELTA_DEFAULT = 10;

/* min dB level to be considered as speech */
static const int TH_MIN_SPEECH_DB_DEFAULT = 50;

/* larger (e.g. 5)  -> more robust to  noise pulses */
static const int TH_MIN_BEG_FRM_DEFAULT = 5;

/* smaller (.e.g. 30) --> quicker latency  time, but
 pauses will be interpreted as end of speech */
#ifdef USE_AUDIO
static const int TH_MIN_END_FRM_DEFAULT = 35;
#else
static const int TH_MIN_END_FRM_DEFAULT = 70;
#endif

/*--------------------------------
  Constants by define
  --------------------------------*/

/* 400 Hz lower cut-off index for frequency shaping.
 Typically 256 * 400/8000 = 13 for 256 sample window and 8kHz sampling */
#define  L_CUT  ( WINDOW_LEN * 400 / SAM_FREQ )

/* 2700 Hz upper cut-off index for frequency shaping.
 Typically 256* 2700/8000 = 86 for 256 sample window and 8kHz sampling */
#define  H_CUT  ( WINDOW_LEN * 2700 / SAM_FREQ )


/* For normal 256 sample FFT set to 22938 => 0.70, Q 15 */
# if ( WINDOW_LEN == 512 )
#define L_RATE 27416
#else
#define L_RATE 22938
#endif

/* For normal 256 sample FFT set to 27853 => 0.85, Q 15 */
#if ( WINDOW_LEN == 512 )
#define H_RATE 30211
#else
#define H_RATE 27853
#endif


/* Lower correlation index for search for correlation peak.
 Corresponds to 400 Hz  */
#define  LEND  ( SAM_FREQ / 400 )

/* Higher correlation index for searching for correlation peak.
 Corresponds to 67 Hz or highest index available corresponding to lowest
 frequency available. */
#define  HEND  ( MIN( ( SAM_FREQ / 67 ), ( (WINDOW_LEN>>1) - 1) ) )

/*  0.5, Q 15 */
#define  SMOOTH 16384

#define  AC_SCALE       64         /* auto correlation scale */
//#define  SPEECH_DELTA   10    
#define  INI_NOISE      70
#define  INI_SPEECH    (INI_NOISE /* +SPEECH_DELTA */)
//#define  MIN_SPEECH_DB  50  /* min dB level to be considered as speech */

//#ifdef USE_AUDIO
//#define  MIN_END_FRM    35  /* smaller (.e.g. 30) --> quicker latency  time */
//#else
//#define  MIN_END_FRM    70  /* file based, handling longer pausses */
//#endif

#define  SLUPTC         12055      /* exp(-1) = 0.3679, Q 15 */
#define  SLDNTC         32119     /* exp(-1/50) = .980198 Q 15 */
#define  NLUPTC         SLDNTC 
#define  NLDNTC         SLUPTC

//#define  SLDNTC         31693      /* exp(-1/30) = 0.9672, Q 15 */
//#define  NLUPTC         32440      /* exp(-1/100) = 0.99, Q 15 */
// #define  NLDNTC         21964      /* exp(-2/5) = 0.6703, Q 15 */

//#define  MIN_BEG_FRM   5  /* larger (e.g. 5)  -> more robust to  noise pulses */


/*
extern void init_uttdet(gmhmm_type *);
extern void set_default_uttdet(gmhmm_type *);

extern void compute_uttdet(short *power_spectrum, short cnt,short frm_cnt, short *, gmhmm_type *, NormType *);

extern void noise_subs(short *ps_signal, short norm, short *ps_noisy_smoothed, short *noise_floor,
                       ushort, short *);

extern short update_level(short new_val, short old, short uptc, short dntc);
extern void init_spect_sub(short *);

 */

/*
 ** un scaled: W_B in Q 15
 */
/*
#define SMOOTHING(A, W_B, B) ((long) ( 32767 - W_B ) * A + (long) W_B * B)
 */

#endif
