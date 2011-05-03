/*=======================================================================

 *
 * winlen.h
 *
 * Header defining audio collection parameters, including frame size,
 * window size and sample rate.
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

 This header defines parameters for collecting audio samples and how to
 put them in frames.
======================================================================*/

#ifndef _WINLEN_H
#define _WINLEN_H

/* Number of samples making up a frame window. This is the number of samples
 that will be windowed by the Hamming window, and the number of samples input
 to the FFT during MFCC computation. It must be 256 or 512. */

// #define WINDOW_LEN 256
#define WINDOW_LEN 512

#if WINDOW_LEN == 256
#define WINDOW_PWR 8
#elif WINDOW_LEN == 512
#define WINDOW_PWR 9
#endif

/* Number of samples in a frame of data that user must copy to buffer */

// #define   FRAME_LEN    160
#define FRAME_LEN 480


/* A/D Sampling rate */

// #define SAM_FREQ 8000
#define SAM_FREQ 24000

#endif /* _WINLEN_H */
