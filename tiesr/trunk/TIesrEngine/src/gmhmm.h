/*=======================================================================

 *
 * gmhmm.h
 *
 * Header for internal data and structures in TIesr.
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


 This header contains only structures and data used internally in the
 search engine.  It rightly should be called search.h, but is retained
 for historical reasons.

======================================================================*/

#ifndef GMHMM_H
#define GMHMM_H

#include "tiesr_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
#include "status.h"
*/

#include "tiesrcommonmacros.h"

/*
Now in tiesrcommonmacros.h
#define ushort unsigned short
*/

/* ---------------------------------------------------------------------------
   define
   ---------------------------------------------------------------------- */
/* Now in tiesrcommonmacros.h
#define TRUE  1
#define FALSE 0
*/


/* real time guard parameters */

#define REALTIME_TH  50  // frames that trigger more pruning as realtime guard, 50 is one second
#define REALT_GUARD  800




#define   MAX_STR            1024
#define   MAX_WORD           1024
#define   MAX_MONOPHONES     75   /* maximum number of monophones */

/* static + dynamic */
/*
#define   MAX_DIM            (2 * MAX_DIM_MFCC16) 
*/


#define   BAD_SCR            -32768
#define   PMCFRM             8         /* must be power of 2 */
#define   PMCFRM_P2          3

#define PRUNE_COEFF -3  /* default pruning */
#define WIP 0   /*  default word insertion penalty */

#define NSYM 2 /* hash code nbr */

//#define   MAX_STATE_CELL     20000    /* these 4 const define search space */
//#define   MAX_SYM_CELL       4000     /* actual space allocated  */ 
//#define   MAX_BEAM_CELL      10000 
//#define   MAX_TIME_CELL      2000 
//#define   MAX_SYM_CELL2     (MAX_SYM_CELL>>1)  

#define NN (unsigned short) 20 /* store last NN time stamps */
#define   MAX_WORD_UTT      128/* max words in an utterance */

/*
** number of frames to be stored in memory for JAC adaptation:
*/
#define   MAX_FRM_NBR 600   /* 50 frames/second under 8KHz (200 -> 4 seconds of speech */

#define MAX_NBR_SEGS  70 /* per utterance max segments */

/* Search array structure sizing */
#define BEAM_SZ  3
#define SYMB_SZ  3 /* 2 times cell 2 */
#define STATE_SZ 3
#define TIME_SZ  2


/* From search.cpp */
#define UNDEF_CELL  0xffff
#define UNDEFINEDTIME (-1)


/* ---------------------------------------------------------------------------
   backtrace structures 
   ---------------------------------------------------------------------- */

/* state cell */
typedef struct {
  short          scr;   /* accumulated score */
  unsigned short sym;   /* previous sym */
  unsigned short frm;   /* time of previous sym */
} StateCell;

/* sym cell */
typedef struct {
  unsigned short sym_code;    /* MSB bit 1 means not active */
  unsigned short start_cell;  /* start state cell index */
  unsigned short depth;  
} SymCell;


/* backtrace cell */
typedef struct ele {
  unsigned short sym;
  unsigned short idx_flag; /* store 1. back cell index AND 2. empty flag */
  unsigned short next;     /* index to next cell */
} BeamType;


typedef struct  {           /* points to the first element of beam cells */
  unsigned short frm;       /* frame index */
  unsigned short beam_idx;  /* the index of BeamType cell */
} TimeType;


typedef struct {
  unsigned short high_pos;   /* watermark, peak usage estimate */
  unsigned short cur_pos;    /* next available cell */
} PosType;

typedef struct {             /* peak usage statistics */
  PosType      state;        /* state cell statistics */
  PosType      sym[NSYM];    /* sym cell statistics   */
  PosType      beam;         /* beam cell (backtrace) statistics */
  PosType      time;         /* time cell (backtrace) statistics */
} EvalStat;

typedef struct {
  unsigned short max_nbr_syms; /* maximum number of symbols */
  short nbr_syms; /* number of accessed symbols */
  /* [15]: 1 : previous time ; 0 : current time */
  short *sym2pos;
  unsigned short *syms;
} Sym2PosType; 

typedef struct {
  unsigned short cur_sym[NSYM];    /* active symbol's start cell position 
				    for male/female */
  unsigned short cur_state;  /* active states's start cell position */

} EvalIdx;

/* ---------------------------------------------------------------------------
   net structure, top level grammar, it consists of HMM's
   ---------------------------------------------------------------------- */

//typedef struct {
//  unsigned short hmm_code; /* HMM code, not context-expanded */
//  unsigned short n_next;   /* the number of symbols in next[]. if bit15 == 1 then the last element in
//                              "next[]" is the word code  */
//  unsigned short next[1];  /* the list of following symbols, 
//			      cannot use short *, more than 1 */
//} SymType;


/* Now in search_user.h*/
//typedef struct {
//  unsigned short   n_hmm;     /* total # of hmm (including m and f) */
//  unsigned short   n_sym;     /* size of symlist[] */
//  unsigned short   n_word;    /* # of words */
//
//  unsigned short   n_set;     /* # of hmm set, e.g. 2 for male + female */
//  unsigned short   n_hmm_set; /* # of hmm per set, 
//                                 e.g. = n_hmm / 2 for male / female */
//
//  unsigned short   start;            /* start node */
//  unsigned short   stop;             /* stop node */
//  unsigned short   symlist[1];       /* list of all nodes, 
//					cannot use short *, more than 1 */
//} TransType;

/* ---------------------------------------------------------------------------
   HMM structure
   ---------------------------------------------------------------------- */

/* Now in search_user.h */
//typedef struct {
//
//  short          tran;    /* offset pointing into the transition matrix pool,
//                             NOT converted into real addr in initialization
//                          */
//  unsigned short pdf_idx[1]; /* each entry is an offset pointing into
//				base_pdf[],
//				it is the observation distribution for each
//				emission state,
//				pdf_idx[ n_state - 1 ]
//			     */
//} HmmType;
//

/* gaussian observation likelihood type evaluation */
/* Now in search_user.h
typedef enum ObsType 
{
   VITERBI,
   FULL,
   COMPONENT
} ObsType;
*/

/*
** compensation for robustness
*/
/* Now in gmhmm_type.h
typedef enum CompensationType {
  NONE,
  CMN,
  JAC,
};
*/



/*--------------------------------*/
/* Macros used for tiesr search engine */

#define HCODE(sym) ((sym) & 1)

#define   STATE_CELL(base,pos,n_state,time_idx, max_state_cell) \
      (time_idx) ? ((base) + (pos)) : \
      ((base) + max_state_cell - (pos) - n_state)

/* was: ((base) + MAX_STATE_CELL - 1 - (pos) - n_state) */

#define   SYM_CELL(base,pos,time_idx,max_sym_cell2) \
      (time_idx) ? ((base) + (pos)) : \
     ((base) + max_sym_cell2 - 1 - (pos))

#define   PREV_SYM(time_idx,pc)   (time_idx)? pc-- : pc++

#define   NEXT_SYM(time,pc)   ((time) & 1) ? pc++ : pc--

/* 
Now in tiesrcommonmacros.h
#define   MAX(a,b)  ((a) > (b) ? (a): (b))
#define   MIN(a,b)  ((a) < (b) ? (a): (b))
*/

/*
** new macros for single grammar multi-hmm-set decoding:
*/

#define VTL_SYM(sym,sym_d) ((sym)+(sym_d))

#define GET_HMM(base_hmms, hmmcode, hmm_dlt) (HmmType *) ( base_hmms + base_hmms[ hmmcode + hmm_dlt ])

/*
** extract n_next info:
*/
#define GET_N_NEXT(n_next) (n_next & 0x7fff)



/*
** introduced for name dilaing: 
*/

/* Two macros named identically, except for case.  Not only that,
   nbr_states was a variable within code.  The
   macro nbr_states is never used in code, only through the macro NBR_STATES,
   so for now, renamed it.  It should be deleted entirely. 
*/
/* number of HMM states (incl. exit) */
/* #define nbr_states(hmm,b_tran) (*( b_tran + hmm->tran )) */

/* #define NBR_STATES(hmm,base_tran) nbr_states(hmm, base_tran) */

#define NR_STATES(hmm,b_tran) (*( b_tran + hmm->tran ))

/* the number of emitting states + 1 */
#define NBR_STATES(hmm,base_tran) NR_STATES(hmm, base_tran) 

#define GET_PI(hmm,base_tran)     (base_tran + hmm->tran + 1)   /* initial prob, skip nbr_state */

#define get_a_gvn_trans(hmm,base_tran)    (base_tran + hmm->tran + NBR_STATES(hmm,base_tran))

#define get_ai_gvn_trans(hmm,i,base_tran) (get_a_gvn_trans(hmm,base_tran)+(NBR_STATES(hmm,base_tran)*(i)))

#define GET_AIJ(hmm,i,j, base_tran) (get_ai_gvn_trans(hmm,i,base_tran)[j]) /* access to transition prob: a_ij  */

#define HAS_TRANSITION(tr_prob) ((tr_prob) > BAD_SCR)
 
/* The below code is revised since the two macros perform identically and are
   confusing since the names are the same except for case.

*/
/*
#define for_ems_states(i,hmm,b_tran) for (i=0; i< nbr_states(hmm,b_tran) - 1; i++) 

#define FOR_EMS_STATES(i,hmm,base_tran) for_ems_states(i,hmm,base_tran)
*/
#define FOR_EM_ST(i,hmm,b_tran) for (i=0; i< NR_STATES(hmm,b_tran) - 1; i++) 

#define FOR_EMS_STATES(i,hmm,base_tran) FOR_EM_ST(i,hmm,base_tran)


#define FOR_ALL_STATES(i,hmm) for (i=0; i< NBR_STATES(hmm); i++) 

#define GET_BJ_IDX(hmm,j) (hmm->pdf_idx[j])  /* i.e. the rank in state distribution pool */

#define GET_MIX(gv,pdf_idx) (gv->base_mixture + gv->base_pdf[ pdf_idx ])

#define GET_BJ(gv,hmm,j) GET_MIX(gv,GET_BJ_IDX(hmm,j))

#define MIX_SIZE(state_pdf) (*(state_pdf)) /* number of gaussian in the mixture */
#define GET_MIXING_COMPONENT(state_pdf)  (state_pdf+1) /* pointor to the mix cell */
#define COMPONENT_SIZE 3 /* for mixture component: weight, mean, sigma2 */

#define SIZEOFAMIX (1+COMPONENT_SIZE) /* size of a single gaussian mixture */

#define NEXT_MIXING_COMPONENT(state_pdf) (state_pdf+COMPONENT_SIZE)
/*
** iteration on each mixing component:
*/

#define FOR_EACH_MIXING(k,pmix,pdf) \
 for (k=0, pmix = GET_MIXING_COMPONENT(pdf); k<MIX_SIZE(pdf); k++, pmix = NEXT_MIXING_COMPONENT(pmix))
/*
** sequential access of mixture components:
*/
#define MIXTURE_WEIGHT(mixing_pdf)  (mixing_pdf[0])

#define INDEX_FOR_MEAN(mixing_pdf)  (mixing_pdf[1])

#define INDEX_FOR_VARC(mixing_pdf)  (mixing_pdf[2])

/*
** return the begining of the array for mean and sigma2:
*/
#define GET_MU(base_mu,pmix,_dim_) (base_mu + (INDEX_FOR_MEAN(pmix) * _dim_))

#define GET_SIGMA2(base_var, pmix,_dim_) (base_var + (INDEX_FOR_VARC(pmix) * _dim_))

#define FOR_EACH_MEAN(i,m,_dim_, base_mu,n_mu) for (m=base_mu, i=0; i<n_mu; i++, m += _dim_)

#define FOR_EACH_SIGMA2(i,m,_dim_, base_mu,n_mu)  FOR_EACH_MEAN(i,m,_dim_, base_mu,n_mu)

#define ADJ_POINTOR(p_old, base_old, base_new) (base_new + (p_old - base_old))

/*
** given the size of different search arrays, calculate total search space size
*/

#define SEARCH_SIZE(beam_z, sym_z, state_z, time_z) \
        (beam_z * BEAM_SZ  +  SYMB_SZ * sym_z + state_z * STATE_SZ + TIME_SZ * time_z)

#define SEARCH_SPACE(evalstat) \
        SEARCH_SIZE(evalstat.beam.high_pos, MAX(evalstat.sym[0].high_pos,evalstat.sym[1].high_pos), \
        evalstat.state.high_pos, evalstat.time.high_pos)


/* From search.cpp */
#define CELLEMPTY(idx_flag) ((idx_flag) == 0x7fff)

#define MATCH_TIME(tm, time_base, stop_frm) for (tm = 0; time_base[tm].frm != stop_frm; tm++);

#define MATCH_SYMB(bm, bm0, beam0, best_sym) for (bm = beam0 + bm0; bm->sym != best_sym; bm = beam0 + bm->next)

#define NOT_END_IX_CELL(index_beam, max_beam_cell) (index_beam != max_beam_cell)

#define BACKWARD_MARKING(bm,b_index,cont, beam, max_beam_cell) \
   do { \
     if (bm->idx_flag & 0x8000 ) break; /* predecessors already been marked */  \
     b_index = bm->idx_flag; \
     bm->idx_flag |= 0x8000; /* color it */  \
     if ((cont = NOT_END_IX_CELL(b_index,max_beam_cell)==TRUE)) bm = beam + b_index; \
   } \
   while (cont) ; /* only need to traceback to "seen" */ 

//trail code:
#define ZBACKWARD_MARKING(bm,b_index,cont, beam, max_beam_cell, depth) \
   do { \
        depth++; \
     b_index = bm->idx_flag; \
     bm->idx_flag |= 0x8000; /* color it */  \
     if ((cont = NOT_END_IX_CELL(b_index,max_beam_cell)==TRUE)) bm = beam + b_index; \
   } \
   while (cont) ; /* only need to traceback to "seen" */ 


#define GET_NEW_CELL(found,i,pos, maxm_pos) \
  if (found) pos.cur_pos = i+1;  /* return the empty backtrace cell */ \
  else    /* update watermark, check overflow */ { \
    if (pos.high_pos == maxm_pos) { \
     PRT_ERR(fprintf(stderr, "Error: backtrace beam or time cell overflow (%d)\n", maxm_pos)); \
     return USHRT_MAX; \
    } \
    else {\
      i = pos.high_pos; \
      pos.cur_pos = ++pos.high_pos; \
    } \
  }

#define SET_NEXT_SYM(sym_cell,ps,direction, nbr) {sym_cell = ps; if ( --(nbr) ) ps += direction; else ps = NULL; }

#define SET_SYM_CELL(ps0,ps1,sym_cell,nbr0,nbr1,direction, expr) \
    if (!ps0) SET_NEXT_SYM(sym_cell,ps1,direction, nbr1) \
      else if (!ps1) SET_NEXT_SYM(sym_cell,ps0,direction, nbr0) \
           else if (expr) SET_NEXT_SYM(sym_cell,ps0,direction, nbr0) \
                else SET_NEXT_SYM(sym_cell,ps1,direction, nbr1)



/* 
** given mixture array, dim, and i-th mixing component, return precision vector
 */

#ifdef BIT8VAR
#define get_var(base_var, mix_ar, dim_, i_) (base_var + mix_ar[ i_ * 3 + 3 ] * ((dim_)>>1))
#else
#define get_var(base_var, mix_ar, dim_, i_) (base_var + mix_ar[ i_ * 3 + 3 ] * ((dim_)))
#endif


#ifdef BIT8MEAN 
#define get_mean(base_mu, mix_ar, dim_, i_) (base_mu + mix_ar[ i_ * 3 + 2 ] * ((dim_)>>1))
#else
#define get_mean(base_mu, mix_ar, dim_, i_) (base_mu + mix_ar[ i_ * 3 + 2 ] * ((dim_)))
#endif



#define GET_ALPHA(alpha,t,NBRST) ((t) & 1? (alpha + NBRST): alpha)

#define FOR_E_STATES(i,b_i,nbr_states, beta, T) for (i=0, b_i = beta; i<nbr_states; i++, b_i += T) 




/* ---------------------------------------------------------------------------
   function prototype
   ---------------------------------------------------------------------- */

/* Not needed or used here now. */
/*
extern short gauss_det_const(short *, ushort, const short []) ;

extern short to_short(float value, int qpoint);
*/

//extern void const_init();

/*
extern short circ_idx(short idx);
extern void compute_regression(short buf_idx, short type, short *reg_mfcc, short n_mfcc, const short *,
			       short mfcc_buf[ MFCC_BUF_SZ ][ MAX_DIM_MFCC16 ]);
*/

//extern void pmc(short *sp_mfcc,   short *noise_log_ps, short *pmc_mfcc, short *chn, short jac, short n_mfcc);

//extern short lo_gpolyfit(long value, short compensate);

#endif
