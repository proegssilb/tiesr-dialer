/*=======================================================================
 
 *
 * siproc.cpp
 *
 * TIesr processing functions.
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

 This source defines a few functions used for TIesr SI recognition.

======================================================================*/

/*
#include "mfcc_f.h"
#include "gmhmm.h"
#include "gmhmm_type.h"
#include "uttdet.h"
#include "gmhmm_si_api.h"
*/

/* Windows OS specific includes */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif


#include "tiesr_config.h"

#include "tiesrcommonmacros.h"
#include "status.h"
#include "search_user.h"
#include "gmhmm_si_api.h"
#include "sbc_user.h"
#ifdef USE_NBEST
#include <nbest_user.h>
#endif

#ifdef USE_AUDIO
#include "volume_user.h"
#endif

/* #define WORK_ON_SEARCH */
/*
** works for offline tests. Has no effect if audio device is used.
*/
#ifdef WORK_ON_SEARCH 
static void GetSearchInfo(void *gvv)
{
  gmhmm_type *gv = (gmhmm_type *)gvv;
  PRT(printf("DR: peak count, beam = %d, sym = %d, state = %d, time = %d\n", 
	 gv->evalstat.beam.high_pos,
	 NSYM * MAX(gv->evalstat.sym[0].high_pos,gv->evalstat.sym[1].high_pos),
	 gv->evalstat.state.high_pos, gv->evalstat.time.high_pos));  
  printf("Search size = %d words, score = %f (%d frames)\n", SEARCH_SPACE(gv->evalstat),
	 (float) gv->best_sym_scr /  gv->frm_cnt / 64,  gv->frm_cnt);
}
#endif

/* GMHMM_SI_API */ 
/*
short SpeechDetected(void *gvv)
{
  gmhmm_type *g = (gmhmm_type *) gvv;

  return ( g->speech_detected);
}
*/

/* GMHMM_SI_API */ 
/*
short SpeechEnded(void *gvv)
{
  gmhmm_type *g = (gmhmm_type *) gvv;
  
  return ( g->speech_detected && ( g->uttdet_state == NON_SPEECH ));
}
*/

/*
** per utterance initialization
*/

#ifdef _MONOPHONE_NET

static int find_hmm_idx(int c, int l, int r, int hmm_idx, Hmmtbl* hmm)
{
   int   i;
   
   for ( i = 1; i < hmm_idx; i++ )
   {
      if ( hmm[i].c == c && hmm[i].l == l && hmm[i].r == r ) return i;
   }
   
   return 0;
}

static TIesrEngineStatusType  rs_CopyAlignFrom( gmhmm_type * src_eng, 
						gmhmm_type * tgt_eng)
{
  short i, si; 
  int c, l, r, ui, n_triphone; 
  short sDoTriphone = FALSE; 
  FILE * fp; 
  Hmmtbl *triphonehmms = NULL; 
  Hmmtbl tmpmem; 

  fp = fopen(HMM_TRIPHONE_INDEX_FILE, "rb");
  if (fp){
    /* exist triphone HMM index file */
    i = 0; 
    fread(&n_triphone, sizeof(int), 1, fp);
    fread(&ui, sizeof(int), 1, fp);
    triphonehmms = (Hmmtbl*) malloc(sizeof(Hmmtbl)*ui);
    do{
      fread(&triphonehmms[i], sizeof(Hmmtbl), 1, fp); 
      i++;
    }while(i < ui); 
    fclose(fp);
    sDoTriphone = TRUE; 
  }

  tgt_eng->nbr_ans = src_eng->nbr_ans; 
  for (i=0;i<tgt_eng->nbr_ans;i++) tgt_eng->answer[i] = src_eng->answer[i]; 
  tgt_eng->nbr_seg = src_eng->nbr_seg;
  for (i=0;i<tgt_eng->nbr_seg;i++) {
    if (sDoTriphone){
      if (i==0) r = n_triphone; 
      else if (IS_WORD_END_TIME( src_eng->stp[i] )) r = n_triphone; /* word-level transcription */
      else if (src_eng->stp[i] >= src_eng->stp[i-1]) /* sentence-level transcription */
	r = n_triphone; 
      else 
	r = src_eng->hmm_code[i-1]; 

      c = src_eng->hmm_code[i]; 

      if (i==tgt_eng->nbr_seg - 1) l = n_triphone; 
      else if (IS_WORD_END_TIME(src_eng->stp[i+1])) l = n_triphone; 
      else if (src_eng->stp[i] <= src_eng->stp[i+1])
	l = n_triphone; 
      else
	l = src_eng->hmm_code[i+1]; 
      
      si = (short) find_hmm_idx(c, l, r, ui, triphonehmms); 
    }else
      si = src_eng->hmm_code[i]; 
    tgt_eng->hmm_code[i] = si; 
    tgt_eng->stp[i] = CLR_MARK_WORD_END_TIME(src_eng->stp[i]);
    tgt_eng->stt[i] = CLR_MARK_WORD_END_TIME(src_eng->stt[i]);
  }

  tgt_eng->hmm_dlt = 0; /* assume male model */
  
  if (triphonehmms) free(triphonehmms);
  return eTIesrEngineSuccess;
}
#endif

/*--------------------------------*/
/*
** back tracking search space, set error messsages:
*/

/* GMHMM_SI_API */ TIesrEngineStatusType CloseSearchEngine(TIesrEngineStatusType status, gmhmm_type *gvv)
{
  gmhmm_type *global_var = (gmhmm_type *) gvv;
#ifdef _MONOPHONE_NET
  global_var = (gmhmm_type*) global_var->pAux; 
#endif

  global_var->nbr_seg = 0;
  global_var->nbr_ans = 0;
  if ( status == eTIesrEngineSuccess ) search_a_frame(NULL, 0, global_var->frm_cnt - 1, global_var );
  if ( status == eTIesrEngineSuccess ) {
    if (global_var->best_sym != USHRT_MAX ) /* backtrace and print result */
#ifdef USE_NBEST
      {
	if (global_var->bUseNBest)
	  status = nb_back_trace_beam(global_var->best_sym, global_var->frm_cnt - 1,  
				      global_var->hmm_code,  global_var->stt,  global_var->stp, 
				      &(global_var->nbr_seg), &(global_var->hmm_dlt), global_var);
	else
	  status = back_trace_beam(global_var->best_sym, global_var->frm_cnt - 1,  
				   global_var->hmm_code,  global_var->stt,  global_var->stp, 
				   &(global_var->nbr_seg), &(global_var->hmm_dlt), global_var);
      }
#else
     {
#ifdef _MONOPHONE_NET
      status = nb_back_trace_beam(global_var->best_sym, global_var->frm_cnt - 1,  
				global_var->hmm_code,  global_var->stt,  global_var->stp, 
				&(global_var->nbr_seg), &(global_var->hmm_dlt), global_var);
#else   //this is the regular deco
      //printf("===========regular==========\n");
      if (gvv->word_backtrace==WORDBT)
      status = back_trace_beam(global_var->best_word,  global_var->best_word_frm,
      				global_var->hmm_code,  global_var->stt,  global_var->stp, 
      				&(global_var->nbr_seg), &(global_var->hmm_dlt), global_var);
      else      				
      status = back_trace_beam(global_var->best_sym, global_var->frm_cnt - 1,  
      				global_var->hmm_code,  global_var->stt,  global_var->stp, 
      				&(global_var->nbr_seg), &(global_var->hmm_dlt), global_var);
      
				
				
#endif
     }
#endif
    else status = eTIesrEngineAlignmentFail;
  }
  if (global_var->nbr_seg > MAX_NBR_SEGS)  status = eTIesrEngineSegmentMemoryOut;
  //  fprintf(stderr,"maximum (MAX_NBR_SEGS = %d) capacity exceeded\n", MAX_NBR_SEGS);
#ifdef USE_AUDIO
  global_var->vol = set_volume_flag(global_var);
#endif
#ifdef WORK_ON_SEARCH 
  GetSearchInfo(global_var); 
#endif

#ifdef _MONOPHONE_NET
  rs_CopyAlignFrom( global_var , gvv ); 
#endif

#ifdef USE_NBEST
  nb_clear_time_mark(global_var);
#endif

  return status;
}

/* GMHMM_SI_API */ 
/* 
short GetFrameCount(void *g)
{
  return   ((gmhmm_type *)g)->frm_cnt;
}
*/

/*
** best cumulated score
*/

/* GMHMM_SI_API */ 
/*
long GetTotalScore(void *g)
{
  return   ((gmhmm_type *)g)->best_sym_scr;
}
*/

/* GMHMM_SI_API */
/*
 ushort  GetCycleCount(void *g)
{
  return ((gmhmm_type *)g)->nbr_cpy;
}
*/

/* GMHMM_SI_API */ 
/*
ushort GetAnswerCount(void const *gv) 
{
  return ((gmhmm_type *)gv)->nbr_ans;
}
*/


/* GMHMM_SI_API */ 
/*
short GetVolumeStatus(void *gv)
{
  return ((gmhmm_type *)gv)->vol;
}
*/


/* GMHMM_SI_API */ 
/* 
unsigned short *GetAnswerIndex(void *gv)
{
  return ((gmhmm_type *)gv)->answer;
}
*/


/* GMHMM_SI_API */ 
/*
const char *GetAnswerWord(unsigned short i, void *gvv)
{
  gmhmm_type *gv = (gmhmm_type *)gvv;
  return gv->vocabulary[ gv->answer[ gv->nbr_ans - 1 - i ] ];
}
*/

/*
** return water mark search space size
*/

unsigned short GetSearchMemorySize(gmhmm_type *gvv)
{
  gmhmm_type *gv = (gmhmm_type *)gvv;
  return SEARCH_SPACE(gv->evalstat);
}

/*
void SetTIesrPrune(void *gv, short value)
{
  ((gmhmm_type *)gv)->prune = value * (1<<6);
}
*/

/*
void GetTIesrPrune(void *gv, short *value)
{
  *value = (((gmhmm_type *)gv)->prune) >> 6;  
}
*/


/*
void SetTIesrTransiWeight(void *gv, short value)
{
  ((gmhmm_type *)gv)->tranwgt = (value)*(1<<6);
}
*/

/*
void GetTIesrTransiWeight(void *gv, short *value)
{
  *value = (((gmhmm_type *)gv)->tranwgt) >> 6; 
}
*/
