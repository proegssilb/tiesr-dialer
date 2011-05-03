/*=======================================================================
 volume.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


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

#ifdef USE_AUDIO


#include "status.h"
#include "gmhmm_type.h"
#include "volume_user.h"

/*
** check volume range 
*/

/* GMHMM_SI_API */
TIESRENGINECOREAPI_API void SetTIesrVolRange(gmhmm_type *gvv, unsigned short low_v, unsigned short high_v)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  gv->low_vol_limit = low_v; 
  gv->high_vol_limit = high_v;
}

/* GMHMM_SI_API */
TIESRENGINECOREAPI_API void GetTIesrVolRange(gmhmm_type* gvv, unsigned short *low_v, unsigned short *high_v)
{
  gmhmm_type *gv = (gmhmm_type *) gvv;
  *low_v  = gv->low_vol_limit;
  *high_v = gv->high_vol_limit;
}


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

