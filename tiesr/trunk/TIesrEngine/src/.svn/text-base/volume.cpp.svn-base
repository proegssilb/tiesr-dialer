/*=======================================================================

 *
 * volume.cpp
 *
 * Crude volume measure.
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

 Simple volume measure based on maximum signal swing.

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "volume.h"
*/

#include "tiesr_config.h"




#include "status.h"
#include "gmhmm_type.h"
#include "volume_user.h"

/*
 ** check volume range
 */

/* GMHMM_SI_API */
TIESRENGINECOREAPI_API void SetTIesrVolRange( gmhmm_type *gvv, unsigned short low_v, unsigned short high_v )
{
#ifdef USE_AUDIO
   gmhmm_type *gv = (gmhmm_type *) gvv;
   gv->low_vol_limit = low_v;
   gv->high_vol_limit = high_v;
#endif
}
/* GMHMM_SI_API */
TIESRENGINECOREAPI_API void GetTIesrVolRange( gmhmm_type* gvv, unsigned short *low_v, unsigned short *high_v )
{
#ifdef USE_AUDIO
   gmhmm_type *gv = (gmhmm_type *) gvv;
   *low_v = gv->low_vol_limit;
   *high_v = gv->high_vol_limit;
#else
   *low_v = 0;
   *high_v = 0;
#endif
}

#ifdef USE_AUDIO

TIESRENGINECOREAPI_API short set_volume_flag(gmhmm_type *gv)
{
  short volume_flag;
  
  //   short volume = (amplitude_max - amplitude_min) >> 1;
     unsigned short volume = (unsigned short)(((long)gv->amplitude_max - (long)gv->amplitude_min) >> 1);   
  
     if (volume < gv->low_vol_limit) {
        PRT_ERR(printf("volume too low (%d)!\n", volume));
        volume_flag = TIesrEngineVolumeLow;
     }
     else {
       if (volume >  gv->high_vol_limit ) { 
        PRT_ERR(printf("volume too high (%d)!\n", volume));
        volume_flag = TIesrEngineVolumeHigh;
       }
       else {
        PRT_ERR(printf("volume (%d) OK.\n", volume));
        volume_flag = TIesrEngineVolumeOK;
       }
     }
     return volume_flag;
}

#endif

