/*=======================================================================

 *
 * engine_sireco_init.cpp
 *
 * Speaker independent recognizer API object.
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


 This function defines the API that is exposed to the user of the
 TIesr SI recognizer.

======================================================================*/

/*
#include "mfcc_f.h"
#include "tiesr_engine_api_sireco.h"
#include "gmhmm.h"
#include "jacheader.h"
#include "gmhmm_si_api.h"
*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"

#include "tiesr_engine_api_sireco.h"
#include "gmhmm_si_api.h"
#include "gmhmm_type_common_user.h"
#include "volume_user.h"
#include "uttdet_user.h"
#include "jac-estm_user.h"
#include "jac_one_user.h"


#ifdef USE_CONFIDENCE
#include "confidence_user.h"
#endif


#if defined (WIN32) || defined (WINCE)
/*---------------------------------------------------------
DllMain

Windows DLL entry point if Windows OS.
----------------------------------------------------------*/


BOOL APIENTRY DllMain( HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif


/*---------------------------------------------------------
TIesrEngineOpen

This function initializes pointers to all functions in the
TIesrEngineSIRECOType structure.
-----------------------------------------------------------*/
TIESRENGINESIAPI_API
void TIesrEngineOpen (TIesrEngineSIRECOType *tiesr)
{
  tiesr->OpenASR           = OpenASR; 
  tiesr->CloseASR          = CloseASR; 
  tiesr->OpenSearchEngine  = OpenSearchEngine; 
  tiesr->CallSearchEngine  = CallSearchEngine; 
  tiesr->CloseSearchEngine = CloseSearchEngine; 
  tiesr->SpeechDetected    = SpeechDetected; 
  tiesr->SpeechEnded       = SpeechEnded; 


  tiesr->JAC_load   = JAC_load;
  tiesr->JAC_save  = JAC_save;
  tiesr->JAC_clear  = JAC_clear;
  tiesr->JAC_update = JAC_update;

  tiesr->GetFrameCount   = GetFrameCount; 
  tiesr->GetTotalScore = GetTotalScore;

  tiesr->GetCycleCount   = GetCycleCount; 
  tiesr->GetAnswerIndex  = GetAnswerIndex; 
  tiesr->GetAnswerWord   = GetAnswerWord; 
  tiesr->GetAnswerCount  = GetAnswerCount; 
  tiesr->GetVolumeStatus = GetVolumeStatus; 
  tiesr->GetSearchMemorySize = GetSearchMemorySize;  
  tiesr->GetMaxFrames = GetMaxFrames;
//#ifdef USE_CONFIDENCE
  tiesr->GetConfidenceScore = GetConfidenceScore; 
//#ifdef USE_CONF_DRV_ADP
  tiesr->SetTIesrConfidenceAdaptation = SetTIesrCM;
  tiesr->GetTIesrConfidenceAdaptation = GetTIesrCM; 
//#endif
//#endif

#ifdef REALTIMEGUARD
  tiesr->getRealTimeGuardTh=GetRealTimeGuardTh;
  tiesr->getGetRealTGuard=GetRealTGuard;
  tiesr->setRealTimeGuardTh=SetRealTimeGuardTh;
  tiesr->setGetRealTGuard=SetRealTGuard;
#endif

// Defines should not change the compatibility of 
// TIesr processing that is optional
//#ifdef USE_GAUSS_SELECT
  tiesr->SetTIesrGaussSelection = SetTIesrGaussSel;
  tiesr->GetTIesrGaussSelection = GetTIesrGaussSel; 
//#endif

//#if defined(USE_ORM_VAD)||defined(USE_ONLINE_REF)
  tiesr->SetTIesrOnlineReferenceModel = SetTIesrORM; 
  tiesr->GetTIesrOnlineReferenceModel = GetTIesrORM; 
//#endif

//#ifdef USE_SNR_SS
  tiesr->SetTIesrNoiseSubtraction = SetTIesrSS; 
  tiesr->GetTIesrNoiseSubtraction = GetTIesrSS; 
//#endif

  tiesr->SetTIesrPrune        = SetTIesrPrune;
  tiesr->SetTIesrTransiWeight = SetTIesrTransiWeight;
  tiesr->SetTIesrSAD          = SetTIesrSAD; 
  tiesr->SetTIesrJacRate      = SetTIesrJacRate; 
  tiesr->SetTIesrJAC          = SetTIesrJAC; 
  tiesr->SetTIesrSBC          = SetTIesrSBC; 

#ifdef USE_AUDIO
  tiesr->SetTIesrVolRange     = SetTIesrVolRange; 
#endif

  tiesr->GetTIesrPrune        = GetTIesrPrune;
  tiesr->GetTIesrTransiWeight = GetTIesrTransiWeight;
  tiesr->GetTIesrSAD          = GetTIesrSAD; 
  tiesr->GetTIesrJacRate      = GetTIesrJacRate; 
  tiesr->GetTIesrJAC          = GetTIesrJAC; 
  tiesr->GetTIesrSBC          = GetTIesrSBC;
#ifdef USE_AUDIO
  tiesr->GetTIesrVolRange     = GetTIesrVolRange; 
#endif

#ifdef _MONOPHONE_NET
  tiesr->GetBestWordStartAt = GetBestWordStartAt;
#endif

  tiesr->GetNumNbests = GetNumNbests;
  tiesr->SetTIesrNBest = SetTIesrNBest;
  tiesr->GetTIesrNBest = GetTIesrNBest;

#ifdef USE_NBEST
  tiesr->GetBestWordStartAt = GetBestWordStartAt;
#endif

  return;
}


/*--------------------------------------------------------------
TIesrEngineClose

This function performs any necessary closing operations.  None are
used at this time.
----------------------------------------------------------------*/
TIESRENGINESIAPI_API
void TIesrEngineClose(TIesrEngineSIRECOType *)

{
  return;
}
