/*=======================================================================

 *
 * search.cpp
 *
 * HMM Viterbi search.
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

 Viterbi HMM search functions.

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"
#include "search_user.h"
#include "dist_user.h"
#include "gmhmm.h"
#include "sbc_user.h"
#include "load_user.h"
#include "pmc_f_user.h"
#include "uttdet_user.h"
#ifdef USE_NBEST
#include "nbest_user.h"
#endif
#ifdef USE_CONFIDENCE
#include "confidence_user.h"
#endif
#if defined(USE_RAPID_SEARCH) || defined(RAPID_JAC) || defined(USE_AVL_TREE)
#include "rapidsearch_user.h"
#endif

//#define DBG_SEARCH
//#define SHOW_NBEST

//#define DEBUGP

#ifdef SHOW_DIAGONISIS
//testing and statistical
long int global_count; 
long int global_count2;
long int global_frame;
int sttnode=0;

#endif

#ifdef DBG_SEARCH
short global_active_sym=0;
short max_glb_active_sym = 0;
#endif
/*
**   (beam_z * 3 + 2 * sym_z * 2 + state_z * 3 + 2 * time_z)
*/

TIESRENGINECOREAPI_API void set_search_space
(short *search_space, ushort beam_z, ushort sym_z, ushort state_z, ushort time_z, gmhmm_type *gv)
{
  unsigned int c = 0;
  short k; 

  gv->max_state_cell = state_z;
  //gv->max_sym_cell2 = (sym_z>>1) ;
  gv->max_sym_cell2 = (sym_z>>1); 
  gv->max_beam_cell  = beam_z;
  gv->max_time_cell  = time_z;

//first beam cell
  gv->beam_base = (BeamType*) (search_space + c);
  
  c += gv->max_beam_cell * BEAM_SZ;  

//second sym cell (in two parts).
  gv->symcell_base = (SymCell*) (search_space + c);

  /*sym cell has size of 2.  2 genders(or even/odd hmms), t,t+1 (or even odd frame)  */
  //c += 2 * gv->max_sym_cell2 * 2/* ! make it to max_sym_cell */  * 2;
  c += SYMB_SZ * gv->max_sym_cell2 * 2; /* ! make it to max_sym_cell */   


//third state cell
  gv->statecell_base = (StateCell *) (search_space + c);
  c += gv->max_state_cell * STATE_SZ;

//last time cell
  gv->time_base = (TimeType *) (search_space + c);
  c += TIME_SZ * gv->max_time_cell;

//  printf("NOw C= %d - and %d ==\n", c,  SEARCH_SIZE(beam_z, sym_z, state_z, time_z));
#ifdef USE_NBEST
  if (gv->bUseNBest){
    gv->max_nb_beam_cell  =  NBESTELEMS; 
    tmem = (short*) malloc(sizeof(short)*gv->max_nb_beam_cell * 3);
    gv->nb_beam_base = (BeamType*) tmem; 
    tmem = (short*) malloc(sizeof(short)*gv->max_time_cell);
    gv->nb_time_base = (TimeType*) tmem; 
  }else{
    gv->nb_beam_base = NULL;
    gv->nb_time_base = NULL;
  }
#endif

#ifdef _MONOPHONE_NET
  if (rs_get_monophone_search_space(gv)){
    tmem = (short*) malloc(sizeof(short)*gv->max_beam_cell * 3);
    rs_get_monophone_search_space(gv)->nb_beam_base = (BeamType*) tmem; 
    tmem = (short*) malloc(sizeof(short)*gv->max_time_cell);
    rs_get_monophone_search_space(gv)->nb_time_base = (TimeType*) tmem; 
  }
#endif

  gv->sym2pos_map.nbr_syms = 0;
  for (k=gv->sym2pos_map.max_nbr_syms-1;k>=0;k--)
    gv->sym2pos_map.sym2pos[k] = -1; 

}

static void init_last_time_table(gmhmm_type *gv)
{
  unsigned short i;
  for (i=0; i<NN; i++){
    gv->time_table_idx[i] = UNDEFINEDTIME;
  }
}

/* ---------------------------------------------------------------------------
   this is the watermark statistics, used to compute the peak usage of
   a batch of recognition test files
   called per file
   ---------------------------------------------------------------------- */

static void init_high_pos(gmhmm_type *gv)
{
  short i;
  gv->evalstat.state.high_pos = 0;
  for (i=0; i< NSYM; i++){
    gv->evalstat.sym[i].high_pos = 0;
  }
  gv->evalstat.beam.high_pos = 0;
  gv->evalstat.time.high_pos = 0;
#ifdef USE_NBEST
  gv->evalstat.nb_beam.high_pos = 0;
  gv->evalstat.nb_time.high_pos = 0;
#endif

}

/* ---------------------------------------------------------------------------
   called per frame
   ---------------------------------------------------------------------- */

static void init_cur_pos( short idx, gmhmm_type *gv)
{
  short i, k;

  gv->eval[ idx ].cur_state = 0;
  for (i=0; i< NSYM; i++) {
    gv->eval[ idx ].cur_sym[i] = 0; 
  }

  for (k=0;k<gv->sym2pos_map.nbr_syms;k++){
    i = gv->sym2pos_map.syms[k];
    gv->sym2pos_map.sym2pos[i] = -1; 
  }
  gv->sym2pos_map.nbr_syms = 0; 
}

/* ---------------------------------------------------------------------------
   clear Gaussian mixture score storage
   ---------------------------------------------------------------------- */

static void clear_obs_scr(gmhmm_type *gv)
{
  short i = gv->n_pdf;
  for (i=0; i< gv->n_mu; i++)
    gv->gauss_scr[i]= LZERO; 

}

/* ---------------------------------------------------------------------------
   update eval cell peak usage estimate watermark,
   check overflow for SYM and STATE cells.
   ---------------------------------------------------------------------- */

static short
eval_cell_overflow( short n_state, short h_code, gmhmm_type *gv )
{
  /* need 1 sym cell and n_state state cells */
  PosType *pos = gv->evalstat.sym + h_code;
  
   if (n_state == 32767) {
     PRT_ERR(printf("code large\n"));
     return eTIesrEngineEvalCellOverflow;
  }
  pos->cur_pos = gv->eval[ 0 ].cur_sym[h_code] + gv->eval[ 1 ].cur_sym[h_code] + 1;
  
  gv->evalstat.state.cur_pos = gv->eval[ 0 ].cur_state + gv->eval[ 1 ].cur_state + n_state;
  
  /* update watermark */

  if ( pos->cur_pos > pos->high_pos ) pos->high_pos = pos->cur_pos;
  if ( gv->evalstat.state.cur_pos > gv->evalstat.state.high_pos )
    gv->evalstat.state.high_pos =  gv->evalstat.state.cur_pos;

  /* check overflow */

  if ( pos->high_pos > gv->max_sym_cell2 ) {
    PRT_ERR(fprintf(stderr, "Error: Sym eval cell overflow, %d > %d\n", 
		    pos->high_pos, gv->max_sym_cell2));
    return eTIesrEngineEvalCellOverflow;
  }
  
  if ( gv->evalstat.state.high_pos > gv->max_state_cell ) {
    PRT_ERR(fprintf(stderr, "Error: State eval cell overflow\n"));
    PRT_ERR(printf(" short n_state, short h_code = %d =%d  max_state_cell = %d\n",  n_state,  h_code, gv->max_state_cell ));
    return eTIesrEngineEvalCellOverflow;
  }

  return eTIesrEngineSuccess;        /* no overflow */
}
    
/* ---------------------------------------------------------------------------
   allocate eval cells (one sym cell and its corresponding state cell"s")
   ---------------------------------------------------------------------- */
/* @param eval_idx : evaluation cell of the current time, $cur_idx 
 */
static StateCell *
allocate_eval_cell(EvalIdx *eval_idx, unsigned short sym, short n_state, short cur_idx,
		   gmhmm_type *gv, int depth)
{
  StateCell *pstate, *psco;     /* ptr to state cell */
  SymCell   *psym;       /* ptr to sym cell */
  short h_code = HCODE(sym); /* male/female? */
  short mapidx; 

  /* cell overflow? */

  if ( eval_cell_overflow( n_state, h_code, gv ) != eTIesrEngineSuccess ) 
    return NULL;
  
  /* allocate sym cell */
  psym = h_code? gv->symcell_base + gv->max_sym_cell2: gv->symcell_base;
  
  /*get pointer of current sym*/
  psym = SYM_CELL(psym, eval_idx->cur_sym[h_code], cur_idx, gv->max_sym_cell2); 
  
  /* have the symbol lists */
  /* get the sym2pos mapping */
  mapidx = gv->sym2pos_map.nbr_syms;
  if ((mapidx < gv->sym2pos_map.max_nbr_syms) && (mapidx >= 0) && (sym < gv->sym2pos_map.max_nbr_syms) ){
    gv->sym2pos_map.syms[mapidx] = sym; 
    gv->sym2pos_map.nbr_syms++;
    gv->sym2pos_map.sym2pos[sym] = eval_idx->cur_state; 
  }
  else 
    return NULL;

  eval_idx->cur_sym[h_code]++;            /* increment to next available cell */

  /* corresponding start state cell */
  psym->start_cell = eval_idx->cur_state;
  psym->sym_code = sym;
  psym->depth=depth;
  
  /* allocate state cell */
  pstate = STATE_CELL(gv->statecell_base, eval_idx->cur_state, n_state, cur_idx, gv->max_state_cell);

  eval_idx->cur_state += n_state;   /* increment to next available cell */


  psco = pstate;
  while ( n_state-- ){
    psco->scr = BAD_SCR; 
    psco->frm = (ushort) UNDEFINEDTIME; 
    psco++;
  }

  return pstate;
}

/*
** get time information of a given beam cell
*/
static unsigned short get_frame_time(unsigned short bm, gmhmm_type *gv)
{
  int i;
  unsigned short q;
  
  for (i=0; i < gv->evalstat.time.high_pos; i++) {
    if (gv->time_base[i].beam_idx != USHRT_MAX) {
      for (q = gv->time_base[i].beam_idx; NOT_END_IX_CELL(q, gv->max_beam_cell); q = gv->beam_base[q].next)  {
	if (q == bm) return gv->time_base[i].frm;
      }
    }
  }
  return 0; /* impossible to reach here */
}

/* ---------------------------------------------------------------------------
   find existing start state cell for a sym
   a state cell cache can improve efficiency and reduce costs.
   @param idx : odd frame index ? 
   ---------------------------------------------------------------------- */
static StateCell *
find_state_cell(EvalIdx *eval_idx, unsigned short idx, unsigned short sym, unsigned n_state, gmhmm_type *gv)
{
  short i_pos = -1; 
  StateCell *pstate = NULL; 

  if (sym < gv->sym2pos_map.max_nbr_syms )
    i_pos = gv->sym2pos_map.sym2pos[sym]; 
  if (i_pos >= 0 )
    pstate = STATE_CELL(gv->statecell_base, i_pos, n_state, idx, gv->max_state_cell);

  return pstate;
}

/* ---------------------------------------------------------------------------
   mark backtrace cell as being used (not available)
   ---------------------------------------------------------------------- */

static void mark_cells_state(unsigned short stop_frm, unsigned short best_sym, gmhmm_type *gv,
		      unsigned short last_frm)
{
   BeamType *bm, *beam = gv->beam_base;
   short time;
   unsigned short frm_diff, b_index, cont;   
   TimeType *time_base = gv->time_base;
   
   /*get the correct "time" from time_base, or using the quick access list, so that the stop_frame== time_cell.frm*/
   if ((frm_diff = last_frm - stop_frm ) < NN) {
     time = gv->time_table_idx[frm_diff];
     if (time ==  UNDEFINEDTIME) { // not stored yet, locate time: 
       MATCH_TIME(time, time_base, stop_frm);
       gv->time_table_idx[frm_diff] = time; // store it 
     }
   }
   else MATCH_TIME(time, time_base, stop_frm);
   
   
   
   /* follow the chain to locate the symbol: seach from the beam_index+beam_base for best_sym, store in bm */
  //  MATCH_SYMB(bm,time_base[time].beam_idx, beam, best_sym);
  //// #define MATCH_SYMB(bm, bm0, beam0, best_sym) 
   for (bm = beam + time_base[time].beam_idx; bm->sym != best_sym; )
   {
   	bm = beam + bm->next;
   }

   
   /* backward marking , everying in the past of the beam starting at bm, set cell's idxflag to |8000*/
   BACKWARD_MARKING(bm,b_index,cont, beam,gv->max_beam_cell);
}

/* ---------------------------------------------------------------------------
   mark active beam cells for completed sym
   ---------------------------------------------------------------------- */
static void coloring_beam(unsigned short cur_time_index, gmhmm_type *gv)
{
  BeamType *beam_base = gv->beam_base, *bm, *beam = beam_base;
  unsigned short q, b_index, cont, c_bm_cnt = 0; 
  short depth; 

  if (cur_time_index == USHRT_MAX) return ; /* no beam cell has this time (no sym was collected at this time) */
  for (q =  gv->time_base[cur_time_index].beam_idx; NOT_END_IX_CELL(q,gv->max_beam_cell); q = beam_base[q].next) { 
    bm = beam_base + q;
    depth=0;
    BACKWARD_MARKING(bm,b_index,cont, beam, gv->max_beam_cell);
    c_bm_cnt ++; 
  }
  gv->beam_count=c_bm_cnt;
  gv->cur_beam_count = c_bm_cnt; 

#ifdef DEBUGP
  PRT_ERR(printf ("frame %d, number of beam: %d. symcell pos 0/1 : %d, %d \n", gv->frm_cnt, c_bm_count, gv->evalstat.sym[0].cur_pos,gv->evalstat.sym[1].cur_pos));
#endif

}

/*
** update beamcell cache
*/
static void update_time_table(short p[], unsigned short n, gmhmm_type *gv)
{
  unsigned short i;
  TimeType *time_base = gv->time_base;
  for (i = n - 1; i > 0; i--) {
    if ((p[i-1] != UNDEFINEDTIME) && (time_base[p[i-1]].beam_idx != USHRT_MAX)) p[i] = p[i-1];
    else p[i] =  UNDEFINEDTIME;
  }
  p[0] = UNDEFINEDTIME;
}
  
/* ---------------------------------------------------------------------------
   garbage collect the backtrace cells
   ---------------------------------------------------------------------- */

static void 
compact_beam_cells( BeamType *beam_base, TimeType *time_base,gmhmm_type *gv )
{
  short i, t;
  unsigned short beam_idx, s_next, count=0;
  BeamType *bm = beam_base, *p, *head;

  /*label the end of list, should be put out */
  //if (gv->last_beam_cell != UNDEF_CELL) beam_base[gv->last_beam_cell].next =  gv->max_beam_cell;
  
  /* mark completed symbol (in time cell )*/
  coloring_beam(gv->next_time,gv);

  /* at this point, all active beam cells are marked */
  /* set available(un-marked) to 0x7fff, and un-mark the occupied (marked) */

  for (i = gv->evalstat.beam.high_pos - 1, p = bm + i; i >= 0; p--, i--) { 
    if ( p->idx_flag & 0x8000 ) p->idx_flag &= 0x7fff;   /* marked -- unmark it*/
    else {                                      /* not marked, available */
      p->idx_flag = 0x7fff;                     /* empty */
      gv->evalstat.beam.cur_pos = i;                /* move to lowest empty */
      count++;
    }
  }
    #ifdef DEBUGP
      printf("COMPACT!  beam cell are cut by %d. at frame %d \n", count , gv->frm_cnt);
    #endif
  
  
  /*
  ** update time table (head of symbol list)
  */
  
  for (t = gv->evalstat.time.high_pos - 1; t>=0;  t--) {   //pos means the next available, so pos-1 is the current
   
   
    beam_idx = time_base[t].beam_idx;
    if (beam_idx != USHRT_MAX) { /* there is a list of symbols that follows */
      /* follow the list until the first non empty cell located: (it will become new head of the time table*/
      for (; NOT_END_IX_CELL(bm[beam_idx].next,gv->max_beam_cell); beam_idx = bm[beam_idx].next) 
	if (!CELLEMPTY(bm[beam_idx].idx_flag)) break;

  
      if (CELLEMPTY(bm[beam_idx].idx_flag)) { /* the list contains only empty cells */
	beam_idx = USHRT_MAX;                 /* set the list to NULL */
	gv->evalstat.time.cur_pos = t;            /* lower the curt position for compactness of lower portion */
      }
      else { /* head found, and the list contains at least one cell, update symbol chain: */
      	for (head = bm + beam_idx; NOT_END_IX_CELL(head->next, gv->max_beam_cell); head = bm + head->next) {
	  // find the first active one, or the end 
	  for (s_next = head->next, p = bm + s_next; CELLEMPTY(p->idx_flag) && NOT_END_IX_CELL(p->next,gv->max_beam_cell); s_next = p->next, p = bm + s_next) ;
	  
	  if (CELLEMPTY(p->idx_flag)) { // stopped because of empty cell, that the end of the list 
	    head->next = gv->max_beam_cell;   break;
	  }
	  else head->next = s_next;
	}
	
      }
  

      time_base[t].beam_idx = beam_idx; 
    }
    else gv->evalstat.time.cur_pos = t;
  }
  
}

/* ---------------------------------------------------------------------------
   init the backtrace cells
   ---------------------------------------------------------------------- */

static void 
init_beam_list(gmhmm_type *gv)
{
  gv->evalstat.beam.cur_pos = 0;
  gv->evalstat.time.cur_pos = 0;

#ifdef USE_NBEST
  gv->evalstat.nb_beam.cur_pos = 0;
  gv->evalstat.nb_time.cur_pos = 0;
#endif  
}

/* 
 * locate an empty backtrace cell
 * get next available cell, curt is the last used +1 
 * update top position, if necessary.  -- and gv-> last _beam_cell and the curbeat.next are all updated
 */
static unsigned short next_beam_cell(gmhmm_type *gv)
{
  unsigned short i, N = gv->evalstat.beam.high_pos;
  short found = 0;
  BeamType *beam, *beam_base = gv->beam_base;
//printf("========frame %d beam cell %d  %d \n", gv->frm_cnt,  gv->evalstat.beam.cur_pos,  gv->evalstat.beam.high_pos);

  for (i = gv->evalstat.beam.cur_pos, beam = beam_base + i; i < N; i++, beam++) /*from current to high limit, look for empty one*/
    if (CELLEMPTY(beam->idx_flag)) { found = TRUE;  break; }  //check a flag to see cell empty, or get a new one later

  GET_NEW_CELL(found,i, gv->evalstat.beam, gv->max_beam_cell);
  if (gv->last_beam_cell != UNDEF_CELL) beam_base[gv->last_beam_cell].next = i; /*the link of the current is initialized here*/
  gv->last_beam_cell = i;
  return i;  
}

/*
** return next available time cell
*/

static unsigned short next_time_cell(gmhmm_type *gv)
{
  unsigned short i;
  short found = FALSE;
  TimeType *time = gv->time_base;

  for (i = gv->evalstat.time.cur_pos; i < gv->evalstat.time.high_pos; i++)
    if (time[i].beam_idx == USHRT_MAX) { found = TRUE; break; }
  GET_NEW_CELL(found,i, gv->evalstat.time, gv->max_time_cell);
  return i;  
}

/* ---------------------------------------------------------------------------
   find the beam cell with the specified sym and time
   ---------------------------------------------------------------------- */

/*
** use chained symbol list.
*/
static unsigned short find_beam_index(unsigned short sym, unsigned short frm, gmhmm_type *gv,
			       unsigned short last_frm)
{
  BeamType *bm, *beam = gv->beam_base;
  TimeType *time_base = gv->time_base;
  short time;
  unsigned short  idx; //frm_diff = last_frm - frm,
  
//only if last frame has done mark_cell_state, the cache can be used. removed it from the old code. 
//  if (frm_diff < NN) time = gv->time_table_idx[frm_diff]; /* it must be in, bcs it's back trace */
//  else 
  MATCH_TIME(time, time_base, frm);

  /*from the beam at the old time (before the start of this sym, search for the beam differ from this sym*/
  for (idx = time_base[time].beam_idx, bm = beam + idx; bm->sym != sym; idx = bm->next, bm = beam + idx);
  return idx;
}

/* ---------------------------------------------------------------------------
   grow the backtrace tree by one backtrace cell
 ---------------------------------------------------------------------------*/

static short
grow_beam_cell(unsigned short sym, StateCell *p_cell, short first_time, unsigned short frm_cnt, 
	       gmhmm_type *gv, unsigned short last_frm)
{
  unsigned short idx;

  idx = next_beam_cell(gv); /*get a new beam cell*/
//if (gv->frm_cnt>0 )printf("===================now,word %d \n", sym);
  if ( idx == USHRT_MAX ) 
    return eTIesrEngineBeamCellOverflow;

  gv->beam_base[idx].sym = sym; /* symcode of this cell */

  /* previous frame sym, starting frame, last frame*/
  
  gv->beam_base[idx].idx_flag = (p_cell->sym == USHRT_MAX)?  gv->max_beam_cell  //zlook
          : find_beam_index(p_cell->sym, p_cell->frm, gv, last_frm);  //previous stats' symble and starting frame.


#ifdef _MONOPHONE_NET
  PRT_ERR
    (printf("grow beam: idx %d, sym %d, time %d, trace %d\n", idx, 
	 sym, frm_cnt, gv->beam_base[idx].idx_flag));
#endif

  if (first_time) {
  	
    gv->beam_count=0;
    gv->next_time = next_time_cell(gv);
    if (gv->next_time == USHRT_MAX) 
      return eTIesrEngineTimeCellOverflow;
    gv->time_base[gv->next_time].frm = frm_cnt;
    gv->time_base[gv->next_time].beam_idx = idx; /* this was changed from address to index */
  }

  return eTIesrEngineSuccess;
}

/* ---------------------------------------------------------------------------
   compute Gaussian mixture score
   ---------------------------------------------------------------------- */

/* experimental methods */

//short logcorrect(short logdiff)
//{   short   correcttable[7]={44, 30, 20, 13, 8, 5, 3}; /* 0.69 0.47 0.32 0.2 0.13 0.08 0.05.. for 0 , 0.5, 1, 1.5, 2, 2.5.. -log(b/a) vis log(1+b/a) curve */
//    //    short   correcttable[7]={37, 26, 16, 11, 6, 3, 1}; /* 0.58 0.4 0.25 0.17 0.1 0.05.. for 0 , 0.5, 1, 1.5, 2, 2.5.. -log(b/a) vis log(1+b/a) curve */
//	logdiff>>=5;  logdiff=logdiff>6?6:logdiff; /*Q1*/
//	return correcttable[logdiff];
//}



TIESRENGINECOREAPI_API short
gauss_obs_score_f(short *feature, int pdf_idx, gmhmm_type *gv)
{
  short i, k;
  long  scr;
  short total_scr, nbr_dim = gv->nbr_dim;
  short n_mix;
  short *p2wgt;
  short *mixture;
#ifndef BIT8MEAN
  short j;
  long  diff;
  short *mu, *invvar, gconst, *feat;
  short diff_s;
#endif

  total_scr = BAD_SCR;

  mixture = GET_MIX(gv,pdf_idx);
  
  n_mix = MIX_SIZE(mixture);
  p2wgt = mixture + 1;

  for (i = 0; i < n_mix; i++) {      /* n mixtures */
    k = mixture[i*3 + 2]; 
    if (gv->gauss_scr[ k ] == LZERO )
      {
   
#ifndef BIT8MEAN
	mu = get_mean(gv->base_mu, mixture, nbr_dim, i);   
	invvar = get_var(gv->base_var, mixture, nbr_dim, i);
	gconst = gv->base_gconst[ mixture[ i * 3 + 3 ] ];
	feat = feature;
            
	GAUSSIAN_DIST(gv, scr, feat, j, nbr_dim, diff, diff_s, gconst, mu, invvar);
#else
	GAUSSIAN_DIST(gv, scr, nbr_dim, mixture, i, feature);
#endif

	gv->gauss_scr[ k ] = LONG2SHORT(scr); 

      }
    else
      {
	scr=(long) gv->gauss_scr[ k ] ;
      }	 

    scr += p2wgt[ i * 3 ];         /* mixture weight */
   
    if ( scr > (long) total_scr ) total_scr = LONG2SHORT(scr);
    
  }

  return total_scr;
}

/*-----------------------------------------------------------------------
 search_if_comes_to_the_end_of_net
 
 If enough utterance frames have been processed, determine if sym_index
 argument is the index of a stop node of the grammar network. This indicates
 that it is possible that there is a valid end of the grammar.

 sym_index: A grammar symbol index
 gv: recognizer instance structure
 
 ----------------------------------*/
static Boolean search_if_comes_to_the_end_of_net(short sym_index, gmhmm_type *gv)
{
  SymType *stop_sym;
  short i;
  
  /* Do not check if end of grammar is possible if fewer than a 
   minimum number of frames have been processed or possible end of grammar
   is already declared */
  if( gv->frm_cnt < TH_END_GRAMMAR_MIN_END_FRM || gv->sr_is_end_of_grammar )
     return gv->sr_is_end_of_grammar;

  
  /* Search stop symbol index list; is sym_index that of a stop symbol? */
  stop_sym = (SymType *) ( gv->base_net + gv->trans->stop );
  for( i = 0; i < GET_N_NEXT( stop_sym->n_next); i++ )
  {
     /* If sym_index is stop symbol index, then grammar end is possible */
     if( sym_index == stop_sym->next[i] )
     {
        gv->sr_is_end_of_grammar = TRUE;
        return TRUE;
     }
  }
  
  return FALSE;
}


/*--------------------------------------------------------------------------
 init_search_last_syms
 
 Initialize end-of-grammar Boolean that declares that the utterance could 
 have reached the end of the grammar.
------------------------------------*/
static void init_search_last_syms( gmhmm_type * gv)
{

  gv->sr_is_end_of_grammar = FALSE; 

}

/* ---------------------------------------------------------------------------
   search expansion within HMM
   ---------------------------------------------------------------------- */

static void
within_model_path(HmmType *hmm,  StateCell *p_prev,  StateCell *p_cur, gmhmm_type *gv, short prune)
{
  long       cur_prob;
  short      *trnprob;
  short      i, j;
  StateCell  *state_cell, *pprev;
  short      n_state, n1;

  n_state = NBR_STATES(hmm,gv->base_tran);
  trnprob = gv->base_tran + hmm->tran + n_state;
  n1 = n_state - 1;
  pprev = p_prev;
  for (i = 0; i < n1; i++, trnprob += n_state, pprev++) {  /*for all states in the HMM*/

    if ( pprev->scr > prune ) {  /*if a possible start -- within model prune*/
      
      for (j = 0; j < n1; j++) {   /*for all target states within model*/

	if ( trnprob[j] > BAD_SCR ) {          /* there exist i to j jump. note transprob is updated above with i */

	  state_cell = p_cur + j;
	  cur_prob = (long) pprev->scr + (long) trnprob[j];
	  
	  if ( cur_prob > (long) state_cell->scr ) {  //viterbi on transition prob.
	    state_cell->scr = (short) cur_prob; 
	    state_cell->sym = pprev->sym;   /* propogate prev sym & frm */
	    state_cell->frm = pprev->frm;  /*starting time, B*/
	  }
	}
      }
    }
  }
} 

/* ---------------------------------------------------------------------------
   do the within and cross model transition
   ---------------------------------------------------------------------- */

#define SET_NEXT_SYM(sym_cell,ps,direction, nbr) {sym_cell = ps; if ( --(nbr) ) ps += direction; else ps = NULL; }

#define SET_SYM_CELL(ps0,ps1,sym_cell,nbr0,nbr1,direction, expr) \
    if (!ps0) SET_NEXT_SYM(sym_cell,ps1,direction, nbr1) \
      else if (!ps1) SET_NEXT_SYM(sym_cell,ps0,direction, nbr0) \
           else if (expr) SET_NEXT_SYM(sym_cell,ps0,direction, nbr0) \
                else SET_NEXT_SYM(sym_cell,ps1,direction, nbr1)
/*update path from t to t+1*/

static short 
cross_model_path(short exit_scr, unsigned short v_sym, unsigned short t_sym, 
		 unsigned short hmm_dlt, unsigned short sym_dlt, short cur_idx, 
		 unsigned short frm_cnt, gmhmm_type *gv, short depth, 
		 unsigned short starttime )
{
  unsigned short i, symcode, hmmcode, n_state, nbr_next;
  SymType        *prev_sym, *psym;
  HmmType        *hmm;
  StateCell      *start_cell;
  long           start_scr;
  short          *startprob, *endprob;
  EvalIdx *eval_idx = gv->eval + cur_idx;
  prev_sym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_sym ] );
#ifdef USE_NBEST
  ushort uBeamIdx, cSym; 
  bIsWord = is_word_sym(prev_sym);
#endif
  
  
  /*number for next possible symbols*/
  nbr_next = GET_N_NEXT(prev_sym->n_next);
//printf("000 sym: %d , n next %d\n", t_sym, nbr_next);  
  
  for (i = 0; i < nbr_next; i++) {  /*for all posible next symbols*/
    symcode = prev_sym->next[ i ];  /*pick one on the next-list*/
    
    // get next symble
       
    psym = (SymType *) ( gv->base_net + gv->trans->symlist[ symcode ] );
    hmmcode = psym->hmm_code;
    hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt);   //using hmm_dlt to ensure the same gender

    n_state = NBR_STATES(hmm,gv->base_tran);
    /* start state cell may already exist, bcs it may have two predecessors and one already allocated: */
    start_cell = find_state_cell(eval_idx, cur_idx, VTL_SYM(symcode, sym_dlt), n_state, gv);

    /*alocate sym and state cells for this new sym -- using sym_dlt to ensure sym of the same gender */
    if ( ! start_cell ) 
    {
      start_cell = allocate_eval_cell(eval_idx, VTL_SYM(symcode, sym_dlt), n_state, cur_idx,gv,depth+1);  //last 1: grow

    }
    if ( ! start_cell ) 
      return eTIesrEngineEvalCellOverflow;


    startprob = GET_PI(hmm, gv->base_tran);
    endprob = startprob + n_state - 1;

#ifdef USE_NBEST
    uBeamIdx = UNDEF_CELL;
#endif
    for (; startprob < endprob; startprob++, start_cell++) { /* itr through initial probs */

      if ( *startprob > BAD_SCR ) {  //if it can be a initial state

	start_scr = (long) exit_scr + (long) *startprob;
#ifdef USE_NBEST
	if (bIsWord && gv->bUseNBest ){
	  /* obtain the nbest index for the current symbol at the time */
	  cSym =  VTL_SYM(symcode, sym_dlt);
	  if (uBeamIdx == UNDEF_CELL || 
	      uBeamIdx == gv->max_nb_beam_cell) 
	    uBeamIdx = nb_find_nbest_index(cSym, frm_cnt - 1, gv); 
	  
	  nb_update_nbest(gv->ptr_nbest_tmp->pNBestCells, uBeamIdx, start_cell, v_sym, cSym,  
			  frm_cnt - 1, start_scr, 
			  start_scr >= (long) start_cell->scr, gv); 
	}
#endif
	if ( start_scr >= (long) start_cell->scr ) { 
	  /* Viterbi, for the winner,
	     remember the frame index and the symbol where we are from */
	  start_cell->scr = (short) start_scr;
	  start_cell->frm = starttime; //frm_cnt - 1;  // this is true for HMM backtrace. not word based backtrace.
	  start_cell->sym = v_sym;  // word sym


	}
#ifdef USE_NBEST
	if (gv->bUseNBest) nb_uniq_nbest(gv->ptr_nbest_tmp->pNBestCells, uBeamIdx, start_cell, gv); 
#endif
      }
    }
  }

    
  return eTIesrEngineSuccess;
}

static short
path_propagation(short cur_idx, short prev_idx, unsigned short frm_cnt, gmhmm_type *gv, short prune)
{
   unsigned short v_sym, t_sym, hmmcode, sym_dlt, hmm_dlt;
   SymCell        *sym_prev, *ps0 = NULL, *ps1 = NULL;
   HmmType        *hmm;
   short          exit_prob,    status, n_state;
   StateCell      *p_prev, *p_cur;
   SymType        *psym;
   unsigned short *nbr_sym =  gv->eval[ prev_idx ].cur_sym;   /*set current sym.. and call it nbr_sym*/
   EvalIdx *eval_idx = gv->eval + cur_idx;
   /* loop through all previous sym cells in the list*/
   short direction = cur_idx ? 1 : -1;
  
   /*pointers from top or bottom of the sym space, */
   if (nbr_sym[0]) ps0 = SYM_CELL(gv->symcell_base, nbr_sym[0] - 1, prev_idx, gv->max_sym_cell2);
   if (nbr_sym[1]) ps1 = SYM_CELL(gv->symcell_base + gv->max_sym_cell2, nbr_sym[1] - 1, prev_idx,gv->max_sym_cell2 );
#ifdef DBG_SEARCH
   printf("sym number %d===========\n" ,nbr_sym[0]+nbr_sym[1]);
   global_active_sym=0;
#endif
   
   if ((!ps0) && (!ps1)) {
     return eTIesrEngineAlignmentFail; 
   }


   while (ps0 || ps1) { 

     /*
       --------- search through all symbol cells --------
       give value to sym_prev = ps0 or ps1, and [move] one of ps0 and ps1 by 1 or -1, based on the direction 
       NOTE, but the gv->eval[ prev_idx ].cur_sym, and the PS? will be changed: -1 and next*/
     SET_SYM_CELL(ps0,ps1,sym_prev,nbr_sym[0],nbr_sym[1],direction, ps0->start_cell > ps1->start_cell); 
     /*for the chosen direction: give the ps? to sym_prev, and move the ps*/
     if ( !( sym_prev->sym_code & 0x8000 ) ) {   /* active */
#ifdef DBG_SEARCH
       global_active_sym++;
#endif
       v_sym = sym_prev->sym_code;  

      /* lope only max 2 times. Move pointers (t_sym stuff)  relative to 'lower gender', 
      a  translater from gender dependent symcode to gender indep symtype code in GRAMAR NETWORK*/
       for (hmm_dlt = 0, sym_dlt = 0, t_sym = v_sym; t_sym >= gv->trans->n_sym;
	    t_sym  -= gv->trans->n_sym,  hmm_dlt  += gv->trans->n_hmm_set, sym_dlt  += gv->trans->n_sym) ;
	    

       psym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_sym ] );
       hmmcode = psym->hmm_code;
       hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt);
       
       

       n_state = NBR_STATES(hmm,gv->base_tran);
       /* ------- ABOVE: it find hmm from gender-dependant symble, to gender indep sym in network and find the HMM.. and number of state. a lot of work! */


       /*space from top or bottom, n_state is only used in higher memory part.*/
       p_prev = STATE_CELL(gv->statecell_base, sym_prev->start_cell, n_state, prev_idx, gv->max_state_cell);
   
       /*find state cell of v_sym from space upto eval_idx->current_sym. (it may exist in previous idx,but not cur idx). */
       //Note: p_prev is from the old space, and p_cur is for th new space -- it may be there or not. If not, make new.
       //can be made faster: at first, all the search will sure fail.
       p_cur = find_state_cell(eval_idx, cur_idx, v_sym, n_state, gv); 

//p_cur=NULL;
       
       /*now we get the start state_cell for this frame and last frame */
       
       /*      if now entering a new sym, alocate mem for sym and state cells*/
       if ( ! p_cur ) {
       	/*these syms are just from last frame*/
       	
        
       	p_cur = allocate_eval_cell(eval_idx, v_sym, n_state, cur_idx, gv,sym_prev->depth);
       }
        

       if ( p_cur == NULL ){
	 return eTIesrEngineEvalCellOverflow;
       }

       /*label all the within model path, if score is OK, prune is OK, */
       within_model_path(hmm, p_prev, p_cur, gv, prune); /*hmm, last state cell, new allocated state cell, ...*/
        
       exit_prob = p_prev[ n_state - 1 ].scr;

#ifdef  DYNAMIC_PRUNE
       if ( exit_prob > prune + SYMEND_PRUNE_DELTA +WORDEND_PRUNE_DELTA ) { //cross symbol prune , +400 (check word end)
#else
       if ( exit_prob > prune ) {
#endif	 
	 /* exit of current HMM, connecting to next HMM */

	 // if also word transition, gives the new ending word  to v_sym, otherwise, uses the old one 
// old:	  status = cross_model_path(exit_prob + gv->tranwgt, v_sym, t_sym, hmm_dlt, sym_dlt, cur_idx, frm_cnt,gv, sym_prev->depth);
	
	 unsigned short starttime;

	if (gv->word_backtrace==WORDBT){ 

	 if (psym->n_next & 0x8000)  
	 {

	  // unsigned short word_sym = gv->trans->symlist[ t_sym ]; //psym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_sym ] );psym->next[GET_N_NEXT(psym->n_next)];//zx
	    starttime= frm_cnt - 1;  //time of start of new word. 
	  //status = cross_model_path(exit_prob + gv->tranwgt, word_sym, t_sym, hmm_dlt, sym_dlt, cur_idx, frm_cnt,gv, sym_prev->depth, starttime);
	  status = cross_model_path  (exit_prob + gv->tranwgt, v_sym,    t_sym, hmm_dlt, sym_dlt, cur_idx, frm_cnt,gv, sym_prev->depth, starttime);

	 }
	 else     // p_prev->sym will be the word info of last sym
	 { starttime = p_prev[ n_state - 1 ].frm;
	   status = cross_model_path(exit_prob + gv->tranwgt, p_prev[n_state -1].sym, t_sym, hmm_dlt, sym_dlt, cur_idx, frm_cnt,gv, sym_prev->depth,starttime);
	 
	  if ( status != eTIesrEngineSuccess )  
	    return status;
	 }
	 
	}
	else //hmm based BT
	{
	   status = cross_model_path(exit_prob + gv->tranwgt, v_sym, t_sym, hmm_dlt, sym_dlt, cur_idx, frm_cnt,gv, sym_prev->depth, frm_cnt-1);
	}
	 
	 
	 if ( status != eTIesrEngineSuccess )  
	   return status;

	 search_if_comes_to_the_end_of_net(t_sym, gv);

       }
       
     }//end of active loop
     gv->eval[ prev_idx ].cur_state = sym_prev->start_cell;   /* (keep curt lower level). change the state_indx for a new sym in the loop */

   }  //end of while loop

#ifdef DBG_SEARCH
     printf("active syms = %d\n", global_active_sym);
#endif

   if ( gv->eval[ prev_idx ].cur_state != 0 ) {  // needed?
     return eTIesrEngineAlignmentFail;
   }
   
#ifdef DBG_HMM
   printf("--------active sym %d , total HMM: %d  , %d \n", global_active_sym,  gv->trans->n_hmm, gv->trans->stop);
#endif

   return eTIesrEngineSuccess;
}

/* ---------------------------------------------------------------------------
   add obs score 
   ---------------------------------------------------------------------- */
static short
  update_obs_prob(short mfcc_feature[], short cur_idx, unsigned short frm_cnt, gmhmm_type *gv, unsigned short last_frm)
{
  unsigned short t_symcode, v_symcode, hmmcode, j, pdf_idx, hmm_dlt, n1;
  unsigned short nbr0, nbr1;  
  SymCell        *sym_cell;
  SymType *psym;
  SymCell *ps0 = NULL, *ps1 = NULL;  
  HmmType        *hmm;

  short          gs,  sym_active, direction, status, n_state, *trnprob, max_exit_scr, first_time_call = TRUE, *sObs = NULL;

#ifdef USE_GAUSS_SELECT
  short iargmix;
#endif

  long           scr;
  StateCell      *max_exit_state = NULL, *state_cell, *start_cell;
  unsigned short last_SYM = USHRT_MAX, last_FRM = USHRT_MAX;
  unsigned short bestdepth = 0, n_actsym=0,sumdepth=0, maxdepth=0;
  
#ifdef SHOW_DIAGONISIS
  if (sttnode!=3) sttnode=0;
#endif

#ifdef USE_CONFIDENCE
  short *p_scr_anti, *pScrAnti = cm_antiscr((ConfType*)gv->pConf, frm_cnt); 
  short     /* for cache */
    cm_prev_scr = LZERO; 
  if (((ConfType*)gv->pConf)->bNeedUpdate == FALSE) 
    pScrAnti = NULL; 
#endif

  direction = cur_idx ? 1 : -1;
  
  gv->next_time = USHRT_MAX;

  nbr0 = gv->eval[ cur_idx ].cur_sym[0];
  nbr1 = gv->eval[ cur_idx ].cur_sym[1];

  gv->ending_sym_count =0;

  
  if (nbr0) ps0 = SYM_CELL(gv->symcell_base, 0, cur_idx, gv->max_sym_cell2);
  if (nbr1) ps1 = SYM_CELL(gv->symcell_base+ gv->max_sym_cell2, 0, cur_idx, gv->max_sym_cell2);
  
  

  while ( ps0 || ps1 ) {  /*loop through all previous (later check active) syms   --- Loop1*/
      SET_SYM_CELL(ps0,ps1,sym_cell,nbr0,nbr1,direction,ps0->start_cell < ps1->start_cell); 
      v_symcode = sym_cell->sym_code; 
      
      for ( hmm_dlt = 0, t_symcode = v_symcode; t_symcode  >= gv->trans->n_sym; 
	    t_symcode -=  gv->trans->n_sym, hmm_dlt +=  gv->trans->n_hmm_set); 
      psym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_symcode ] );
      

      hmmcode = psym->hmm_code;

#ifdef SHOW_DIAGONISIS
      if (sttnode!=3)
	if ( hmmcode==0 && sym_cell->depth <2 ) sttnode=1; 
#endif

      hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt);
      
#ifdef USE_SBC
      if (gv->comp_type & SBC) 
	/* notice that only true symbol therefore the smaller hmmcode is used */
	sObs = sbc_enhance( gv->sbcState, mfcc_feature, 
			    hmmcode, gv->nbr_dim, NULL );
#endif 
      n_state = NBR_STATES(hmm,gv->base_tran);
      
      start_cell = STATE_CELL(gv->statecell_base, sym_cell->start_cell, n_state, cur_idx, gv->max_state_cell);

      sym_active = 0;
    
      max_exit_scr = BAD_SCR;
    
      n1 =  n_state - 1;
      trnprob = gv->base_tran + hmm->tran + n_state + n1;      

      for (state_cell = start_cell, j = 0; j < n1; state_cell++, j++, trnprob += n_state) {  /* for all emit state */
      
#ifdef DYNAMIC_PRUNE_DEPTH
// #ifdef DYNAMIC_PRUNE
	if ( state_cell->scr > BAD_SCR  && 
	     ( frm_cnt < 6 || ( sym_cell->depth + (sym_cell->depth >> 1) < frm_cnt ) ) )	  
#else
	if ( state_cell->scr > BAD_SCR )
#endif
	{
	  /* a sym is active if any of the state is active */
	  sym_active = 1;
	
	  pdf_idx = hmm->pdf_idx[ j ];
#ifdef USE_GAUSS_SELECT
         if (gv->word_backtrace==HMMFA)  //somehow the G_selection has error on the second pass
	  gs = 
	    gauss_obs_score_f(sObs?sObs:mfcc_feature, pdf_idx, gv); 
	 else
	  gs = 
	    rj_gauss_obs_score_f(sObs?sObs:mfcc_feature, pdf_idx, gv, NULL, &iargmix);
	    
//printf("=== gs %d, code: %d, pdf %d feat: %d %d %d\n", gs, v_symcode, pdf_idx, sObs[0], sObs[1], sObs[18]);	   
#else
	  gs = 
	    gauss_obs_score_f(sObs?sObs:mfcc_feature, pdf_idx, gv); 
#endif   
	 

#ifdef USE_CONFIDENCE
	  if ((gs > cm_prev_scr) && (pScrAnti != NULL)){
	    p_scr_anti =  pScrAnti; 

	    if (p_scr_anti){
	      *p_scr_anti = gs; 
	    }
	    
	    cm_prev_scr = gs; 
	  }
#endif

	 scr = (long) state_cell->scr + (long) gs; //gauss_obs_score_f(sObs?sObs:mfcc_feature, pdf_idx, gv); 

	   
	//if (sym_cell->depth *1.5  - gv->frm_cnt>0) scr-= 500;  // soft prune
	//if ( sym_cell->depth > (gv->bestdepth +9) || sym_cell->depth < (gv->bestdepth -10)  ) scr-=500;
	  scr -= (long) gv->best_prev_scr;

	  if ( scr < BAD_SCR ) state_cell->scr = BAD_SCR;
	  else {
	    state_cell->scr = (short) scr;
	    /* coloring pending states (including exit states):
	     * mark active beam cells for active pending states
	     */
	    if (state_cell->frm != USHRT_MAX) 
	      if ((state_cell->frm != last_FRM) || (state_cell->sym != last_SYM)) {  //if it is from a new search node (frame/sym pair)
		/*pass best frame/sym to this state so far*/
		if (gv->need_compact==TRUE ) //|| gv->word_backtrace==FALSE) //if (gv->evalstat.beam.cur_pos > 1000)   //if (gv->frm_cnt==14)
		  {   
		  	mark_cells_state(state_cell->frm, state_cell->sym, gv, last_frm);   
		  }   
		last_FRM = state_cell->frm; last_SYM = state_cell->sym; 
	      }
	  }

	  if ( scr > (long) gv->best_cur_scr ) 
	    {
	      gv->best_cur_scr = (short) scr;
//if (frm_cnt<9) printf ("=========curbest %d\n",gv->best_cur_scr);
	      bestdepth= sym_cell->depth  ;
	    }

#ifdef MODEL_LEVEL_PRUNE
	  mlp_update(gv, hmmcode , scr);
#endif
	
	  if ( *trnprob > BAD_SCR ) {  /* exit? */

	    scr = (long) state_cell->scr  +  (long) *trnprob;
	  
	    if ( scr > (long) max_exit_scr) { 
	      max_exit_scr = (short) scr;
	      max_exit_state = state_cell;
	    }	
	  }
	}
      }  /* end for all emit states */

      if ( ! sym_active )	sym_cell->sym_code |= 0x8000;      /* inactive, prune the sym */
      else { /* exit state */
	sumdepth+=sym_cell->depth;  n_actsym++;      
        maxdepth=sym_cell->depth> maxdepth?sym_cell->depth: maxdepth;
            
	if ( max_exit_scr > BAD_SCR ) {
	  state_cell->scr = max_exit_scr;
	  state_cell->sym = max_exit_state->sym;
	  state_cell->frm = max_exit_state->frm;
	
	  /*if end of sym, put it in the beam cell list*/
	  // word end label, grow beam cell
	  gv->ending_sym_count ++;
	  if (psym->n_next & 0x8000 || gv->word_backtrace!=WORDBT )
	  {
	    status = grow_beam_cell( v_symcode, state_cell, first_time_call, frm_cnt, gv, last_frm);
	    
	    if ( status != eTIesrEngineSuccess ) 
	      return status;
	    first_time_call = FALSE;
          }
	  

	  
	  if ( (long) state_cell->scr > gv->best_sym_scr ) {    /*best_sym_scr update for the last frame*/
	    gv->best_sym_scr = (long) state_cell->scr; 
	    gv->best_sym = v_symcode; 
	  }
	} else  state_cell->scr = BAD_SCR;       /* active sym, but not complete yet */
      }
  }

#ifdef SHOW_DIAGONISIS
    if (sttnode==0 && frm_cnt>1) 
      {/*printf("  %d frame, sttnode exist\n", frm_cnt);*/ sttnode=3;
      }
#endif

    //gv->avedepth=sumdepth/n_actsym; 
    gv->bestdepth=bestdepth;
    
    //lable the end (was in compact beam)
    if (gv->last_beam_cell != UNDEF_CELL) gv->beam_base[gv->last_beam_cell].next =  gv->max_beam_cell;
    
#ifdef DEBUGP
    printf("total sym: %d, active %d , BEST DEPTH: %d, ave depth: %f , max depth %d\n ", ccc, n_actsym, gv->bestdepth, sumdepth*1.0/n_actsym , maxdepth);
#endif


  return eTIesrEngineSuccess;
}

/* ---------------------------------------------------------------------------
   init search
   ---------------------------------------------------------------------- */

static short
init_search(short mfcc_feature[], short cur_idx, gmhmm_type *gv, unsigned short last_frm)
{
  unsigned short t_symcode, hmmcode, exit_state, sym_dlt, hmm_dlt;
  HmmType        *hmm;
  short          i, j;
  StateCell      *state_cell;  
  short          gs, max_exit_scr, *p_enh = NULL;
  long           scr;
  SymType        *psym, *startsym;
  short          status, n_state, pdf_idx, *startprob, *trnprob, first_time_call = TRUE;
  EvalIdx  *eval_idx = gv->eval+cur_idx;

  
#ifdef SHOW_DIAGONISIS
  sttnode=0;
  global_count=0;
#endif

#ifdef DBG_SEARCH
  max_glb_active_sym= 0;
#endif

  gv->ending_sym_count =0;
  gv->bestdepth=0;

  /* expand all fanout after start node (of the top level grammar) */
  startsym = (SymType *) ( gv->base_net + gv->trans->start );

  gv->beam_count=0;//useless
 
  for (hmm_dlt = 0, sym_dlt = 0; hmm_dlt < gv->trans->n_hmm; hmm_dlt += gv->trans->n_hmm_set, sym_dlt+= gv->trans->n_sym)   { /*e.g.: for two genders. */
  //printf("init===gender %d... nhmm %d   hmset %d \n", hmm_dlt, gv->trans->n_hmm, gv->trans->n_hmm_set );
    
    //skip gender for FA. 
    if (gv->word_backtrace==HMMFA &&  hmm_dlt!= gv->best_word_gender  ){ continue; }
     
    for (i = 0, t_symcode = startsym->next[ i ]; 
	 i < GET_N_NEXT(startsym->n_next); 
	 i++, t_symcode = startsym->next[ i ]) {  /*for all the current active hypothesis (symbols) */

      max_exit_scr = BAD_SCR;

      psym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_symcode ] ); /*take one posible symbol*/
      hmmcode = psym->hmm_code;
      hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt); /*using hmm name, gender info, and the base to get the offset for the HMM. */

#ifdef USE_SBC
      if (gv->comp_type & SBC) 
	p_enh = sbc_enhance( gv->sbcState, mfcc_feature, 
			     hmmcode, gv->nbr_dim, NULL);
#endif


      n_state = NBR_STATES(hmm, gv->base_tran);
      exit_state = n_state - 1;
      startprob = GET_PI(hmm,gv->base_tran);
      trnprob = gv->base_tran + hmm->tran + n_state;

      state_cell = allocate_eval_cell(eval_idx, VTL_SYM(t_symcode,sym_dlt), n_state, cur_idx, gv,0);
    
      if ( state_cell == NULL ) {
	return eTIesrEngineEvalCellOverflow;
      }

      for (j = 0; j < n_state - 1; j++) {          /* for all emit states . */

	if ( startprob[ j ] > BAD_SCR ) {          /* locate inititial stateS */

	  pdf_idx = hmm->pdf_idx[ j ];

	/*  if (gv->obs_scr[ pdf_idx ] == 0 )  // a cache in computing. 
	    gv->obs_scr[ pdf_idx ] = 
	      gauss_obs_score_f(p_enh?p_enh:mfcc_feature, pdf_idx, gv);

	  scr = (long) startprob[ j ] + (long) gv->obs_scr[ pdf_idx ];
	*/
	  gs=gauss_obs_score_f(p_enh?p_enh:mfcc_feature, pdf_idx, gv);

	  scr = (long) startprob[ j ] + (long) gs; //gauss_obs_score_f(p_enh?p_enh:mfcc_feature, pdf_idx, gv);

	  if ( scr < BAD_SCR ) state_cell[ j ].scr = BAD_SCR;
	  else state_cell[ j ].scr = (short) scr;

	  if ( scr > (long) gv->best_cur_scr ) 
	    {
	    gv->best_cur_scr = (short) scr;  /*first frame only*/
	    
	    }

#ifdef MODEL_LEVEL_PRUNE
	  mlp_update(gv, hmmcode , scr);
#endif

	  if ( trnprob[ j * n_state + exit_state ] > BAD_SCR ) {
	    scr = (long) state_cell[ j ].scr + 
	      (long) trnprob[ j * n_state + exit_state ];

	    if ( scr > (long) max_exit_scr )
	      max_exit_scr = (short) scr;   /*track the best score to exit the sym*/
	  }
	}
	state_cell[ j ].frm = USHRT_MAX;     /*the 4 lines below say : there is no prior sym and frame*/
	state_cell[ j ].sym = USHRT_MAX;
      } /* end of all emiting states */

      state_cell[ exit_state ].frm = USHRT_MAX; 
      state_cell[ exit_state ].sym = USHRT_MAX;

      /* HMM (sym) completed, create a backtrace cell */

      if ( max_exit_scr > BAD_SCR ) {   /* if can be end of symbol, but sure not for first frame. so why here ? */
	state_cell[ exit_state ].scr = max_exit_scr;
	
	
	
	//status = grow_beam_cell(VTL_SYM(t_symcode, sym_dlt), state_cell + exit_state, first_time_call, 0, 
	//			gv, last_frm);
	gv->ending_sym_count ++;
	if (psym->n_next & 0x8000 || gv->word_backtrace!=WORDBT)  //never should happen in word bt
	  {
	 	   status = grow_beam_cell(VTL_SYM(t_symcode, sym_dlt), state_cell + exit_state, first_time_call, 0, 
				gv, last_frm);
	           first_time_call = FALSE;   //first time to call beam cell stuff -- for this frame!
	     gv->beam_count++;
	     if ( status != eTIesrEngineSuccess ) 
	       return status; 	  
          }
	
	
	
	
      }
    }
  }


  if (gv->last_beam_cell != UNDEF_CELL) gv->beam_base[gv->last_beam_cell].next = gv->max_beam_cell;   
#ifdef USE_NBEST
  if (gv->bUseNBest && gv->last_nbest_cell != UNDEF_CELL) 
    gv->nb_beam_base[gv->last_nbest_cell].next = gv->max_nb_beam_cell;   
#endif

#ifdef DEBUGP
  PRT_ERR(printf("at first, number of beam %d \n", gv->beam_count ));
#endif

  init_search_last_syms(gv);
  
  return eTIesrEngineSuccess;

  
}

/* ---------------------------------------------------------------------------
   find the best score sym and its score
   called at the end of utterance to report result
   ---------------------------------------------------------------------- */
/* 
** this may be redundent, will test further when I work on PMC
*/
static void
find_best_sym(short idx, gmhmm_type *gv)
{
  unsigned short hmmcode, symcode, hmm_dlt, sym_dlt; 
  unsigned short i;
  long           scr;
  StateCell      *state_cell;
  HmmType        *hmm;
  SymType        *stopsym, *psym;
  short          n_state, bsymc;

  if (gv->word_backtrace==WORDBT){	  
    gv->best_word = USHRT_MAX;
    gv->best_word_frm = USHRT_MAX; 
    gv->best_sym = USHRT_MAX; 
  }

  stopsym = (SymType *) ( gv->base_net + gv->trans->stop );
  for (i = 0, symcode = stopsym->next[ i ]; 
       i < GET_N_NEXT(stopsym->n_next); 
       i++, symcode = stopsym->next[ i ]) {      /* for all stop nodes */
    for (hmm_dlt = 0,sym_dlt = 0; hmm_dlt <gv-> trans->n_hmm;  hmm_dlt += gv->trans->n_hmm_set, sym_dlt += gv->trans->n_sym)  
    {
      psym = (SymType *) ( gv->base_net + gv->trans->symlist[ symcode ] );  //get the basic HMM
      hmmcode = psym->hmm_code;
      hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt);

      n_state = NBR_STATES(hmm,gv->base_tran);
      
      state_cell = find_state_cell(gv->eval+idx, idx, VTL_SYM(symcode, sym_dlt), n_state, gv);

      if ( state_cell && state_cell[ n_state - 1 ].scr > BAD_SCR ) {
	/* start state cell */

	scr = (long) state_cell[ n_state - 1 ].scr  + (long) gv->tranwgt;

	if ( gv->best_sym_scr < scr ) {  
	  gv->best_sym_scr = scr;  
	  gv->best_sym = VTL_SYM(symcode,sym_dlt);
          
          if (gv->word_backtrace==WORDBT){	  
	    bsymc=symcode;
	    if (psym->n_next & 0x8000)   // sym is a word end   //just added sym in growbeam
	    {
	      gv->best_word= VTL_SYM(symcode, sym_dlt);   //t_sym to word	
	      gv->best_word_frm=gv->frm_cnt-1;
	    }
	    else
	    {
	     gv->best_word=state_cell[ n_state - 1 ].sym;   //1
	     gv->best_word_frm=state_cell[ n_state - 1 ].frm;      //2
	    }
	    gv->best_word_gender=hmm_dlt;
          }

	}
      }
    }
  }
  
  if (gv->nbest.n_bests == 0) 
    gv->nbest.n_bests = 1; 
#ifdef DBG_HMM  
  printf("---------best word: %d, best word starging frame: %d, bestsym%d\n", gv->best_word, gv->best_word_frm, gv->best_sym );  
#endif
}

/* return TRUE if the input is the different from the elm
          if FALSE, the score is kept with their maximum. */
static Boolean nb_uniq_each(ushort *answers, short n_ans, short scr, NB_element * elm)
{
  short i; 

  if (elm->n_ans != n_ans)
    return TRUE; 
  else{
    for (i=0;i<n_ans; i++)
      if (elm->answers[i] != answers[i]) return TRUE; 
    elm->scr = MAX(scr, elm->scr); 
    return FALSE; 
  }
}

static TIesrEngineStatusType nb_uniq(ushort * answers, short n_ans, short scr, 
				     NB_words* nbest)
{
  short i; 
  short iFound = 0; 
  
  for (i=0;i<nbest->n_bests;i++){
    iFound += nb_uniq_each(answers, n_ans, scr, &nbest->elem[i]);
  }

  if (iFound == nbest->n_bests){
    /* need to save the new hypothesis */
    if (nbest->n_bests >= MAX_N_BESTS)
      return eTIesrEngineNBestMem; 

    nbest->elem[nbest->n_bests].scr = scr; 
    nbest->elem[nbest->n_bests].n_ans = n_ans; 
    for (i=0;i<n_ans; i++)
      nbest->elem[nbest->n_bests].answers[i] = answers[i]; 
    nbest->n_bests++; 
  }
  return eTIesrEngineSuccess;
}

/* use bubble sorting, assuming that each element is unique from others */
static void nb_sort(NB_words* nbest)
{
  NB_element *m, *n;
  short i, j, l, a;
  short* best_idx = nbest->best_idx; 

  j = nbest->n_bests;
  for (i=0;i<j;i++)
    best_idx[i] = i; 

  for (j=nbest->n_bests;j>0;j--){
    for (i=1;i<j;i++){
      l = best_idx[i-1];
      m = &nbest->elem[l];
      
      l = best_idx[i]; 
      n = &nbest->elem[l]; 
      if (n->scr > m->scr){
	/* switch */
	a = best_idx[i];
	best_idx[i] = best_idx[i-1];
	best_idx[i-1] = a; 
      }
    }
  }

}

#ifdef SHOW_NBEST
static short nb_nbr_nbests(NB_words * nbest)
{
  return nbest->n_bests; 
}

static ushort * nb_nbests(NB_words *nbest, short idx, short * nbr_ans)
{
  short i; 

  if (idx >= 0 && idx < nbest->n_bests)
    {
      i = nbest->best_idx[idx]; 
      *nbr_ans = nbest->elem[i].n_ans; 
      return nbest->elem[i].answers;
    }
  return NULL; 
}
#endif

/* find symbols with good scores that are close to the best symbol. 
   the good symbols include the best symbols, from which the top best path is located.
   if assuming tree-structured grammar, backtracing the paths of these good 
   symbols can loate good paths, from which N-best paths can be obtained
*/
static TIesrEngineStatusType
find_good_syms(short idx, gmhmm_type *gv)
{
  unsigned short hmmcode, symcode, hmm_dlt, sym_dlt; 
  unsigned short i, bword, bword_frm;
  long           scr;
  StateCell      *state_cell;
  HmmType        *hmm;
  SymType        *stopsym, *psym;
  short          n_state, bsymc;

  if (gv->word_backtrace==WORDBT){	  

    stopsym = (SymType *) ( gv->base_net + gv->trans->stop );
    for (i = 0, symcode = stopsym->next[ i ]; 
	 i < GET_N_NEXT(stopsym->n_next); 
	 i++, symcode = stopsym->next[ i ]) {      /* for all stop nodes */
      for (hmm_dlt = 0,sym_dlt = 0; hmm_dlt <gv-> trans->n_hmm;  hmm_dlt += gv->trans->n_hmm_set, sym_dlt += gv->trans->n_sym)  
	{
	  psym = (SymType *) ( gv->base_net + gv->trans->symlist[ symcode ] );  //get the basic HMM
	  hmmcode = psym->hmm_code;
	  hmm = GET_HMM(gv->base_hmms, hmmcode, hmm_dlt);
	  
	  n_state = NBR_STATES(hmm,gv->base_tran);
	  
	  state_cell = find_state_cell(gv->eval+idx, idx, VTL_SYM(symcode, sym_dlt), n_state, gv);
	  
	  if ( state_cell && state_cell[ n_state - 1 ].scr > BAD_SCR ) {
	    /* start state cell */
	    
	    scr = (long) state_cell[ n_state - 1 ].scr  + (long) gv->tranwgt;
	    
	    bsymc=symcode;

	    if (psym->n_next & 0x8000)   // sym is a word end   //just added sym in growbeam
	      {
		bword = VTL_SYM(symcode, sym_dlt);   //t_sym to word	
		bword_frm=gv->frm_cnt-1;
	      }
	    else
	      {
		bword = state_cell[ n_state - 1 ].sym;  //1
		bword_frm=state_cell[ n_state - 1 ].frm;      //2
	      }

	    back_trace_beam(bword, bword_frm, 
			    gv->hmm_code,  gv->stt,  gv->stp, 
			    &(gv->nbr_seg), &(gv->hmm_dlt), gv);
	    
	    nb_uniq(gv->answer, gv->nbr_ans, scr, &gv->nbest);
	  }
	}
    }

    nb_sort(&gv->nbest);

#ifdef SHOW_NBEST
    printf("N-bests : \n");
    for (n_state=0;n_state<nb_nbr_nbests(&gv->nbest);n_state++){
      answers = nb_nbests(&gv->nbest, n_state, &bsymc);
      for (k=0;k<bsymc;k++)
	printf("%s ", gv->vocabulary[ answers[ bsymc - 1 - k ]]);
      printf("\n");
    }
#endif
  
  }
  

  return eTIesrEngineSuccess;
}

/* ---------------------------------------------------------------------------
   search main entry 
   ---------------------------------------------------------------------- */

TIESRENGINECOREAPI_API short
search_a_frame(short mfcc_feature[], short not_end, 
	       unsigned short  frm_cnt,  /* input speech frame index, start from 0 */
	       gmhmm_type *gv)

{ 
  short cur_idx, prev_idx;     /* eval rotates between 0 and 1 */
  short status, prune;
  unsigned short last_frm = 0;
  long  tmp;

  /* ------------------------------------------------------------------
     first frame, init search, frm_cnt is global, must start from 0
     ------------------------------------------------------------- */
  gv->last_beam_cell = UNDEF_CELL;
#ifdef USE_NBEST
  gv->nb_next_time = USHRT_MAX;
  gv->nb_first_time_call = TRUE; 
  gv->last_nbest_cell = UNDEF_CELL;
#endif
  
#ifdef USE_SBC
  if (gv->comp_type & SBC) 
    sbc_enhance_all_trans( gv->sbcState,
			   mfcc_feature,
			   gv->nbr_dim);
#endif

  if ( frm_cnt == 0 ) {
    if (gv->max_beam_cell == 0) {
      PRT_ERR(printf("search space not set\n"));
      return eTIesrEngineSearchSpace;
    }
    
    /*reset real time guard*/
    gv->prev_FramesQueued=0;
    gv->guard=0;
    
    //    max_sym_cell2 = (max_sym_cell>>1) ;
    init_last_time_table(gv);  /* init direct time table access table*/
    init_high_pos(gv);           /* init peak usage statistics watermark */

    gv->best_cur_scr = BAD_SCR;
    gv->cum_best_prev_scr = 0;
    gv->cur_beam_count = 0;

    init_beam_list(gv); /* set cur pos of the beam and time table to 0*/

    init_cur_pos( 0, gv ); /*set the Eval Indx , the current sym an state to 0*/
    init_cur_pos( 1, gv );
    clear_obs_scr(gv);  /* clear cache of gaussian*/

#ifdef MODEL_LEVEL_PRUNE 
    mlp_reset(gv);
#endif

    /* init search */

    status = init_search(mfcc_feature, 0, gv, last_frm);

    gv->best_prev_scr = gv->best_cur_scr;
    
    gv->need_compact=FALSE;
    

#ifdef USE_NBEST
    if (gv->bUseNBest) nb_init_nbest(gv); 
#endif

    return status;

  /* --------------------------------------------------
     evaluate a frame
     --------------------------------------------- */

  } else if ( not_end ) {

    cur_idx = frm_cnt & 1;        /*even or odd*/
    prev_idx = ( frm_cnt - 1 ) & 1;

    gv->best_cur_scr = BAD_SCR;
    gv->best_sym = USHRT_MAX;
    gv->best_sym_scr = BAD_SCR;
    
    init_cur_pos( cur_idx, gv ); /*set cur_sym in eval to zero*/
    clear_obs_scr(gv);  /*clear cache*/

    tmp = gv->best_prev_scr + gv->prune * gv->nbr_dim;
#ifdef MODEL_LEVEL_PRUNE
    tmp = mlp_threshold(gv ,tmp, gv->prune * gv->nbr_dim); 
#endif

#ifdef DYNAMIC_PRUNE
    /*--------------added dynamic purning stuff---------
    Note that this kind of prunig is specific to the underlyingt task-- parallel names. A leading 'call/dial' may or may not 
    be a problem, not tested yet.
    
    Note, it may also related to feature dimensions -- now it is tuned to 9MFCC+delta
    
    It is related to vacab size -- but this part is handled in the code, so that pruning changes with the vocab size.
    --------------------------*/
    
    
    #ifdef DEBUGP
    printf("== PRUNE, S=%d , P=%d , %d  beam: %d or =+=%d\n", gv->best_prev_scr, gv->prune , gv->nbr_dim, gv->beam_count, gv->ending_sym_count);
    #endif
    
    
    
       //SymType * startsym = (SymType *) ( gv->base_net + gv->trans->start );
       //int n_fanout= startsym->n_next & 0x7fff; 
       //printf("=== fanout == %d", n_fanout);

#ifdef DYNAMIC_PRUNE_RAMP
    //------1. time constrainted pruning ADJUSTMENT : to deal with score variance issue
    if (frm_cnt<PRUNE_FRAME_TH  && frm_cnt>1 )  //note frm_cnt start from 1 in this part. 20  30 40
    {
     // thresholdd 6000 is the pruning at 20 frame. Note that 1280 is about equivalant to PRUNE=1 before (with 10 mfcc), so 6000 is about PRUNE=5
      	tmp+= (PRUNE_FRAME_TH -frm_cnt)* PRUNE_TIME_FACTOR; // 300; // *6000/20.0;  //6000 tuned with 3000 4000 ....  110000, 10000 seems best for dim10  9000 for 9dim.  1000 is for clean 
    }
    else  // The program set prune, which is PRUNE=7, is applied here. 
     tmp = gv->best_prev_scr + gv->prune * gv->nbr_dim; //no need. Set above already. 
#endif    	
     

#ifdef DYNAMIC_PRUNE_BEAM
      //2 -----active beam based prune.. this might be number of utt dependendent. Now tuened with 90 names.
      int zt;
      int th_n;
      //----this is a rough estimation on the relation between the number of words and the max number of beams
      // reason: n_words *2  contains both gender, beam_count happends at the end of HMM. (.. problem if we use word based BTrace)
      //maybe later should be changed to active hyps????
      th_n=gv->trans->n_word  * PRUNE_BEAM_WORD_RATIO ;   //620; //800 11  800 is used if no netoptimizer to cut the 25% / 8.0  9.0 ... 12.0   12: 11below  best for dim10
      
      if (gv->beam_count > th_n  + PRUNE_BEAM_WORD_DELTA)  // PRUNE_BEAM_WORD_RATIO 3.  900 for optimized net, 1000 for not optimized. tuned with 1000 not tuned    1000
	{
	  zt=(gv->beam_count-th_n)*PRUNE_BEAM_WORD_PNT1; //800(or 620) *11 
	  tmp=tmp+ zt;
	}
      else if (gv->beam_count > th_n)  // not tuned  800 without netoptimizer
	{
	  zt=(gv->beam_count-th_n)*PRUNE_BEAM_WORD_PNT2; // *10
	  tmp=tmp+ zt;
	}
#endif


#ifdef DYNAMIC_PRUNE_SUBLINEAR

      //----3 prevent linear increase of active beam (especially at begining)
      
      //float powt[22]={ 1.0000, 0.9000, 0.8100, 0.7290, 0.6561, 0.5905, 0.5314, 0.4783, 0.4305, 0.3874, 0.3487, 0.3138, 0.2824, 0.2542, 0.2288, 0.2059, 0.1853, 0.1668, 0.1501, 0.1351, 0.1216};
      //0.9^n * 2^14 is used to convert the above line into fixed point.
      short powt[22]={ 16384 , 14746 , 13271, 11944, 10750, 9675, 8706,7836, 7053, 6347, 5713, 5141, 4627,4165,3749,3373 , 3036,2733,2459,2213,1992};
      
      
      if (frm_cnt > 1 && frm_cnt<PRUNE_FRAME_TH ) {
	short tmp_s=(gv->trans->n_word<<1 )* frm_cnt;
	long tmp_l =(tmp_s * powt[frm_cnt])>>14;  //14 is from powt, which is 0.9^n * 2^14
	tmp_s= (short) tmp_l;
	
	if ( gv->beam_count > tmp_s )  
	  {
	    tmp+=PRUNE_GROW_PENALTY; 
	  }
      }  
#endif


      //----4 set minimal prune 
      
      //minimal prunie is about PRUNE=2: 2*64*20=2560, or 2*64*18= 2304
      //zt=zt <   -2560 - gv->prune * gv->nbr_dim ? zt: -2560 - gv->prune * gv->nbr_dim; 
      
      tmp=tmp > gv->best_prev_scr -PRUNE_MINIMUM ? gv->best_prev_scr -PRUNE_MINIMUM: tmp; //PRUNE_MINIMUM  2560
   
    tmp+=500; 
    //---------end of dynamic pruning 
#endif      // for dynamic pruning

     
#ifdef REALTIMEGUARD
   /*50 frames is 1 second, the minimal time for realtime guard to activate. 
     if the queue is larg and increasing, apply penalty increasingly.
     If the queue is large but decreasing, apply less penalty. 
     real th can be 100: 2second , REALT_GUARD= 800, about 0.7 in prunne  */
    if ( /* frm_cnt> gv->realtime_th && */ gv->FramesQueued> (ushort) gv->realtime_th 
	 && ((gv->FramesQueued - gv->prev_FramesQueued) > 0) )  
   {  
   	gv->guard +=gv->realt_guard; /*keep increasing*/
   	tmp+= gv->guard;
   }
    else if ( /*frm_cnt> gv->realtime_th && */ gv->FramesQueued> (ushort)gv->realtime_th )  /* already decreasing, but still high queue*/
   { 
     gv->guard -= gv->realt_guard * ( gv->prev_FramesQueued - gv->FramesQueued )/(gv->FramesQueued - gv->realtime_th) ;
     gv->guard= gv->guard>0 ? gv->guard : 0 ;
     tmp+= gv->guard;
   }
   else
   {
     gv->guard=0;
   }
   gv->prev_FramesQueued = gv->FramesQueued ;
#endif

    tmp=tmp > gv->best_prev_scr -PRUNE_MINIMUM ? gv->best_prev_scr -PRUNE_MINIMUM: tmp; //PRUNE_MINIMUM  2560 

   /*for FA*/     
    if (gv->word_backtrace == HMMFA) tmp = gv->best_prev_scr - 150 * gv->nbr_dim;
   
    prune = MAX(tmp, -32768);  
//printf("prune is %d\n", prune);    
    last_frm = frm_cnt - 1; /* used in mark_cells, etc. -- should move to 3 lines below. */
    /* HMM transition , and determine best path from Gaussians compution of LAST FRAME*/

    status = path_propagation( cur_idx, prev_idx, frm_cnt, gv, prune);

    if ( status != eTIesrEngineSuccess ) 
      return status;

#ifdef MODEL_LEVEL_PRUNE 
    mlp_reset(gv);
#endif 
    /* accumulate acoustic score */
    
     //needed for second FA pass, if FA pass uses Gaussian selection
     //alternatively, this can be put in process_file_1 in FA in jac-estim.cpp
    //if (gv->word_backtrace == HMMFA )  
    // rj_clear_obs_scr(gv, mfcc_feature);
     
    status = update_obs_prob(mfcc_feature, cur_idx, frm_cnt, gv, last_frm );
    if ( status != eTIesrEngineSuccess ) 
      return status;

    /* do the backtrace (memory manage) stuff */
    #ifdef DEBUGP
    printf("beam cell size: %d at frame %d\n", gv->evalstat.beam.high_pos,  frm_cnt );
    #endif
  

    /* do the backtrace stuff */
#ifdef USE_NBEST
    if (gv->bUseNBest) 
      nb_compact_beam_cells( cur_idx, gv->beam_base, gv->time_base, gv , 
			     gv->ptr_nbest_tmp->pNBestCells);
    else
      compact_beam_cells( gv->beam_base, gv->time_base, gv );
#else

    if (gv->need_compact==TRUE )            // || gv->word_backtrace==FALSE ) 
     {
     	compact_beam_cells(  gv->beam_base, gv->time_base, gv );  
        gv->need_compact=FALSE;
        //printf("compact beam==%d==frm %d ===",gv->evalstat.beam.cur_pos,  frm_cnt);
     }
    else if (gv->evalstat.beam.cur_pos > BEAM_CELL_LIMIT)
     { gv->need_compact=TRUE;
     }
    
    /*shift the quick time access cache*/

#endif

    update_time_table(gv->time_table_idx, NN, gv); 
    
    gv->cum_best_prev_scr += (long) gv->best_prev_scr;

    gv->best_prev_scr = gv->best_cur_scr;

#ifdef DBG_HMM
    find_best_sym( frm_cnt & 1, gv); 
#endif

    return status;

  /* --------------------------------------------------
     end, report result
     --------------------------------------------- */

  } else { 
#ifdef SHOW_DIAGONISIS
    global_count2+=	global_count; global_frame+=frm_cnt;
    //#ifdef DEBUGP
    //printf("gc=%d, frame=%d. Gaussian per frame %d. \n GlobalGPF: Gas: %d, GFRM: %d ,Ave GPF %d\n. ", global_count, frm_cnt, global_count/frm_cnt, global_count2,global_frame, global_count2/global_frame);
    //#endif
#endif
    
    gv->best_sym = USHRT_MAX;
    gv->best_sym_scr = BAD_SCR;

    cur_idx = frm_cnt & 1;

    if (gv->bUseNBest){
      if( find_good_syms( cur_idx, gv) != eTIesrEngineSuccess)
	return  eTIesrEngineNBestMem; 
    }

    find_best_sym( cur_idx, gv );

    gv->best_sym_scr += gv->cum_best_prev_scr;

    return eTIesrEngineSuccess;
  }
}



/* 
** backtrace beam cells
** if hmm_codes != NULL, then back track also phone boundaries (in reversed order)
** "answer" contains the word code of the recognized utterance.
** return the number of recognized words in "answer"
*/
// need change.

TIESRENGINECOREAPI_API TIesrEngineStatusType
back_trace_beam(short symcode, short frm, ushort *hmm_codes, ushort *stt, ushort *stp, ushort *nbr_seg, ushort *hmm_delta, gmhmm_type *gv)
{
   BeamType *b = gv->beam_base;
   SymType  *sym, *psym, *zsym;
   short    cont, ii , jj, fano;
   short    time;
   unsigned short t_sym, hmm_dlt, b_index, word_code, l_frm = frm, p;
   TimeType *time_base = gv->time_base;
   BeamType *beam_base = gv->beam_base;
   
   gv->nbr_ans = 0; //keep this during change
   
   #ifdef USE_CONFIDENCE
   ConfType* pConf = (ConfType*) gv->pConf;
   #endif
   
   
   if (gv->word_backtrace==WORDBT)
   {
      //printf("=============word backtrace\n");
      
      if ( frm >= 0 )
      {
         // Index of hmm_codes in backtrace
         p = 0;
         
         // hmm_code of 0 is not necessarily silence with non-TIesrFlex grammars
         // Now all hmm's are tied to words so this is not necessary.
         //if (frm != gv->frm_cnt-1)
         //{//-printf("add hmmcode 0 at end\n");
         //    hmm_codes[p++]=0;
         //}
         
         /* return the beam cell */
         //find the time of the last word before frm (when a beam/time cell generated)
         MATCH_TIME(time, time_base, frm);
         
         
         //use the best sym to do backtrace
         //MATCH_SYMB(b, time_base[time].beam_idx, beam_base, gv->best_sym);
         //#define MATCH_SYMB(bm, bm0, beam0, best_sym) for (bm = beam0 + bm0; bm->sym != best_sym; bm = beam0 + bm->next)
         for (b = beam_base + time_base[time].beam_idx; b->sym != symcode; b = beam_base + b->next)
         {
            ; //printf("bt: %d \n", b->next);
         }
         
         gv->nbr_ans = 0;

         // Loop over all best path words in the beam
         do
         {
            
            /* hmm_d are not used, but it will be when you do phone-level decoding */
            // Find the base path symbol representing the path of the best beam symbol 
            unsigned t_sym;
            for ( t_sym = b->sym; t_sym >= gv->trans->n_sym;  t_sym  -= gv->trans->n_sym) ;
            
            // Pointer to base grammar symbol information for path symbol
            //psym = (SymType *) ( gv->base_net + b->sym );
            psym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_sym ] );
            
            //Initialize search for the symbol that has transition into path symbol
            unsigned short gotit=0;
            unsigned short wordend=0;
            unsigned short t_sym2=t_sym;
            
            // Add word path symbol hmm code to the hmm_codes along the path
            //printf ("hmm: %d symnode: %d  %d\n", psym->hmm_code,t_sym, p);
            if( p >= MAX_NBR_SEGS )
            {
               *nbr_seg = p;
               return eTIesrEngineSegmentMemoryOut;
            }
 
            hmm_codes[p++]=psym->hmm_code;

            
            // Loop to add all symbols of the path corresponding to the present word
            do
            {
                // Find a grammar symbol with transition into path symbol
               
               // Loop over all grammar symbols
               for ( ii=0; ii< gv->trans->n_sym; ii++)//for all the node
               {
                  zsym = (SymType *) ( gv->base_net + gv->trans->symlist[ ii ] );
                  
                  // Loop over all transitions out of the grammar symbol
                  fano=zsym->n_next & 0x7fff;
                  for (jj=0; jj< fano ; jj++)
                  {
                     // for all the fanout of node ii
                     
                     // Check if symbol has transition that goes into path symbol
                     //printf(" %d fanout %d\n", ii, zsym-> n_next & 0x7fff);
                     if ( zsym->next[jj]==t_sym2)
                     {
                        t_sym2=ii;
                        gotit=1;
                        
                        //if ii is a word end, or sil, or start node (note: Sil must be HMM=0!!
                        //if  (zsym->n_next & 0x8000 ||zsym->hmm_code==0 || ii==0 )
                        
                        // This check has been changed. Now, by convention, all grammar symbols 
                        // are connected to words, hence each symbol is either marked as word 
                        // ending, or is a symbol corresponding to an earlier hmm in the word. 
                        // There is no requirement that the hmm_code 0 is silence.  This is just
                        // done by convention in TIesrFlex.  The first grammar symbol (ii=0) does
                        // not necessarily correspond to the beginning of the grammar.
                        // Check if symbol transitioning to path symbol is word ending symbol
                        if(zsym->n_next & 0x8000)
                           wordend=1;  //get word end or hmmcode=0 (sil) ...or get node 0 starting node
                        else
                        { 
                           
                           //printf ("bt: hmm: %d , node %d\n", zsym->hmm_code, ii);

                           // If not a word ending symbol, it must be a symbol corresponding
                           // to an hmm of the word along the path so add it to the hmm_codes.
                           if( p >= MAX_NBR_SEGS )
                           {
                              *nbr_seg = p;
                              return eTIesrEngineSegmentMemoryOut;
                           }

                           hmm_codes[p++]=zsym->hmm_code;
                        }
                        
                        // Found symbol transitioning to path symbol, break out of transition loop
                        break;
                     }
                  }
                  
                  // If symbol transitioning to path symbol found, break out of symbol loop
                  if (gotit)
                  {break;}
               }
               
 
               if (gotit==1)
               {
                  // If the symbol transitioning to path symbol was found, reset the flag,
                  // and go look for next symbol of word if not at first symbol of word
                  gotit=0;
               }
               else
               {
                  // There is no symbol with a transition to path symbol.  Must be at
                  // the start of the grammar, and hence have found all symbols for a whole
                  // word.
                  
                  wordend = 1;
               }
              
               // Loop until all symbols for a word have been added to the hmm_codes array
            }while (wordend==0);     //t_sym2 is not a end of  word or sil
            
            // Not necessary any longer to add a "sil" model code between words.
            // Now the silence model is represented by a "_SIL" word, so will be
            // output by the above word processing loop.
            //printf (" add a hmmcode 0 here sil\n");
            //hmm_codes[p++]=0;
            
            gv->answer[ (gv->nbr_ans)++ ] = psym->next[GET_N_NEXT(psym->n_next)]; //b->sym;  //zx
            if (gv->nbr_ans == MAX_WORD_UTT) return  eTIesrEngineAnswerMemoryOut;
            
            /*move to the next beam cell*/
            b_index =  b->idx_flag & 0x7fff;
            
            if ((cont = NOT_END_IX_CELL(b_index, gv->max_beam_cell)) == TRUE)
            {
               
               b = beam_base + b_index;
               
            }
            else
            {
               ;
            }
            
         } while ( cont );
         
         // hack, it is not used, but for passing some check
         //*nbr_seg = gv->nbr_ans;  *hmm_delta = gv->best_word_gender;  // I am not sure if these two are useful
         /* assigned to the last one */
         if (stt)
         {
            *nbr_seg = p;  *hmm_delta = gv->best_word_gender; /*printf("GENDER: %d\n", *hmm_delta);*/
         }
      }
   }
   
   else
   {//hmm backtrace
      //printf("=============hmm backtrace\n");
      if ( frm >= 0 )
      {
         /* return the beam cell */
         MATCH_TIME(time, time_base, frm);
         MATCH_SYMB(b, time_base[time].beam_idx, beam_base, gv->best_sym);
         gv->nbr_ans = 0;
         p = 0;
         do
         {
            /* hmm_d are not used, but it will be when you do phone-level decoding */
            for (hmm_dlt = 0, t_sym = symcode; t_sym >= gv->trans->n_sym; t_sym  -= gv->trans->n_sym, hmm_dlt += gv->trans->n_hmm_set);
            sym = (SymType *) ( gv->base_net + gv->trans->symlist[ t_sym ] );
            
            if ( sym->n_next & 0x8000 )
            {    /* recognized words, backward order */
               word_code = sym->next[GET_N_NEXT(sym->n_next)];
               
               #ifdef USE_CONFIDENCE
               pConf->answers[gv->nbr_ans] = word_code ;
               pConf->uStp[gv->nbr_ans] = frm + 1;
               pConf->nWords = gv->nbr_ans + 1;
               #endif
               
               gv->answer[ (gv->nbr_ans)++ ] = word_code;
               if (gv->nbr_ans == MAX_WORD_UTT)
                  return  eTIesrEngineAnswerMemoryOut;
            }
            
            b_index =  b->idx_flag & 0x7fff;
            if ((cont = NOT_END_IX_CELL(b_index, gv->max_beam_cell)) == TRUE)
            {
               frm = get_frame_time(b_index, gv);
               b = beam_base + b_index;
               symcode = b->sym;
            }
            else
            {
               symcode = USHRT_MAX;
               frm = USHRT_MAX;
            }
            /* this means phone back track requested */

            if( hmm_codes || stp || stt )
            {
               if( p >= MAX_NBR_SEGS )
               {
                  *nbr_seg = p;
                  return eTIesrEngineSegmentMemoryOut;
               }

               if( hmm_codes )
               {
                  hmm_codes[p] = sym->hmm_code;
                  // printf("hmmcode: %d, node %d\n" , hmm_codes[p], t_sym ); //-1352
               }
               if( stp ) stp[p] = l_frm + 1;
               if( stt ) stt[p] = frm + 1; /* this is the AT LEAST MUST HAVE for hmm tracing */
               //      printf("  %d,  start %d, stop %d\n", p, stt[p], stp[p]);
               p++;
               l_frm = frm;
            }
         } while ( cont );
         
         /* assigned to the last one */
         if (stt)
         {  *nbr_seg = p;  *hmm_delta = hmm_dlt;
            gv->best_word_gender=hmm_dlt; /*printf("GENDER: %d\n", *hmm_delta);*/
         }
      }  //end of hmm or word
      
   }
   return eTIesrEngineSuccess;
}


/*
  
** This file contains function for claculating EM forward/back ward variables.
** - The functions uses per-frame log-likelihood normalization to keep values
**   in the short integer range. 
** - All entry and output variables are in short.
** - Forward variables are calculated on-demand, frame by frame, while backward variables
**   are calculated by block (e.g. for all available frames)
** 
*/

/* 
** compute alpha variable and return likelihood of the given sequence 
** work for multi-entry state models.
*/
static void EM_alpha_0(short pd[], short alpha[], int T, HmmType *hmm, short c_t_beta, gmhmm_type *gv)
{
  ushort i;
  short trans;
  long tmp;
  /* START: t = 0, for all states */
  FOR_EMS_STATES(i,hmm,gv->base_tran) {   /* initial A2: */ 
    trans = GET_PI(hmm,gv->base_tran)[i];
    tmp = (HAS_TRANSITION(trans)? (trans + (long)pd[i * T]): BAD_SCR) - c_t_beta;
    alpha[i] = LONG2SHORT(tmp);
  }
}

/*
**  general case t in [1,T)
*/

static void EM_alpha_gnl(short pd[], short alpha[],  short alfa[], int T, HmmType *hmm, ushort t, short c_t_beta, gmhmm_type *gv)
{
  ushort i,j;
  short *a_i, *a_j, trans;
  long tmp;

  a_j = alpha;
  FOR_EMS_STATES(j,hmm,gv->base_tran) {
    tmp = BAD_SCR;
    a_i = alfa;
    FOR_EMS_STATES(i,hmm,gv->base_tran) {
      trans = GET_AIJ(hmm,i,j,gv->base_tran);
      if (HAS_TRANSITION(trans)) {
	tmp = hlr_logsum(LONG2SHORT(tmp), LONG2SHORT(a_i[i] + (long)trans));
      }
    }
    tmp += pd[j * T + t] - c_t_beta;
    a_j[j]  = LONG2SHORT( tmp );
  }
}

/*
** for exit states:  TERMINATION: t = T-1, emission states 
*/

static void EM_alpha_e(short alpha[],  HmmType *hmm, short c_t_beta, gmhmm_type *gv )
{
  ushort i, nbr_states = NBR_STATES(hmm,gv->base_tran) - 1;
  long tmp; 
  short *a_i, trans;

  tmp = BAD_SCR ; 
  a_i = alpha;
  FOR_EMS_STATES(i,hmm,gv->base_tran) {  /* final, no emission prob A3: */
    trans = GET_AIJ(hmm,i, nbr_states,gv->base_tran);
    if (HAS_TRANSITION( trans )) 
      tmp = hlr_logsum((short)tmp, LONG2SHORT(a_i[i] + (long) trans));
  }
  /* EXIT STATE: t = T-1, exit state */
  a_i = alpha;
  tmp -= c_t_beta;
  a_i[ nbr_states ] = LONG2SHORT( tmp );
}


/*
** compute the alpha (forward) variable used in EM algorithm.
** alpha is calculated for the frame t.
** the function returns a pointor to the memory holding the alpha_t.
*/

TIESRENGINECOREAPI_API short * /* return the starting address of alpha variable for frame t */
EM_alpha_t (short lld[],       /* log likelihood of the segment, lld[j * T + t] for state j frame t */
	    short alpha[],    /* forward variable, size must be 2 * NBR_STATES(hmm) */
	    ushort t,         /* current frame index */
	    ushort T,         /* total number of frames */
	    HmmType *hmm,     /* GTM structure hmm  */
	    short c_t_beta[], /* normalization factor, size T+1, c_t_beta[t] for frame t output */
	    gmhmm_type *gv)
{
    short *a_t = GET_ALPHA(alpha,t,NBR_STATES(hmm,gv->base_tran));
    short *a_t1;
    
    if (t==0) EM_alpha_0(lld, a_t, T, hmm, c_t_beta[T],gv);
    else {
      a_t1 = GET_ALPHA(alpha,t-1,NBR_STATES(hmm, gv->base_tran));
      EM_alpha_gnl(lld, a_t, a_t1, T, hmm, t, c_t_beta[t-1],gv);
    }
    if (t == T-1) EM_alpha_e(a_t, hmm, c_t_beta[t],gv);
    return a_t;
}


/*  
** compute beta (backward) variable used in EM algorithm.
** beta is calculated for all the segment t=[0, T-1].
*/

TIESRENGINECOREAPI_API long  /* return the total log likelihood over the T frames */
EM_beta(short lld[],     /* log likelihood of the segment, lld[j * T + t] for state j frame t */
        short beta[],    /* backward variable, beta[j * T + t] for state j frame t */
	ushort T,        /* total number of frames */
        HmmType *hmm,    /* GTM structure hmm  */
        short c_t_beta[],/* normalization factor, size T+1, c_t_beta[t] for frame t output */
	gmhmm_type *gv)
{
   ushort i,j, nbr_ems_states = NBR_STATES(hmm, gv->base_tran)-1; 
   short trans, *b_i, *b_j, t, c_t; 
   long tmp, log_sum_b = 0;
   
   beta[nbr_ems_states * T + T-1] = 0; /* B3 */
   c_t = BAD_SCR; 

   FOR_E_STATES(i,b_i,nbr_ems_states,beta,T) { /* beta[T-1] from exit state  B2 */
     trans = GET_AIJ(hmm,i,nbr_ems_states, gv->base_tran);
     if (HAS_TRANSITION(trans)) b_i[T-1] = trans;
     else b_i[T-1] = BAD_SCR;
     c_t = hlr_logsum(c_t,b_i[T-1]);
   }
   c_t_beta[T-1] = c_t;

   FOR_E_STATES(i,b_i,nbr_ems_states,beta,T) b_i[T-1] = LONG2SHORT((long)b_i[T-1] - c_t_beta[T-1]);
   
   b_i = beta;
   log_sum_b += c_t_beta[T-1];  

   for (t = T-2; t>=0; t--) { /* beta[t] from beta[t+1]  B1 */
     c_t = BAD_SCR; 
     FOR_E_STATES(i,b_i,nbr_ems_states,beta,T) {
       tmp = BAD_SCR;
       FOR_E_STATES(j,b_j,nbr_ems_states,beta,T) { /* remove trans will make diff btw FWD and BWD var */
	 trans = GET_AIJ(hmm,i,j, gv->base_tran);
	 if (HAS_TRANSITION(trans)) {
	   tmp = hlr_logsum((short)(tmp), LONG2SHORT(trans +(long) lld[j * T + t+1] + b_j[t+1]));
	 }
       }
       b_i[t] = (short)(tmp);
       c_t = hlr_logsum(c_t, b_i[t]);
     }
     c_t_beta[t] = c_t;
     FOR_E_STATES(i,b_i,nbr_ems_states,beta,T) b_i[t] = LONG2SHORT((long)b_i[t] - c_t_beta[t]);
     log_sum_b += c_t_beta[t];
   }
   tmp = BAD_SCR;    /* just check: not necessary for computing other parameters */
   c_t = BAD_SCR;
   FOR_E_STATES(i,b_i,nbr_ems_states,beta,T) {
     trans = GET_PI(hmm, gv->base_tran)[i];
     if (HAS_TRANSITION( trans ))
       tmp = hlr_logsum((short)(tmp), LONG2SHORT((long)trans + lld[i * T + 0] + b_i[0]));
   }
   c_t_beta[T] = hlr_logsum(c_t, (short)(tmp));
   log_sum_b += c_t_beta[T];
   return log_sum_b;
}

/* get the pointer to the log-likelihood score of the model $s_seg */
void EM_get_lld( short s_seg,
		 const short *p_lld, 
		 const short * p_beta, 
		 short T, 
		 SearchMemType * smem, 
		 gmhmm_type * gv)
{
  ushort nbr_ems_states ;
  short s, *b_j, *l_j;
  HmmType * hmm; 

  b_j = (short*) p_lld; l_j = (short*) p_beta; 
  for (s=0;s<s_seg;s++){
    /* for each HMM */
    hmm = GET_HMM(gv->base_hmms, gv->hmm_code[s], gv->hmm_dlt);
    nbr_ems_states = NBR_STATES(hmm, gv->base_tran) - 1; 
    if (b_j) b_j += nbr_ems_states * T; 
    l_j += (nbr_ems_states + 2) * T;
  }
  smem->p_lld_hmm = b_j; 
  smem->p_beta_hmm = l_j; 
}

#ifdef DEV_EM
long EM_beta_word(const short lld[],     /* log likelihood of the segment, lld[j * T + t] for state j frame t */
		  short beta[],
		  /* backward variable, beta[j * T + t] for state j frame t */
		  short sz_bkwd, /* size of the memory for beta */
		  ushort T,        /* total number of frames */
		  short c_t_beta[],
		  /* normalization factor, size T+1, c_t_beta[t] for frame t output */
		  gmhmm_type *gv)
{
  ushort i,j, nbr_ems_states ;
  short trans, *b_i, *b_j, t, c_t, s, *beta_hmm, 
    *lld_hmm, Nq, *beta_em, Nqt; 
  long tmp, log_sum_b = 0, b_ip1_tp1 = 0/* beta(1, q+1, t+1) */;
  HmmType * hmm; 
  SearchMemType smem; 

  for (t=T-1;t>=0;t--){ /* for each time */
    
    c_t = BAD_SCR; 
    for (s=gv->nbr_seg -1;s>=0;s--){
      /* for each HMM */
      hmm = GET_HMM(gv->base_hmms, gv->hmm_code[s], gv->hmm_dlt);
      nbr_ems_states = NBR_STATES(hmm, gv->base_tran) - 1; 
      EM_get_lld( s, lld, beta, T, &smem, gv); 
      beta_hmm = smem.p_beta_hmm; 
      lld_hmm = smem.p_lld_hmm; 
      Nq = nbr_ems_states + 1; 
      beta_em = beta_hmm + T; /* starting pointer to the first emitting states */
      /* the starting point of beta for the current HMM */

      Nqt = Nq * T + t; 
      if (t==T-1){
	beta_hmm[Nqt] = (s==gv->nbr_seg-1)? 0 : BAD_SCR ; 
	
	FOR_E_STATES(i,b_i,nbr_ems_states,beta_em,T) { /* beta[T-1] from exit state  B2 */
	  trans = GET_AIJ(hmm,i,nbr_ems_states, gv->base_tran);
	  if (HAS_TRANSITION(trans)) 
	      b_i[t] = LONG2SHORT(trans + (long)beta_hmm[Nqt]);
	  else b_i[t] = BAD_SCR;
	  c_t = MAX(c_t, b_i[t]); 
	}
      }else{
	beta_hmm[Nqt] = (s==gv->nbr_seg-1)? BAD_SCR : b_ip1_tp1; 
	
	FOR_E_STATES(i,b_i,nbr_ems_states,beta_em,T) {
	  tmp = BAD_SCR;
	  FOR_E_STATES(j,b_j,nbr_ems_states,beta_em,T) { 
	    trans = GET_AIJ(hmm,i,j, gv->base_tran);
	    if (HAS_TRANSITION(trans)) {
	      tmp = 
		hlr_logsum((short)(tmp), 
			   LONG2SHORT(trans +(long) lld_hmm[j * T + t+1] + (long) b_j[t+1]));
	    }
	  }
	  trans = GET_AIJ(hmm, i, nbr_ems_states, gv->base_tran);
	  if (HAS_TRANSITION(trans))
	    tmp = hlr_logsum((short)(tmp), 
			     LONG2SHORT(trans + (long) beta_hmm[Nqt]));
	  b_i[t] = (short)(tmp);
	  c_t = MAX(c_t, b_i[t]);
	}
      }

      tmp = BAD_SCR; 
      FOR_E_STATES(i,b_i,nbr_ems_states,beta_em,T) {
	trans = GET_PI(hmm, gv->base_tran)[i];
	if (HAS_TRANSITION( trans ))
	  tmp = hlr_logsum((short)(tmp), 
			   LONG2SHORT((long)trans + lld_hmm[i * T + t] + b_i[t]));
      }
      beta_hmm[t] = tmp; 
      c_t = MAX(c_t, tmp);

      b_ip1_tp1 = (t==T-1)?BAD_SCR:beta_hmm[t+1];
    }

    c_t_beta[t] = c_t; 

    /* normalize score */
    for (s=t;s<sz_bkwd;s+=T)
      beta[s] = LONG2SHORT((long)beta[s] - (long)c_t); 
  }

  log_sum_b = beta_hmm[0]; 
  return log_sum_b;
}

long EM_alpha_word(short tfrm, 
		   const short lld[],     
		   /* log likelihood of the segment, lld[j * T + t] for state j frame t */
		   short alpha[],    /* backward variable, beta[j * T + t] for state j frame t */
		   ushort T,        /* total number of frames */
		   ushort sz_frwd,  /* size of alpha */
		   short c_t_beta[],
		   /* normalization factor, size T+1, c_t_beta[t] for frame t output */
		   gmhmm_type *gv)
{
  ushort i,j, nbr_ems_states ;
  short trans, *a_i, *a_j, t, s, *alpha_hmm, *alpha_hmm_tm1,
    *p_alpha, *p_alpha_tm1, *alpha_em_tm1,
    *lld_hmm, Nq, *alpha_em;
  long tmp, tmp2, a_im1_tm1 = 0/* beta(1, q+1, t+1) */, log_sum_b;
  HmmType * hmm; 

  t = tfrm; 
  {
    p_alpha = (t&0x0001)?alpha +  (sz_frwd >> 1): alpha; /* alpha(t) */
    p_alpha_tm1 = (t&0x0001)?alpha:alpha +  (sz_frwd >> 1); /* alpha(t-1)*/
    alpha_hmm = p_alpha; 
    alpha_hmm_tm1 = p_alpha_tm1; 
    lld_hmm = (short*) lld; 

    for (s=0;s<gv->nbr_seg;s++){
      /* for each HMM */
      hmm = GET_HMM(gv->base_hmms, gv->hmm_code[s], gv->hmm_dlt);
      nbr_ems_states = NBR_STATES(hmm, gv->base_tran) - 1; 

      alpha_em = alpha_hmm + 1; /* starting pointer to the first emitting states */
      alpha_em_tm1 = alpha_hmm_tm1 + 1; 
      /* the starting point of alpha for the current HMM */
      Nq = nbr_ems_states + 1; 

      if (t==0){
	alpha_hmm[0] = (s==0)?0:BAD_SCR; 
	
	FOR_EMS_STATES(i,hmm,gv->base_tran) {   /* initial A2: */ 
	  trans = GET_PI(hmm,gv->base_tran)[i];
	  tmp = HAS_TRANSITION(trans)? 
	    (trans + (long)lld_hmm[ i * T + t ] + (long)alpha_hmm[0]) : BAD_SCR ;
	  alpha_em[i] = LONG2SHORT(tmp);
	}
      }else{
	alpha_hmm[0] = (s==0)?BAD_SCR:a_im1_tm1; 
	
	a_j = alpha_em;
	FOR_EMS_STATES(j,hmm,gv->base_tran) {
	  tmp = BAD_SCR;
	  a_i = alpha_em_tm1;
	  FOR_EMS_STATES(i,hmm,gv->base_tran) {
	    trans = GET_AIJ(hmm,i,j,gv->base_tran);
	    if (HAS_TRANSITION(trans)) {
	      tmp = hlr_logsum(LONG2SHORT(tmp), LONG2SHORT(a_i[i] + (long)trans));
	    }
	  }
	  trans = GET_PI(hmm,gv->base_tran)[j];
	  if (HAS_TRANSITION(trans)){
	    tmp2 = trans + (long)alpha_hmm[0];
	    tmp = hlr_logsum(LONG2SHORT(tmp), LONG2SHORT(tmp2)); 
	  }
	  tmp += (long)lld_hmm[j * T + t];
	  a_j[j]  = LONG2SHORT( tmp );
	}
      }

      tmp = BAD_SCR ; 
      a_i = alpha_em;
      FOR_EMS_STATES(i,hmm,gv->base_tran) {  /* final, no emission prob A3: */
	trans = GET_AIJ(hmm,i, nbr_ems_states,gv->base_tran);
	if (HAS_TRANSITION( trans )) 
	  tmp = hlr_logsum((short)tmp, LONG2SHORT(a_i[i] + (long) trans));
      }
      alpha_hmm[Nq] = LONG2SHORT(tmp);
      
      a_im1_tm1 = (t==0)?BAD_SCR:alpha_hmm_tm1[Nq];
      alpha_hmm += nbr_ems_states + 2;
      alpha_hmm_tm1 += nbr_ems_states + 2; 
      lld_hmm += nbr_ems_states * T; 

    }

    i = sz_frwd >> 1; 
    for (s=0;s<i;s++)
      p_alpha[s] = LONG2SHORT((long)p_alpha[s] - c_t_beta[t]);

    log_sum_b = p_alpha[i-1]; 
  }

  return log_sum_b;
}

#endif
