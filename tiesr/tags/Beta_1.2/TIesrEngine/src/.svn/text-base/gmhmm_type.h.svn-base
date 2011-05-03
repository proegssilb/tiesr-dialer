/*=======================================================================
 gmhmm_type.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

 
 This header defines the gmhmm_type structure, which is the global
 structure that defines a TIESR recognizer API instance.

======================================================================*/

#ifndef _GMHMM_TYPE_H
#define _GMHMM_TYPE_H

/* Configuration of the recognizers */
#include "tiesr_config.h"

#include "tiesrcommonmacros.h"
#include "mfcc_f_def_struct_user.h"
#include "gmhmm.h"
#include "sbc_struct_user.h"

/*--------------------------------*/
typedef enum CompensationType {
   NONE = 0,
   CMN = 1,
   JAC = 2,
   SBC = 4,
   SVA = 8
} CompensationType;

/* used in TIesrFlex */
typedef struct hmmtb
{            /* hmm's center, left, right monophone index */
  int   c;
  int   l;
  int   r;
} Hmmtbl;

/*--------------------------------*/
/* TransType */
typedef struct {
      unsigned short   n_hmm;     /* total # of hmm (including m and f) */
      unsigned short   n_sym;     /* size of symlist[] */
      unsigned short   n_word;    /* # of words */

      unsigned short   n_set;     /* # of hmm set, e.g. 2 for male + female */
      unsigned short   n_hmm_set; /* # of hmm per set, 
				     e.g. = n_hmm / 2 for male / female */

      unsigned short   start;            /* start node */
      unsigned short   stop;             /* stop node */
      unsigned short   symlist[1];       /* list of OFFSETS FOR  all nodes, 
					    cannot use short *, more than 1 */
} TransType;

#ifdef _MONOPHONE_NET
/* the maximum number of N-best */
#define RS_OUTNBEST 24

typedef struct{
  short wcur; /* current word position */
  short hcur; /* current HMM position */
  ushort word[RS_OUTNBEST];
  ushort hmm[RS_OUTNBEST];
  ushort stt[RS_OUTNBEST];
  ushort stp[RS_OUTNBEST];

  ushort unbest;
  ushort u_nbest_word_start_at[RS_OUTNBEST];
  ushort u_nbest_hmm_start_at[RS_OUTNBEST]; 

  NBestType * pNBestCells; 
}NBestPath;
#endif

#ifdef USE_NBEST
/* the maximum number of N-best */
#define RS_OUTNBEST 24

typedef struct{
  short wcur; /* current word position */
  short hcur; /* current HMM position */
  ushort word[RS_OUTNBEST];
  ushort hmm[RS_OUTNBEST];
  ushort stt[RS_OUTNBEST];
  ushort stp[RS_OUTNBEST];

  ushort unbest;
  ushort u_nbest_word_start_at[RS_OUTNBEST];
  ushort u_nbest_hmm_start_at[RS_OUTNBEST]; 

  NBestType * pNBestCells; 
}NBestPath;
#endif

#define MAX_N_BESTS 30
/* N-best word indices */
typedef struct{
  short scr; /* score of the word sequence */
  short n_ans;/* number of words */
  ushort answers[20];
}NB_element; 

typedef struct{
  short n_bests; /* number of best hypothesis */
  short best_idx[MAX_N_BESTS]; 
  NB_element elem[MAX_N_BESTS];
}NB_words; 

/*--------------------------------*/
/*
** global control variables. invisible to API
*/
typedef struct gmhmm_type
{ 
      ushort avedepth;
      ushort bestdepth;
      //
      // front-end iteration
      //

      /* index to the first mean vector to be JAC compensated */
      ushort index_mean;

      /* count of incoming signal frames (w/o utterance detection) */
      ushort signal_cnt;

      /* being in the speech segment  */
      ushort speech_detected;

      /* frames received in the front-end, index to the first available
	 location (mutiplied by n_mfcc) */
      ushort mfcc_cnt;
      
      /* nbr of times the whole model set has been compensated */
      ushort nbr_cpy;
      
      /* the last sample of the previous window */
      short last_sig;

      /* store an analysis window of sample signal */
      short sample_signal[ WINDOW_LEN ];

      /* max of time signal (per utterance) */
      short amplitude_max;

      /* min of time signal (per utterance) */
      short amplitude_min;


      /* indication of first call */
      short pred_first_frame;

      /* frames fed into search engine, used in search.c */
      short frm_cnt;
      
      /*number of beam*/
      short beam_count;
      unsigned short ending_sym_count ;

      /* CMN, JAC, HLR may change this value */
      short comp_type;


      //
      // Volume testing and indication
      //

      // API low and high volume warning limit parameter settings

      unsigned short  low_vol_limit;
      unsigned short  high_vol_limit;

      // Volume variables

      /* indication of the volume setting */
      short vol;

      
      //
      // feature space dimension and processing
      //

      /* feature dimension, e.g. static mfcc */
      ushort n_mfcc;

      /* actual feature vector size of recognition models (static + dynamic)*/
      short nbr_dim;

      /* actual number of frequency filters: 20 or 26 */
      ushort n_filter;

      /* MFCC buffer for regression */
      short mfcc_buf[ MFCC_BUF_SZ ][ MAX_DIM_MFCC16 ];
  
      /* mfcc mean running vector */
      short mfcc_mean[ MAX_DIM_MFCC16 ];

      const short *cosxfm[  MAX_DIM_MFCC16 ];

      /* mel sacle filter */
      const mel_filter_type *mel_filter;
      

      //
      // SAD
      //
      

      // API SAD threshold parameter settings

      /* noise floor for sad */
      short th_noise_flr;

      /* speech stronger than noise level */
      short th_speech_delta;

      /* min dB level to be considered as speech */
      short th_min_speech_db;

      /* larger (e.g. 5)  -> more robust to noise pulses */
      short th_min_beg_frm;

      /* smaller (.e.g. 30) --> quicker latency  time */
      short th_min_end_frm;

      // SAD variables

      /* length of speech segment */
      short uttdet_cnt;

      /* smooth filter for noise floor */
      short noise_smooth;

      /* smoothed noise estimate */
      short ps_noisy_smoothed[WINDOW_LEN/2];

      /* noise floor */ 
      short noise_floor[WINDOW_LEN/2];

      /* smoothed autoc */
      short sm_autoc;

      /* peak track, speech loudness level */
      short speech_level;

      /* valley track, noise level */
      short noise_level;

      /* this frame is speech? */
      short uttdet_isspeech;

      /* cumulate the product of pram level and duration */
      long  delta_sum_accum;

      /* utterance detector machine state */
      short uttdet_state;

      /* begin frame */
      short uttdet_beg_frm;

      /* end frame */
      short uttdet_end_frm;

  /* end-of-grammar network has been reached, so SAD can trigger end-of-utterance
     the idea is that the end-of-grammar nodes are pointing to gv->trans->stop. From the stop node, 
     the pointing grammar nodes can be located. By looking up the nodes, their numbers are sequential.
     Hence, by checking if the true symbol is within the smallest and the largest numbers, the end of 
     grammar can be detected. */
  Boolean sr_is_end_of_grammar;
  short sr_MAX_END_SYM;   /* the highest true symbol index */
  short sr_MIN_END_SYM; /* the lowest true symbol index */
  
  Boolean gbg_end_of_speech; /* end of speech trigger set from the on-line garbage model
			      trigger if speech is detected but the length of 
			      low-confidence segments is more than a certain number */

      //
      // JAC/SVA/SBC compensation of channel and noise
      //

      // API JAC/IJAC/SVA/SBC parameter settings

      /* Number of model vector means compensated per input frame. */
      /* Depends on size of gtm, machine speed */
      /* Want to complete all mean in less than 1 second */ 
      short jac_vec_per_frame; 

      /* Threshold of component log likelihood required to accum in JAC */
      short jac_component_likelihood;

      /* Maximum size limit of an aligned segment to accum in JAC */
      short jac_max_segment_size;

      /* Coefficient for channel correction temporal smoothing filter */
      short jac_smooth_coef;

      /* IJAC channel discounting factor */
      short jac_discount_factor;

      /* IJAC channel correction deweighting factor */
      short jac_deweight_factor;

      /* IJAC channel inverse variance prior bayesian weight */
      short jac_bayesian_weight;

      /* SVA variance forgetting factor, power of two */
      short sva_var_forget_factor;
      
      /* SBC forgetting factor */
      short sbc_fgt;


      // JAC variables for channel estimate and accumulation.

      /* log-spectral channel */
      short log_H[ N_FILTER26 ];

      /* noise, Q 9 */
      short log_N[ N_FILTER26 ];

      /* not used in SD: */
      long accum [ ACC_MEMORY ]; /* JAC accumulator  */

      /* contribution of current accumulator values to the total  */
      ushort smooth_coef;

      /* SBC structure for dynamic compensation */
      FeaHLRAccType* sbcState;


#ifdef  USE_SVA
      /* variance scaling */
      short log_var_rho [ MAX_DIM ];
      short var_dif1[MAX_DIM]; 
      /* fisher information matrix */
      long  var_fisher_IM[  MAX_DIM ];

      /* per-utterance fisher information matrix */
      long  var_fisher_IM_per_utter[  MAX_DIM ];
#endif

      /* running noise estimation in MFCC domain */
      short mfcc_noise[ MAX_DIM_MFCC16 ];
      long mfcc_noise_init_acc[ MAX_DIM_MFCC16 ];  /*init noise estimation accumulator*/

      //
      // back traced segmental information
      //
      //  ushort hmm_code[MAX_NBR_SEGS];  /* HMM code */
      //  ushort stt[MAX_NBR_SEGS];       /* start    */
      //  ushort stp[MAX_NBR_SEGS];       /* stop     */

      /* variable size, so SI, SA, and SD can all use: */
      ushort *hmm_code;  /* HMM code */
      ushort *stt;       /* start    */
      ushort *stp;       /* stop     */

      ushort nbr_seg;    /* number of phonetic segments   */
      ushort hmm_dlt;    /* delta for hmm code */ 
      ushort max_frame_nbr; /* max. number of frames of mfcc */


      //
      //  search space:
      //

      /* symcell and statecell arrays for evaluation */
      SymCell   *symcell_base;
      StateCell *statecell_base;

      /* backtrace array for evaluation */
      BeamType  *beam_base;      
      ushort cur_beam_count ; /* number of active beam cells for the current time */
      /* time cells */
      TimeType  *time_base;

  Boolean bUseNBest; /* if the Nbest output is invoked */

#ifdef USE_NBEST
  NBestPath * ptr_nbest_tmp; /* pointer to the tempary space for N-best */

  ushort max_nb_beam_cell;
  BeamType * nb_beam_base;
  TimeType * nb_time_base;

  Boolean  nb_first_time_call; 
  ushort nb_next_time; 
  unsigned short last_nbest_cell;

#endif

#ifdef _MONOPHONE_NET
  NBestPath * ptr_nbest_tmp; /* pointer to the tempary space for N-best */

  BeamType * nb_beam_base;
  TimeType * nb_time_base;

  Boolean  nb_first_time_call; 
  ushort nb_next_time; 
  unsigned short last_nbest_cell;

#endif

      //
      // search control
      //

      /* total scratch memory size for (including gmhmm_type) */
      unsigned int total_words;

      /* (-10)*(1<<6)    -6144  6 * 16, Q 6 */
      short prune;

      /* (3)*(1<<6) tidigit 6, wv dig 4, wv cmd 1, Q 6 */
      short tranwgt;


      //
      // models (grammar + gtm)
      //

      /* char pointer to the model directory for TIesrEngine */
      char *chpr_Model_Dir;

      TransType *trans;
      short     *base_net;
      unsigned short  *base_hmms;
      short     *base_mu;
      /* original non-compensated model mu */
      short     *base_mu_orig;
      short     *base_var;
      /* original non-compensated model sigma2 */
      short     *base_var_orig;
      short     *base_tran;
      unsigned short  *base_pdf;
      short     *base_mixture;
      short     *base_gconst;
      short     *obs_scr;
      
      short     *gauss_scr;//[7000];  //this should be the max of total Gaussian number, 6800 is enough. 5000 is enough for a task
      //short tt;  //temp
      
      /* mean and variance scaling, used for unpacking mean vectors */
      short     *scale_mu;
      short     *scale_var;
      /* the size of obs_scr */ 
      unsigned short     n_pdf;
      /* the number of mean vectors */
      unsigned short     n_mu;
      /* the number of variance vectors */
      unsigned short     n_var;
      char      **vocabulary;


      //
      // scales
      //

      /* scale for the feature vectors, used only for SA */
      short *scale_feat;

      /* mu scale power of 2, sta+dyn, points to the feature scaling */
      short muScaleP2[ MAX_DIM ];


      //
      // search space variables: statistics for sym, state, and beam 
      // cells arrays peak usage 
      //
      EvalStat  evalstat;

      /* idx to state cell and sym cell arrays, alternate allocation direction
      **   between begin and end of arrays depending on odd or even frames 
      */
      EvalIdx   eval[2];

    Sym2PosType sym2pos_map; /* mapping from symbol indices to start state cell positions */

      /* best score for current frame evaluation */
      short          best_cur_scr;

      /* best score from previous frame evaluation */
      short          best_prev_scr;

      long           cum_best_prev_scr;
      unsigned short best_sym;
      
      unsigned short best_word; //
      unsigned short best_word_frm; //
      unsigned short best_word_gender; //

  NB_words nbest; /* space saving N-best word indices */

      bool need_compact;
      short word_backtrace;  //0 Hmm , 1, word, -1, hmm  and BT
      
      /* best symbol end score */
      long           best_sym_scr;

      /* index to record next frame table table entry */
      unsigned short next_time;

      /* last beam cell */
      unsigned short last_beam_cell;


      //
      // search space dimensioning 
      //
      ushort max_state_cell;
      ushort max_beam_cell;
      ushort max_time_cell;

      /* max_sym_cell/2 */
      ushort max_sym_cell2;

      /* direct access of time table */
      short time_table_idx[NN];


      //
      // output:
      //

      /* word sequence of the recognized string */
      ushort *answer;

      /* number of words in the recognized string   */
      ushort nbr_ans;

      //
      // pointor for feature vector sequence (for the whole utterance):
      //
      ushort *mem_feature;
/*
** common structure for SD live
*/

      /* max. total number of frames for each utterance */
      ushort total_frames[2];

      /* number of utterance loaded to memory */
      short nbr_loaded_utter;

      /* index of the first available place in mfcc buffer */
      //  ushort mem_start_index;

      /* index of the first available frame in mfcc buffer */
      ushort mem_start_frame;


  /* ---------------  Cluster-dependent JAC, Gaussian selection, Online Reference Modeling -*/
  /*  the following functions with their default arguments are used to setup parameters
      Gaussian selection -> 
      rj_set_param( ushort perc_core_clusters_q15,
		   ushort perc_inter_clusters_q15, 
		   gmhmm_type * gv )
      Online reference modeling, VAD, and End-of-speech detection -> 
      rj_set_ORM(GBG_num_ids, 
	     GBG_END_SPEECH_CNT_TH, GBG_END_SPEECH_TH_PERC, GBG_END_SPEECH_START, gv)
  */
  short * offline_vq; /* pointer to vector-quantized mean vectors */

  short * pPhoneLevelPrune; /* pointer to the phone-level pruning structure */

  /* --------------  spectral subtraction ----------------*/
  /* change noise subtraction parameters using the following function defined in noise_sub_user.h
     void ss_set_parm( ushort sAlpha, short sBeta, NssType* p_ss_crtl);
     the default value of sAlpha is NSS_ALPHA
     the default value of sBeta is NSS_BETA
  */
  short * pNss; /* pointer to noise subtraction data structure */ 

  short cm_score; /* confidence score of the current recognition result, in Q6 */
  short * pConf; /* pointer to the confidence measure data structure */

  /* for force alignment */
  TransType *trans_bk;   /* backup symbol network */
  short     *base_net_bk;
  short nbr_ans_bk; 
  short answer_bk[MAX_NBR_SEGS];

#ifdef USE_AVL_TREE
  /* for AVL tree */
  short * pAVLTree; 
#endif
  
  /*for realtime guard*/
  unsigned int FramesQueued;
  unsigned int prev_FramesQueued;
  
  short guard;/*will be reset when queue reduce, but will increase when queue increase at a high level*/
  short realtime_th;  // frames that trigger more pruning as realtime guard, 50 is one second.
  short realt_guard;
  

} gmhmm_type;




#define display_size(mesg, curt_mem,stt_mem, total_mem_size) \
  PRT_ERR(printf("Current: %6d, Total: %6d, Limit: %6d (words) [%s]\n", curt_mem, stt_mem, total_mem_size, mesg))

#endif
