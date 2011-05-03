/*=======================================================================
 tiesr_engine_api_sireco.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header defines the interface of the TIesr SI recognizer.

======================================================================*/

#ifndef _TIESR_ENGINE_API_SIRECO_H
#define _TIESR_ENGINE_API_SIRECO_H

/* Needed if Windows in order to export DLL functions */
#if defined (WIN32) || defined (WINCE)

#ifndef TIESRENGINESIAPI_API

// The following ifdef block is the standard way of creating macros which
// make exporting from a DLL simpler. All files within this DLL are compiled
// with the TIESRENGINESIAPI_EXPORTS symbol defined on the command line.
// This symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see
// TIESRENGINESIAPI_API functions as being imported from a DLL,
// whereas this DLL sees symbols defined with this macro as being exported.
#ifdef TIESRENGINESIAPI_EXPORTS
#define TIESRENGINESIAPI_API __declspec(dllexport)
#else
#define TIESRENGINESIAPI_API __declspec(dllimport)
#endif

#endif

#else

#ifndef TIESRENGINESIAPI_API
#define TIESRENGINESIAPI_API
#endif

#endif


/* 
** Data structure specifying TIESR SI algorithms.
*/

/*
#include "status.h"
#include "winlen.h"
#include "use_audio.h"
*/

#include "tiesr_config.h"
#include "status.h"

/* This is only included here, since the user must know how many
   samples per frame to supply to the recognizer. */
#include "winlen.h"

/*--------------------------------*/
/*  
    The TIesrEngineSIRECOType structure defines an instance of the
    TIesr SI recognizer.  The user must instantiate and delete an
    instance by opening or closing the instance.
*/

/*
** scope limitation: API function visible only within variable
*/

typedef struct gmhmm_type* TIesr_t;
//typedef const struct  gmhmm_struct* cTIesr_t;
typedef struct gmhmm_type const* cTIesr_t;


typedef struct TIesrEngineSIRECO {

      //
      // Doers (functions)
      //
      TIesrEngineStatusType (*OpenASR)(char [], int, TIesr_t );
      void (*CloseASR)(TIesr_t);
      TIesrEngineStatusType (*OpenSearchEngine)(TIesr_t); 
      TIesrEngineStatusType (*CallSearchEngine)(short [], TIesr_t, unsigned int);
      TIesrEngineStatusType (*CloseSearchEngine)(TIesrEngineStatusType, TIesr_t );

      short (*SpeechDetected)( TIesr_t);
      short (*SpeechEnded)( TIesr_t);

      TIesrEngineJACStatusType (*JAC_load)(const char *bias_file, 
					   TIesr_t, 
					   const char* pNetDir,
					   const char* pPhoneticClusterTree);

      TIesrEngineJACStatusType (*JAC_save)(const char *bias_file, TIesr_t);

      TIesrEngineJACStatusType (*JAC_clear)(TIesr_t, 
					    const char* aModelDir, 
					    const char* aTreeFile); 

      TIesrEngineJACStatusType (*JAC_update)(TIesr_t);


      //
      // Getters (queries)
      //
      short (*GetFrameCount)( cTIesr_t);  
      long (*GetTotalScore)(cTIesr_t);
      unsigned short  (*GetCycleCount)( cTIesr_t); 
      const unsigned short  *(*GetAnswerIndex)(cTIesr_t, short);
      const char *(*GetAnswerWord)(unsigned short, cTIesr_t, short);
      unsigned short (*GetAnswerCount)(cTIesr_t, short); 
  /* setup N-best */
  void (*SetTIesrNBest)(TIesr_t, short); 
  void (*GetTIesrNBest)(TIesr_t , short*);
  unsigned short  (*GetNumNbests)(cTIesr_t); 

//#ifdef USE_CONFIDENCE
      short (*GetConfidenceScore)(cTIesr_t);
//#ifdef USE_CONF_DRV_ADP
/* 
   1) setup paramters for confidence-driven adaptation 
confidence score of an utterance has to be larger than a threshold to have adaptation 
     default adaptation_cm_threshold = -200 
   2) set the threshold of nbest to declare low confidence score 
   default is 20
*/     
  void (*SetTIesrConfidenceAdaptation)(short, short, TIesr_t); 
  void (*GetTIesrConfidenceAdaptation)(short*, short*, TIesr_t);
//#endif
//#endif

// Defines should not change the compatibility
// of TIesr processing that is optional      
//#ifdef USE_GAUSS_SELECT
  /* setup parameters for Gaussian selection 
     default percentage_of_core = 16384, 
     percentage_of_inter = 0 */
  void (*SetTIesrGaussSelection)(TIesr_t, unsigned short , unsigned short );
  void (*GetTIesrGaussSelection)(cTIesr_t , unsigned short * , unsigned short *); 
//#endif

//#if defined(USE_ORM_VAD)||defined(USE_ONLINE_REF)
  /* setup parameters for on-line reference modeling method
     default setup 
     number_of_frame_to_generate_ORM = 9
     threshold_noise_level = 12000
     th_llr_low_noise = 150,
     th_llr_high_noise = 80,
     number_of_frame_with_LLR_below_threshold = 80
     percentage_of_the_maximum_LLR_to_get_threshold = 3277
     starting_frame = 30 ) 
     th_nbr_frm_after_llr_peak = 150 
  */
  void (*SetTIesrOnlineReferenceModel)(short, short ,
				       short, short , 
				       short, unsigned short , short, short, TIesr_t);
  void (*GetTIesrOnlineReferenceModel)(short*, short*, short*, short*, 
				       short*, unsigned short*, short*, short *, TIesr_t);
//#endif

// Options should not redfine the user interface
//#ifdef USE_SNR_SS
  /* setup parameters for noise subtraction 
     default setup
     alpha = 29491
     beta = 3 */
  void (*SetTIesrNoiseSubtraction)(unsigned short, short, TIesr_t);
  void (*GetTIesrNoiseSubtraction)(unsigned short*, short*, TIesr_t);
//#endif


#ifdef REALTIMEGUARD

  
  void (*getRealTimeGuardTh)(short*, TIesr_t); 
  void (*getGetRealTGuard)(short*, TIesr_t); 
  
  void (*setRealTimeGuardTh) (short, TIesr_t);
  void (*setGetRealTGuard) (short, TIesr_t);
  
#endif

      short (*GetVolumeStatus)(cTIesr_t);   

      //  void (*GetSearchInfo)(void *);
      unsigned short (*GetSearchMemorySize)(TIesr_t);
      unsigned short (*GetMaxFrames)(TIesr_t);

      //
      // Setters (modifiers)
      //
      void (*SetTIesrPrune)(TIesr_t, short);
      void (*SetTIesrTransiWeight)(TIesr_t, short);
      void (*SetTIesrSAD)(TIesr_t, short, short, short, short, short);
      void (*SetTIesrJacRate)( TIesr_t, short);
      void (*SetTIesrJAC)( TIesr_t, short, short, short, short, short,
			   short, short );
      void (*SetTIesrSBC)( TIesr_t, short );


#ifdef USE_AUDIO
      void (*SetTIesrVolRange)(TIesr_t, unsigned short, unsigned short);
#endif


      //
      // Getters for the previous block of Setters 
      //

      void (*GetTIesrPrune)(cTIesr_t, short*);
      void (*GetTIesrTransiWeight)(cTIesr_t, short*);
      void (*GetTIesrSAD)(cTIesr_t, short*, short*, short*, short*, short*);
      void (*GetTIesrJacRate)(TIesr_t, short*);
      void (*GetTIesrJAC)( cTIesr_t, short*, short*, short*, short*, short*,
			   short*, short* );
      void (*GetTIesrSBC)( cTIesr_t, short* );

#ifdef USE_AUDIO
      void (*GetTIesrVolRange)(TIesr_t, unsigned short *, unsigned short *);
#endif

} TIesrEngineSIRECOType;


/*--------------------------------*/
/* 
   This code defines the two functions that instantiate and close
   an instance of the TIesrEngineSIRECOType structure which 
   implements the TIesr SI recognizer engine.
*/

#ifdef __cplusplus
extern "C"
{
#endif
TIESRENGINESIAPI_API void TIesrEngineOpen (TIesrEngineSIRECOType *);
TIESRENGINESIAPI_API void TIesrEngineClose(TIesrEngineSIRECOType *);
#ifdef __cplusplus
}
#endif


#endif /* _TIESR_ENGINE_API_SIRECO_H */
