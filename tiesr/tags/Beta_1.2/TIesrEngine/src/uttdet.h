/*=======================================================================
 uttdet.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header file contains the utterance detector parameters.
 
======================================================================*/

#ifndef _UTTDET_H
#define _UTTDET_H

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

// #define  L_CUT          19         /* 256 * 600 / 8000 */
//#define  H_CUT          58         /* 256 * 1800 / 8000 */

//#define  L_CUT  6     /* 256 * 200/8000 */

#define  L_CUT  13     /* 256 * 400/8000 */
#define  H_CUT  86    /* 256*2700/8000 */


#define  L_RATE         22938      /* 0.70, Q 15 */
#define  H_RATE         27853      /* 0.85, Q 15 */
#define  LEND           20         /* 400 Hz */
#define  HEND           120       /* 75 Hz (66Hz?) */

#define  SMOOTH        16384  /* 13107 (0.4) */      /* 0.5, Q 15 */

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
