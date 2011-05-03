/*=======================================================================
 sbc_user.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header file provides the SBC user API interface.

======================================================================*/

#ifndef _SBC_USER_H
#define _SBC_USER_H


/*--------------------------------------------------------------
The Windows method of exporting functions from a DLL.
---------------------------------------------------------------*/
#if defined (WIN32) || defined (WINCE)

/* If it is not defined already, define a macro that does
Windows format export of DLL functions */
#ifndef TIESRENGINECOREAPI_API

// The following ifdef block is the standard way of creating macros which
// make exporting from a DLL simpler. All files within this DLL are compiled
// with the TIESRENGINECOREAPI_EXPORTS symbol defined on the command line.
// This symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see
// TIESRENGINECOREAPI_API functions as being imported from a DLL,
// whereas this DLL sees symbols defined with this macro as being exported.
#ifdef TIESRENGINECOREAPI_EXPORTS
#define TIESRENGINECOREAPI_API __declspec(dllexport)
#else
#define TIESRENGINECOREAPI_API __declspec(dllimport)
#endif
#endif

#else

#ifndef TIESRENGINECOREAPI_API
#define TIESRENGINECOREAPI_API
#endif

#endif
/*--------------------------------------------------------------------*/


#include "tiesr_config.h"
#include "tiesrcommonmacros.h"
#include "hlr_status.h"

#include "gmhmm_type.h"
#include "search_user.h"

#include "sbc_struct_user.h"

/* for debug of SBC */
//#define _SBC_DEBUG 1

/*--------------------------------
  Function interface to the SBC API
  --------------------------------*/

/* dump temporary information - only used for debugging so not used now */
/* void sbc_dump(FeaHLRAccType *pSBC, ushort i_node_idx); */


TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_save(FeaHLRAccType* pSBC , FILE* pf );


TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_load(FeaHLRAccType* pSBC , FILE* pf );


/* reset sufficient statistics before accumulating statistics for the current utterance.
   @param pSBC pointer to FeaHLRAccType
*/
TIESRENGINECOREAPI_API void sbc_rst_SBC_stat(FeaHLRAccType* pSBC );


/* reset sufficient statistics before accumulating for the current segment
   @param pSBC pointer to FeaHLRAccType
*/
TIESRENGINECOREAPI_API void sbc_rst_seg_stat(FeaHLRAccType* pSBC );


/*----------------------------------------------------------------
  sbc_open
  
  Set all elements of the FeaHLRAccType to opening values.  This 
  ensures that the structure is constructed correctly.
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API void sbc_open( FeaHLRAccType *pSBC );


/* 
   initialization of SBC method
   @param pSBC Pointer to FeaHLRAccTYpe structure holding sbc state
   @param fnDir path of network, hmms
   @param fname file name of phonenetic clustering tree
   @param load_monophone_only TRUE if only load monophone, FALSE also load clustering tree
   @param cep_dim cepstral feature vector size
   @param nbr_hmms number of HMMs
   @param bTriphone: TRUE if maps triphone to monophone
   FALSE if maps monophone to itself

   return NULL if the process is failed, otherwise, 
          return pointer to the address of FeaHLRAccType
*/
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_init_tree( FeaHLRAccType* pSBC,
					const char* fnDir, 
					const char *fname, 
					short load_monophone_only,
					ushort cep_dim, 
					ushort n_hmms, 
					Boolean bTriphone );

/* load triphone to monophone mapping list */
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_load_hmm2phone(const char gdir[],
					    ushort n_hmms, 
					    FeaHLRAccType *sa,
					    Boolean bTriphone );

/* return the leaf of the triphone HMM indexed by $hmm_idx */
short sbc_phone2terminalidx(ushort hmm_idx, FeaHLRAccType * sa);

/* obtain enhanced feature vector, assuming that it has not yet been calculated
   @param pSBC pointer to FeaHLRAccType
   @param org_obs pointer to the original feature vector
   @param obs_enh pointer to the address of the enhanced feature vector, need to have allocated space by user
   @param physical_hmm HMM code
   @param nbr_dim feature vector size
   @param p_sbc_bias pointer to the address of transformations, need to have an allocated space by user. 
*/
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_re_enhance( FeaHLRAccType * pSBC,
					 short *org_obs,
					 short *obs_enh,
					 ushort physical_hmm, 
					 short nbr_dim, 
					 short* p_sbc_bias);

/* obtain enhanced feature vector by sbc_enhance_all_trans.
   @param pSBC pointer to FeaHLRAccType
   @param obs observation feature vector
   @param physical_hmm hmm index
   @param nbr_dim feature vector size
   @param p_sbc_bias if the pointer is not NULL, set the bias to the address by the pointer
   return address of the enhanced feature vector, NULL if failed. 
*/
TIESRENGINECOREAPI_API short* sbc_enhance(
		  FeaHLRAccType * pSBC, 
		  short *obs,
		  ushort physical_hmm, 
		  short nbr_dim, 
		  short* p_sbc_bias);


/* feature bias removal of the current mfcc feature vector. 
   Save the enhanced features to all used transformation indices. Use sbc_enhance
   to obtain the enhanced feature vector.
   @param pSBC pointer to FeaHLRAccType
   @param obs input feature vector
   @param nbr_dim feature vector size of the input feature vector
*/
TIESRENGINECOREAPI_API TIesrEngineHLRStatusType sbc_enhance_all_trans( FeaHLRAccType * pSBC,
						short *obs,
						short nbr_dim);


/* accumulate sufficient statistics for SBC estimates.
   @param hmm_code HMM code
   @param cep_dim feature vector size
   @param seglength length of the current segment
   @param pSBC pointer to FeaHLRAccType
   @param fgtrho forgetting factor for SBC sufficient statistics accumulation
                 dif2 = dif2*fgtrho + newdif2* (1- fgtrho)
   return TRUE if sucess, otherwise return FALSE
*/
TIESRENGINECOREAPI_API Boolean sbc_acc_SBC(ushort hmm_code, ushort cep_dim,
		 ushort seglength, FeaHLRAccType * pSBC ,
		 ushort fgtrho );


/* update SBC parameters 
   @param gv pointer to gmhmm_type
*/
TIESRENGINECOREAPI_API void sbc_update_SBC(gmhmm_type* gv );


/* accumulate statistics 
   @param nbr_dim dimension of cepstral vector
   @param T number of frames of the current segment
   @param p_mean pointer to mean vector, Q11
   @param p_invvar pointer to inverse variance vector, Q9
   @param mfcc MFCC cepstral, Q11
   @param gamma posterior probabilty, Q15
   @param p_bias pointer to the bias of SBC method for the current model, Q11
   @param pSBC pointer to FeaHLRAccType. 

 */
TIESRENGINECOREAPI_API void sbc_acc_cep_bias(ushort nbr_dim, ushort T, short * p_mean, short *p_invvar,
		      const short * mfcc, ushort gamma, short * p_bias, FeaHLRAccType* pSBC );


TIESRENGINECOREAPI_API void sbc_observation_likelihood(ushort seg_stt, short T, ushort hmm_code,
				HmmType *hmm, 
				short pdf[], short n_mfcc, gmhmm_type *gv );


/*---------------------------------------------------------------- 
  sbc_set_default_params

  Set default SBC dynamic parameter settings in the global recognizer
  instance structure.  This is part of the capability provided so that
  the application designer can make changes to some SBC parameters
  advanced settings to improve recognition performance in some
  situations.  Setting parameters to other than the defaults should be
  done cautiously.
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API void sbc_set_default_params( gmhmm_type *gv );

/* return pointer to the HMM2phone */
TIESRENGINECOREAPI_API short * sbc_rtn_hmm2phone(FeaHLRAccType *sa);

/*---------------------------------------------------------------- 
  sbc_free

  This function frees any memory allocated during use of SBC.  This
  function must be called after using SBC if sbc_init_tree is called
  and has returned successfully, since that may have allocated
  memory. 
  ----------------------------------------------------------------*/
TIESRENGINECOREAPI_API void sbc_free( FeaHLRAccType* pSBC );

/* for confidence driven adaptation */
void sbc_backup(FeaHLRAccType * sa);
void sbc_restore(FeaHLRAccType * sa);

#endif
