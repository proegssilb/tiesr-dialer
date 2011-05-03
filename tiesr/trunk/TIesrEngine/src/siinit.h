/*=======================================================================

 *
 * siinit.h
 *
 * Header for implementation parameters for TIesr initialization.
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

 This header file contains macros that are used in specifying
 parameters for speaker-independent recognition.  Specifically,
 the parameters define the memory sizes used during search.

======================================================================*/

#ifndef SIINIT_H
#define SIINIT_H


/* macros for search space */
#define BEAM_R   9851
#define STATE_R  9209
#define TIME_Z   200 /* words, MIN 118 */

#endif
