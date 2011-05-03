/*=======================================================================
 TIesrFlex.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This source defines the functionality that allows a user to
 dynamically generate a TIesr binary grammar network file and a set
 of binary HMM model files for a specific grammar that is input as a
 string.  These files can be used by TIesr to create dynamic, state
 dependent recognition results.

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

// C++ include files
#include <cstdlib>
#include <new>
#include <cstring>
#include <cctype>


#include "TIesrFlex.h"
#include "TIesrFlexLocal.h"


/* Windows OS specific DLL entry */
#if defined (WIN32) || defined (WINCE)
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


#ifdef USE_NET_OPTIMIZE

#include "nfa_basic.h"
#include "nettype.h"
#include "autotype.h"
#include "interface.h"
#include "automaton_io.h"
#include "genlist.h" /* list processing macros */
#include "gen_1d.h"  /* one-D array processing macros */

FREELIST(free_augm, AugmType)

static void free_augm_list(PtAugmType *p, int nbr)
{
   int i;
   for (i=0; i<nbr; i++) free_augm(p[i]);
}

SETCONST1D(init_cwd, PtAugmType)

#endif


//--------------------------------------------------------------------
// This is the constructor of a class that has been exported.
// see Grammar.h for the class definition
CTIesrFlex::CTIesrFlex(void)
{
   
   // initially no allocated data for questions and hmms
   qs_base = NULL;
   qs_offset = NULL;
   
   tree_base = NULL;
   treetbl_base = NULL;
   
   tran_base = NULL;
   tranoffset_base = NULL;
   m_numTrans = 0;
   tran_offset = NULL;
   tran_idx_aug = NULL;
   
   cluster_base = NULL;
   clusteroffset_base = NULL;
   m_numClusters = 0;
   cluster_offset = NULL;
   cluster_idx_aug = NULL;
   
   
   // Initialize mixture pdf clustered weight vector pointer
   weight_base = NULL;
   
   mean_base = NULL;
   mean_idx_aug = NULL;
   m_numMeans = 0;
   
   // Used by VQHMM for fast JAC and ORM
   mean_idx = NULL;
   
   var_base = NULL;
   var_idx_aug = NULL;
   m_numVars = 0;
   
   gconst_base = NULL;
   scale_base = NULL;
   sil_hmm = NULL;
   m_dict = NULL;
   
   
   #ifdef OFFLINE_CLS
   vq_centroid = NULL;
   pROM2Cls = NULL;
   #endif
   
   // initially there is no grammar loaded in this object
   cfg = NULL;
   
   // Allocation for storing pronunciations of words
   m_wordProns = NULL;
   
   // Initialize dynamic allocation of tokens in grammar parse
   token = NULL;
   m_tokenMax = 0;
   token_idx = 0;
   
   // Initialize dynamic allocation of rules
   rule = NULL;
   m_ruleMax = 0;
   rule_idx = 0;
   
   // Initialize dynamic allocation of nodes
   node = NULL;
   m_nodeMax = 0;
   node_idx = 0;
   
   // Initialize dynamic allocation of dependencies in grammar
   depend = NULL;
   m_dependMax = 0;
   depend_idx = 0;
   
   // Initialize dynamic allocation of sentnodes
   sentnode = NULL;
   m_sentnodeMax = 0;
   sentnode_idx = 0;
   
   // Initialize dynamic allocation of sentnodes
   senttran = NULL;
   m_senttranMax = 0;
   senttran_idx = 0;
   
   // Initialize dynamic allocation of words in grammar
   word = NULL;
   m_wordMax = 0;
   word_idx = 0;
   
   // Initialize dynamic allocation of hmm entries in grammar
   hmm = NULL;
   m_hmmMax = 0;
   hmm_idx = 0;
   
   // Initialize dynamic allocation of symbols, which are
   // grammar context-expanded hmms.
   sym = NULL;
   m_symMax = 0;
   sym_idx = 0;
   
   // Initialize dynamic allocation of list of stop symbols
   stop_next = NULL;
   m_stopMax = 0;
   n_stop_next = 0;
   
   return;
}


//--------------------------------
CTIesrFlex::~CTIesrFlex(void)
{
   
   // Remove input codebook information
   // This includes dynamic arrays used for data output
   free_codebook();
   
   // Remove any existing word pronunciations
   delete [] m_wordProns;
   
   
   #ifdef OFFLINE_CLS
   FreeVQHMM();
   #endif
   
   // remove the dictionary
   delete m_dict;
   
   // Free all of the grammar structures allocated
   FreeGrammarData();
   
   return;
}


#ifdef OFFLINE_CLS

//--------------------------------
short CTIesrFlex::LoadVQHMM(const char* dirname)
{
   char buf[FILENAME_MAX];
   short nread , sz_ROMMean;
   FILE *fp;
   
   
   /* read ROM mean to cluster mapping */
   /* load RAM mean to ROM mean index mapping */
   
   // Load ROM mean to cluster mapping information
   
   // Open ROM mean to cluster class mapping file
   strcpy( buf, dirname);
   strcat( buf, "/ROM2cls.bin");
   fp = fopen( buf, "rb");
   if ( !fp )
      goto FailedLoad;
   
   // Allocate space for ROM to cluster mapping table
   nread = fread(&sz_ROMMean, sizeof(short), 1, fp);
   nread = fread(&vq_n_cs, sizeof(short), 1, fp);
   if( nread != 1 )
      goto FailedLoad;
   pROM2Cls = (short*) malloc(sizeof(short)*sz_ROMMean);
   if( ! pROM2Cls )
      goto FailedLoad;
   
   // Read the mapping table
   nread = fread(pROM2Cls, sizeof(short), sz_ROMMean, fp);
   if( nread != sz_ROMMean )
      goto FailedLoad;
   fclose(fp);
   
   
   // Read cluster centroid information
   
   // Open cluster centroid file
   strcpy( buf, dirname);
   strcat( buf, "/cls_centr.bin");
   fp = fopen( buf, "rb");
   if( !fp )
      goto FailedLoad;
   
   // Allocate space for centroids
   nread = fread(&vq_n_cs, sizeof(short), 1, fp);
   nread = fread(&vq_nbr_dim, sizeof(short), 1, fp);
   if( nread != 1 )
      goto FailedLoad;
   vq_centroid = (short*) malloc(sizeof(short)* vq_n_cs * vq_nbr_dim);
   if( !vq_centroid )
      goto FailedLoad;
   
   // Read vq centroids
   nread = fread(vq_centroid, sizeof(short), vq_n_cs * vq_nbr_dim, fp);
   if( nread != vq_n_cs * vq_nbr_dim )
      goto FailedLoad;
   fclose( fp );
   
   return OK;
   
   
   // Failure to load successfully
   FailedLoad:
      if( fp )
      {
         fclose(fp);
      }
      
      FreeVQHMM();
      
      return FAIL;
      
}



//--------------------------------
short CTIesrFlex::SaveVQHMM(const char* dirname)
{
   char buf[FILENAME_MAX];
   FILE *fp;
   unsigned char* pRAM2Cls;
   short nwrite;
   short i, im;
   
   
   // Allocate RAM to cluster mapping arrray
   im = (mean_idx_cnt&0x0001) ?  mean_idx_cnt + 1 : mean_idx_cnt + 2;
   pRAM2Cls = (unsigned char*) malloc(sizeof(unsigned char) * im );
   if( !pRAM2Cls )
      return FAIL;
   
   /* Determine mapping of RAM mean to ROM mean cluster index */
   for (i=0;i<mean_idx_cnt;i++)
   {
      im = mean_idx[i];
      im = pROM2Cls[im];
      pRAM2Cls[i] = (unsigned char)im;
   }
   
   
   /* Open file to save table mapping RAM mean to ROM mean cluster class */
   strcpy( buf, dirname);
   strcat( buf, "/o2amidx.bin" );
   fp = fopen( buf, "wb" );
   if( !fp )
      goto SaveFail;
   
   // Write the table
   i = mean_idx_cnt;
   
   // Must write proper endian format
   // fwrite(&i, sizeof(short), 1, fp);
   int failed;
   failed = output_int16( fp, i );
   if( failed )
      goto SaveFail;
   
   // Indices are all unsigned char so don't care about endian format
   nwrite = fwrite(pRAM2Cls, sizeof(unsigned char), i, fp);
   if(  nwrite != i )
      goto SaveFail;
   
   // Finished writing mean to centroid mapping
   fclose(fp);
   free(pRAM2Cls);
   pRAM2Cls = NULL;
   
   
   /* Open file to hold cluster centriods */
   strcpy(buf, dirname);
   strcat( buf, "/vqcentr.bin");
   fp = fopen(buf, "wb");
   if( !fp )
      goto SaveFail;
   
   // Write the centroid information
   // Must write in proper endian format
   // nwrite = fwrite(&vq_n_cs, sizeof(short), 1, fp);
   failed = output_int16( fp, vq_n_cs );
   if( failed )
      goto SaveFail;
   
   // nwrite = fwrite(&vq_nbr_dim, sizeof(short), 1, fp);
   failed = output_int16( fp, vq_nbr_dim );
   if( failed )
      goto SaveFail;
   
   // nwrite = fwrite(vq_centroid, sizeof(short), vq_n_cs * vq_nbr_dim, fp);
   for( int numEl = 0; numEl < vq_n_cs*vq_nbr_dim; numEl++ )
   {
      failed = output_int16( fp, vq_centroid[numEl] );
      if( failed )
         goto SaveFail;
   }
   
   
   fclose( fp );
   
   return OK;
   
   
   // Failed to save mapping or vq centroids
   SaveFail:
      
      if( fp )
         fclose(fp);
      
      
      if( pRAM2Cls )
      {
         free( pRAM2Cls );
      }
      
      return FAIL;
}


/*--------------------------------
 FreeVQHMM

 Free space allocated for codebook mean to cluster class map and
 codebook mean cluster class centroids.
 --------------------------------*/
void CTIesrFlex::FreeVQHMM()
{
   if( pROM2Cls )
   {
      free(pROM2Cls);
      pROM2Cls = NULL;
   }
   
   if( vq_centroid )
   {
      free(vq_centroid);
      vq_centroid = NULL;
   }
   
   return;
}

#endif


//-----------------------------
CTIesrFlex::Errors CTIesrFlex::LoadLanguage( const char *direct,
const char *language,
const char *dict_ident,
const char *hmm_ident,
const int lit_end,
const int addClosure )
{
   char dir[FILENAME_MAX];
   char fname[FILENAME_MAX];
   
   // Symbian OS
   //TText tfname[MAX_STR];
   //TBufC<MAX_STR> dfname;
   
   int failed;
   
   CTIesrDict::Errors dError;
   
   // specify whether to output little-endian
   little_endian = lit_end;
   
   // free any existing model data and dynamic structures for model output
   free_codebook();
   
   
   // Construct the dictionary for looking up words
   // This dictionary is a class member
   try
   { m_dict = new CTIesrDict(); }
   catch( std::bad_alloc &ex )
   {
      return ErrFail;
   }
   
   dError = m_dict->LoadDictionary( direct, language, dict_ident, addClosure );
   if( dError != CTIesrDict::ErrNone )
   {
      delete m_dict;
      m_dict = NULL;
      return ErrDict;
   }
   
   // number of phones in dictionary. Does not include 'sil' model
   n_phone = m_dict->GetPhoneCount();
   
   // directory containing the hmm information
   strcpy( dir, direct);
   strcat( dir, "/" );
   strcat( dir, language );
   strcat( dir, "/" );
   strcat( dir, hmm_ident );
   strcat( dir, "/" );
   
   
   // For PC, load files to RAM
   // For embedded device, pointers to data in ROM would save RAM
   
   
   // Obtain the configuration of the input tree and HMM information
   strcpy( fname, dir );
   strcat( fname, "fxconfig.bin" );
   failed = read_config( fname );
   if( failed )
      goto readfail;
   
   // For now can not handle CMN byte means.  There is not software to
   // expand byte means to short means and add the bias.
   if( m_bInputByteMeans && m_bInputCMN )
      return ErrType;
   
   // Read in scale data if mean or variance in bytes
   if( m_bInputByteMeans || m_bInputByteVars )
   {
      strcpy( fname, dir );
      strcat( fname, "scale.bin" );
      failed = read_scale( fname );
      if( failed )
         goto readfail;
   }
   
   
   // File of questions and establish pointers
   // for DSP want pointers to questions and question table in ROM
   // questions
   strcpy( fname, dir );
   strcat( fname, "_qs.bin" );
   
   
   // Determine if questions file exists.
   // If it does not, assume input describes a monophone HMM set
   FILE* fp;
   fp = fopen( fname, "r" );
   if( fp )
   {
      fclose(fp);
      m_bInputMonophone = false;
   }
   else
   {
      m_bInputMonophone = true;
   }
   
   // If input is not monophone, then read acoustic decision tree data,
   // since the input must describe a triphone HMM set
   if( !m_bInputMonophone )
   {
      failed = read_qs( fname );
      if( failed )
         goto readfail;
      
      // question table - offsets of questions, turned into addresses
      // by read_qstbl
      strcpy( fname, dir );
      strcat( fname, "_qstbl.bin" );
      failed = read_qstbl( fname );
      if( failed )
         goto readfail;
      
      // find pointers to tree
      strcpy( fname, dir );
      strcat( fname, "_tree.bin" );
      failed = read_tree( fname );
      if( failed )
         goto readfail;
      
      // read tree table, and adjust to memory locations
      strcpy( fname, dir );
      strcat( fname, "_treetbl.bin" );
      failed = read_treetbl( fname );
      if( failed )
         goto readfail;
   }
   
   // find pointers to hmm transition info and offsets in ROM
   strcpy( fname, dir );
   strcat( fname, "fxtran.cb" );
   failed = read_fxtran( fname );
   if( failed )
      goto readfail;
   
   strcpy( fname, dir );
   strcat( fname, "fxtran.off" );
   failed = read_fxtroff( fname );
   if( failed )
      goto readfail;
   

   
   // find pointers to hmm clustered pdf info in ROM
   strcpy( fname, dir );
   strcat( fname, "fxclust.cb" );
   failed = read_fxclust( fname);
   if( failed )
      goto readfail;
   
   //find pointers to read the base file
   if(m_bInputWeightConfig != 0 )
   {
      strcpy( fname, dir );
      strcat( fname, "fxweight.cb" );
      failed = read_weight(fname);
      if( failed )
         goto readfail;
   }
   
   strcpy( fname, dir );
   strcat( fname, "fxclust.off" );
   failed = read_fxcloff( fname);
   if( failed )
      goto readfail;
   

   
   // find pointers to hmm mean, variance, and gconst information in ROM
   strcpy( fname, dir );
   strcat( fname, "fxmean.cb" );
   fp = fopen(fname, "r");
   if(fp != NULL)
   {
      fclose(fp);
      failed = read_mean( fname );
      if(failed)
         goto readfail;
   }
   else
   {
      failed = makemean(dir);
      if( failed )
         goto  readfail;
   }
   strcpy( fname, dir );
   strcat( fname, "fxvar.cb" );
   failed = read_var( fname );
   if( failed )
      goto readfail;
   
   strcpy( fname, dir );
   strcat( fname, "fxgconst.cb" );
   failed = read_gconst( fname );
   if( failed )
      goto readfail;
   
   // get silence hmm number of states and pointer to hmm info
   strcpy( fname, dir );
   strcat( fname, "fxsil.hmm" );
   failed = read_sil( fname );
   if( failed )
      goto readfail;
   
   
   #ifdef OFFLINE_CLS
   failed = LoadVQHMM(dir);
   if (failed)
      goto readfail;
   #endif
   
   return ErrNone;
   
   readfail:
      delete m_dict;
      m_dict = NULL;
      
      free_codebook();
      
      #ifdef OFFLINE_CLS
      FreeVQHMM();
      #endif
      
      return ErrFail;
}


//------------------------------------------------
CTIesrFlex::Errors CTIesrFlex::ParseGrammar( const char * cfg_string,
unsigned int aMaxPronunciations,
int aIncludeRulePron,
int aAutoSilence )
{
   int result;
   CTIesrFlex::Errors error;
   
   // Free any preexisting data
   FreeGrammarData();
   
   // Allocate a new copy of the grammar text
   try
   { cfg = new char[strlen(cfg_string) + 1]; }
   catch( std::bad_alloc &ex )
   {
      return ErrMemory;
   }
   
   strcpy( cfg, cfg_string );
   
   
   // Maintain a copy of the autosilence request
   m_bAutoSilence = aAutoSilence;
   
   
   // Initialize variables for the parse.  Since this includes allocating
   // space for structures, it must be tested for throwing exceptions.
   try
   {
      init();
   }
   catch( char const* err )
   {
      return ErrMemory;
   }
   
   
   // parse the grammar.  Note that exception handling must be
   // checked within yyparse so that if stack memory is allocated then
   // it can be released appropriately when an exception is raised.
   result = yyparse();
   if( result != 0 )
      return ErrFail;
   
   
   // At the conclusion of yyparse, an fsa2 word network (Mealy network
   // with words on transitions) has been created.  Now, if multiple
   // pronunciations are desired, the multiple words must be added to
   // the fsa2 network.
   m_bIncludeRulePron = aIncludeRulePron;
   m_maxPronunciations = aMaxPronunciations;
   
   // Keep track of the number of base word entries before adding multiple
   // pronunciations.
   m_baseWordIndex = word_idx;
   
   if( aMaxPronunciations > 1 )
   {
      error = AddMultiplePronunciations( );
      if( error != ErrNone )
         return error;
   }
   
   
   // Create structure to hold pronunciations of words
   // Avoids doing lookup of pronunciations twice; once in
   // create_hmm and once in create_sym_net.
   try
   { m_wordProns = new Pron_t[word_idx]; }
   catch( std::bad_alloc &ex )
   {
      return ErrFail;
   }
   
   
   // Create hmm[] list. This function makes a list of all of the
   // context-sensitive HMMs that are needed for the grammar.
   // create_hmm uses 'demand', so may raise an exception.
   
   /* Old WinCE version
    __try
    {
    create_hmm();
    }
    __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
    {
    m_parseFail = true;
    }
    */
   
   // C++ version
   try
   {
      create_hmm();
   }
   catch( char* exc )
   {
      m_parseFail = true;
   }
   if( m_parseFail )
   {
      delete [] m_wordProns;
      m_wordProns = NULL;
      return ErrFail;
   }
   
   
   /* --------------------------------------------------
    create sym net (sym[]) from fsa2:
   
    start_sym and stop_sym are extra,
    they do not have hmm associated,
    sym_idx does not count these two sym
    -------------------------------------------------- */
   
   // These should be handled by FreeGrammarData now
   // sym_idx = 0;
   // n_stop_next = 0;
   
   clear_sentnode_visit_flag();
   
   // create_sym_net uses 'demand' function, so may raise an
   // exception
   
   /* old WinCE version
    __try
    {
    create_sym_net( &sentnode[ 0 ] );
    }
    __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
    {
    m_parseFail = true;
    }
    */
   
   // C++ version
   try
   {
      create_sym_net(  0 );
   }
   catch( char* exc )
   {
      m_parseFail = true;
   }
   if( m_parseFail )
   {
      delete [] m_wordProns;
      m_wordProns = NULL;
      return ErrFail;
   }
   
   //  printf("n_hmm = %d, n_sym = %d, n_start_sym = %d, n_stop_sym = %d\n",
   //   hmm_idx * 2,        /* male & female */
   //   sym_idx, sentnode[ 0 ].n_next, n_stop_next);
   
   /* --------------------------------------------------
    debug: expand sym net
    Check if the sym net (sym[], hmm[]) is correct?
    you can choose to print either phone (0) or word (1)
    -------------------------------------------------- */
   /*
    buf[0] = '\0';
   
    expand_sym_net( sentnode[ 0 ].n_next, sentnode[ 0 ].next, buf, 1 );
    */
   
   /* --------------------------------------------------
    output models
    -------------------------------------------------- */
   
   // Now a separate member function
   //output_models( argv[3] );
   
   
   // Done with pronunciation
   delete [] m_wordProns;
   m_wordProns = NULL;
   
   return ErrNone;
}

//--------------------------------
CTIesrFlex::Errors CTIesrFlex::DownloadGrammar()
{
   int failed;
   
   // TODO: figure out how to construct and download all of the
   // binary data to device memory.  This may be to the DSP via a DSP Bridge.
   // This will include several existing functions in flexphone to
   // create the binary data, but it must be loaded to memory a chunk
   // at a time, not written out to files, as was done in the original
   // function output_models().
   
   
   failed = download_net();
   if( failed )
      return ErrFail;
   
   failed = download_hmm2phone();
   if( failed )
      return ErrFail;
   
   failed = download_gtm();
   if( failed )
      return ErrFail;
   
   return ErrNone;
}



/*-----------------------------------------------------
 OutputGrammar

 This function outputs the grammar network file and HMM model files
 that can be used by TIesrSI to recognize the grammar.  The user can
 specify the directory where the files are to be written, and whether
 byte means or byte variances are desired in the output model set.

 Further, the user can specify if the grammar is to be optimized
 after being output in non-optimized format.  This processing step
 requires additional time, and requires the TIesrOptimize API.  This
 API is not needed if the USE_NET_OPTIMIZATION macro is not defined.
 ----------------------------------------------------------------*/
CTIesrFlex::Errors CTIesrFlex::OutputGrammar(char *aDirName, int aByteMeans, int aByteVars ,
int aOptimize )
{
   int failed;
   
   
   // TODO: figure out how to construct and download all of the
   // binary data to device memory.  This may be to the DSP via a DSP Bridge.
   // This will include several existing functions in flexphone to
   // create the binary data, but it must be loaded to memory a chunk
   // at a time, not written out to files, as was done in the original
   // function output_models().
   
   
   // Check for validity of byte/short output request.  There is not software
   // in TIesrFlex available to expand byte means or vars to short means or vars.
   if( (m_bInputByteMeans && !aByteMeans) ||
   (m_bInputByteVars && !aByteVars) )
   {
      return ErrType;
   }
   
   
   failed = output_net( aDirName, "/net.bin" );
   
   if( failed )
      return ErrFail;
   
   failed = output_hmm2phone( aDirName );
   if( failed )
      return ErrFail;
   
   failed = output_gtm( aDirName );
   if( failed )
      return ErrFail;
   
   
   // If input mean or variance are in short, and output is requesting
   // byte, then convert to byte.
   if( ( ! m_bInputByteMeans  && aByteMeans ) ||
   ( ! m_bInputByteVars && aByteVars ) )
   {
      // Note: bias has already been added to short mean vectors in the file
      failed = model_bit_16_to_8( aDirName, aDirName, true, 0, aByteMeans, aByteVars );
   }
   if( failed )
      return ErrFail;
   
   
   // Output the configuration file for the models, which specifies
   // the type of mean and variance vectors, byte or short, and
   // the endian of the data.
   failed = output_config( aDirName, aByteMeans, aByteVars );
   if( failed )
      return ErrFail;
   
   
   #ifdef OFFLINE_CLS
   SaveVQHMM(aDirName);
   #endif
   
   
   #ifdef USE_NET_OPTIMIZE
   
   if( aOptimize )
   {
      /* The code below was for quick implmentation of network
       * optimization.  These functions are
       * not API compliant - they do not check mallocs etc. Not to be used
       * now. */
      
      NetTransType trans;
      int disp_automaton = false;
      StateType *nfa;
      int nbr_labels, nbr_nfa_states, initial_state, eps_order;
      PtAugmType *composed_words;
      char inputnet[300], outputnet[300];
      
      
      strcpy(inputnet, aDirName);
      strcat(inputnet, "/net.bin.org");
      strcpy(outputnet, aDirName);
      strcat(outputnet, "/net.bin");
      nbr_nfa_states = read_net(  inputnet, "ueslesshmmlist", &trans);    /* net file */
      if (nbr_nfa_states==0) return ErrFail;
      
      nfa = newmem(StateType, nbr_nfa_states);
      composed_words = newmem(PtAugmType, trans.n_word);
      
      init_cwd( composed_words, trans.n_word, NULL);
      initial_state = convert_to_nfa(&trans, nfa, &nbr_labels, composed_words);
      
      eps_order = nbr_labels -1; /* always the last (the last is EPS) */
      call_determinize_min(nfa, nbr_labels - 1, nbr_nfa_states, initial_state,
      composed_words, trans.hmm_name, trans.n_hmm,
      trans.words, trans.n_word, outputnet, eps_order, disp_automaton);
      
      free_net(&trans);  /* placed here because c.d.m. uses hmm, wrd */
      free_augm_list(composed_words, trans.n_word);
      free(composed_words);
      free_automaton(nfa, nbr_nfa_states);
   }
   else
   {
      return ErrNone;
   }
   
   #else
   
   if( aOptimize )
      return ErrNoOptimize;
   else
      return ErrNone;
   
   #endif
   
   return ErrNone;
}


//----------------------------------------------------------------
//  Private CTIesrFlex class function implementation


//--------------------------------
void CTIesrFlex::init()
{
   // Initial rule allocation.  m_ruleMax should be zero.
   if( 0 >= m_ruleMax )
   {
      rule = reinterpret_cast<Grule*>
      ( ExpandArray( reinterpret_cast<void*>(rule), sizeof(Grule), RULE_ALLOC, &m_ruleMax ) );
   }
   
   /* accommodate null grammar, enrollment only */
   rule[0].node = -1;
   rule[0].name = NULL;
   
   
   //for lexical analysis - local replacement for yylex
   cfg_char = cfg;
   
   // For exception handling
   m_parseFail = false;
   
   
   //yylineno = 1;
}

//-------------------------------------------------------------
// This yylex function, which supports the yyparse grammar parser
// is written by hand to avoid using the lex utility
// since the lexical analyzer is simple, and it will avoid use
// of global tables.
//

//int CTIesrFlex::yylex( int *yylval )
int CTIesrFlex::yylex( long *yylval )
{
   
   
   char newtoken[MAX_WORDLEN];
   int  newtokidx = 0;
   char* tokensave;
   
   
   // Find next token
   while( true )
   {
      while( *cfg_char == ' ' || *cfg_char == '\n' || *cfg_char == '\t' )
      {
         cfg_char++;
      }
      
      if( *cfg_char == '\0')
         return 0;
      
      else if( *cfg_char == '(' )
      {
         cfg_char++;
         return LPRN;
      }
      
      else if( *cfg_char == ')' )
      {
         cfg_char++;
         return RPRN;
      }
      
      else if( *cfg_char == '[' )
      {
         cfg_char++;
         return LBKT;
      }
      
      else if( *cfg_char == ']' )
      {
         cfg_char++;
         return RBKT;
      }
      
      else if( *cfg_char == '|' )
      {
         cfg_char++;
         return VBAR;
      }
      
      
      else if( *cfg_char == '.' )
      {
         cfg_char++;
         return PERIOD;
      }
      
      else if( strncmp( cfg_char, "--->", 4) == 0 )
      {
         cfg_char += 4;
         return ARROW;
      }
      
      else if( strncmp( cfg_char, "start", 5 ) == 0 )
      {
         cfg_char += 5;
         return START;
      }
      
      else if( myisalpha( (int) *cfg_char ) || *cfg_char == '-'
      || *cfg_char == '\'' || *cfg_char == '_' )
      {
         while( myisalpha( (int) *cfg_char ) || *cfg_char == '-'
         || *cfg_char == '\'' || *cfg_char == '_' )
         {
            newtoken[ newtokidx++ ] = *cfg_char++;
         }
         
         newtoken[newtokidx] = '\0';
         
         // Check if all token pointers in the token pointer array are used
         if( token_idx >= m_tokenMax )
         {
            // Allocate TOKEN_ALLOC more tokens in the dynamic token array.
            // This will throw an exception if the expansion fails.
            token = reinterpret_cast<char**>
            ( ExpandArray( reinterpret_cast<void*>(token), sizeof(char*), TOKEN_ALLOC, &m_tokenMax ) );
         }
         
         tokensave = (char*)malloc( strlen(newtoken) + 1 );
         if( !tokensave )
            throw( "Token malloc fail in yylex" );
         
         // Save the character string pointer in order to free it later
         token[token_idx] = tokensave;
         token_idx++;
         
         // Send yyparse the location of the new token character string
         // *yylval = (int) strcpy( tokensave, newtoken );
         *yylval = (long) strcpy( tokensave, newtoken );
         return NAME;
      }
      
      else
      {
         cfg_char++;
      }
   }
}


//-------------------------------------------------------------------------
// This is the generated grammar string parse function.
//--------------------------------------------------------------------------
int CTIesrFlex::yyparse()
{
   
   register int yystate;
   register int yyn;
   register short *yyssp;
   
   // register int *yyvsp;
   register long *yyvsp;

   int yyerrstatus;
   int yychar1 = 0;
   
   short yyssa[200];

   // int yyvsa[200];
   long yyvsa[200];
   
   short *yyss = yyssa;

   // int *yyvs = yyvsa;
   long *yyvs = yyvsa;

   int yystacksize = 200;
   int yyfree_stacks = 0;
   
   int yychar;
   
   // int yylval;
   long yylval;

   int yynerrs;
   
   // int yyval;
   long yyval;
   
   int yylen;
   
   yystate = 0;
   yyerrstatus = 0;
   yynerrs = 0;
   yychar = -2;
   
   yyssp = yyss - 1;
   yyvsp = yyvs;
   
   yynewstate:
      
      *++yyssp = yystate;
      
      if (yyssp >= yyss + yystacksize - 1)
      {
         
         // maintain pointers to old stacks
         
          // int *yyvs1 = yyvs;
         long *yyvs1 = yyvs;
         
         short *yyss1 = yyss;
         int yystacksize1 = yystacksize;
         
         
         int size = yyssp - yyss + 1;
         
         // if present stack size is maximum allowed, then exit with failure
         if (yystacksize >= 10000)
         {
            yyerror("parser stack overflow");
            if (yyfree_stacks)
            {
               free(yyss);
               free(yyvs);
               
            }
            return 2;
         }
         // create new stacks with larger size limited by max
         yystacksize *= 2;
         if (yystacksize > 10000)
            yystacksize = 10000;
         
         yyfree_stacks = 1;
         
         yyss = (short *) malloc(yystacksize * sizeof (*yyssp));
         if( yyss )
            __yy_memcpy((char *)yyss, (char *)yyss1,
            size * (unsigned int) sizeof (*yyssp));
         
         // yyvs = (int *) malloc(yystacksize * sizeof (*yyvsp));
         yyvs = (long *) malloc(yystacksize * sizeof (*yyvsp));

         if( yyvs )
            __yy_memcpy((char *)yyvs, (char *)yyvs1,
            size * (unsigned int) sizeof (*yyvsp));
         
         // free old malloc allocated stacks
         if( yystacksize1 > 200 )
         {
            free(yyss1);
            free(yyvs1);
         }
         
         // return if malloc fail
         if( yyss == 0 || yyvs == 0 )
            goto yyabortlab;
         
         
         yyssp = yyss + size - 1;
         yyvsp = yyvs + size - 1;
         
         if (yyssp >= yyss + yystacksize - 1)
            goto yyabortlab;
      }
      
      goto yybackup;
      yybackup:
         
         yyn = yypact[yystate];
         if (yyn == -32768)
            goto yydefault;
         
         if (yychar == -2)
         {
            
            // The TIesrFlex specific yylex keeps a running store of malloced tokens
            // and will throw an exception if not able to malloc space.  If this fails
            // then yyparse must abort, freeing its stacks.
            try
            {
               yychar = yylex(&yylval);
            }
            catch( char* ex )
            {
               goto yyabortlab;
            }
         }
         
         if (yychar <= 0)
         {
            yychar1 = 0;
            yychar = 0;
            
         }
         else
         {
            yychar1 = ((unsigned)(yychar) <= 265 ? yytranslate[yychar] : 18);
            
            
         }
         
         yyn += yychar1;
         if (yyn < 0 || yyn > 26 || yycheck[yyn] != yychar1)
            goto yydefault;
         
         yyn = yytable[yyn];
         
         
         if (yyn < 0)
         {
            if (yyn == -32768)
               goto yyerrlab;
            yyn = -yyn;
            goto yyreduce;
         }
         else if (yyn == 0)
            goto yyerrlab;
         
         if (yyn == 27)
            goto yyacceptlab;
         
         if (yychar != 0)
            yychar = -2;
         
         *++yyvsp = yylval;
         
         
         if (yyerrstatus) yyerrstatus--;
         
         yystate = yyn;
         goto yynewstate;
         
         
         yydefault:
            
            yyn = yydefact[yystate];
            if (yyn == 0)
               goto yyerrlab;
            
            
            yyreduce:
               yylen = yyr2[yyn];
               if (yylen > 0)
                  yyval = yyvsp[1-yylen];
               
               // Each call to functions that may raise an exception
               // must be guarded
               switch (yyn)
               {
                  
                  case 1:
                  {
                     strcpy(start_name, (char *) yyvsp[-2]);
                     
                     /*
                      __try
                      {
                      process_grammar();
                      }
                      __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
                      {
                      m_parseFail = true;
                      }
                      ;*/
                     
                     try
                     {
                        process_grammar();
                     }
                     catch( char* exc )
                     {
                        //if( exc == CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     
                     break;
                  }
                  case 2:
                  {
                     strcpy(start_name, (char *) yyvsp[-3]);
                     
                     /*
                      __try
                      {
                      process_grammar();
                      }
                      __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
                      {
                      m_parseFail = true;
                      }
                      ; */
                     
                     
                     try
                     {
                        process_grammar();
                     }
                     catch( char* exc )
                     {
                        //if( exc == CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     
                     break;
                  }
                  case 5:
                  {
                     /*
                      __try
                      {
                      yyval = (int) create_rule( (char *) yyvsp[-3], (Gnode *) yyvsp[-1] );
                      }
                      __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
                      {
                      m_parseFail = true;
                      }
                      ;*/
                     
                     
                     try
                     {
                        yyval = (int) create_rule( (char *) yyvsp[-3], (int) yyvsp[-1] );
                     }
                     catch( char* exc )
                     {
                        // if( exc ==  CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     break;
                  }
                  case 6:
                  {
                     /*
                      __try
                      {
                      yyval = (int) create_node((char *) yyvsp[0], NULL, NULL, 0);
                      }
                      __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
                      {
                      m_parseFail = true;
                      }
                      ; */
                     
                     try
                     {
                        yyval = (int) create_node((char *) yyvsp[0], -1, -1, 0);
                     }
                     catch( char* exc )
                     {
                        //if( exc ==  CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     break;
                  }
                  case 7:
                  {
                     yyval = yyvsp[-1];
                     ;
                     break;}
                  case 8:
                  {
                     /*
                      __try
                      {
                      yyval = (int) process_optional( (int) yyvsp[-1] );
                      }
                      __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
                      {
                      m_parseFail = true;
                      }
                      ;*/
                     
                     try
                     {
                        yyval = (int) process_optional( (int) yyvsp[-1] );
                     }
                     catch( char* exc )
                     {
                        // if( exc ==  CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     break;
                  }
                  case 9:
                  {
                     yyval = yyvsp[0];
                     ;
                     break;}
                  case 10:
                  {
                     /*
                      __try
                      {
                      attach_at_end(( Gnode *) yyvsp[-1], ( Gnode *) yyvsp[0]);
                      }
                      __except( GetExceptionCode() == CTIesrFlex::STATUS_FAIL_PARSE )
                      {
                      m_parseFail = true;
                      }
                      */
                     
                     try
                     {
                        attach_at_end( (int) yyvsp[-1], (int) yyvsp[0]);
                     }
                     catch( char* exc )
                     {
                        //if( exc ==  CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     yyval = yyvsp[-1];
                     
                     break;
                  }
                  case 11:
                  {
                     yyval = yyvsp[0];
                     ;
                     break;}
                  case 12:
                  {
                     try
                     {
                        /*// the function attach_at_para_end_tree constructs prefix tree
                         // of the symbol network.
                         // However, it is not yet the default
                         attach_at_para_end_tree((Gnode *) yyvsp[-2], ( Gnode *) yyvsp[0]);
                         */
                        attach_at_para_end( (int) yyvsp[-2], (int) yyvsp[0] );
                     }
                     catch( char* exc )
                     {
                        //if( exc ==  CTIesrFlex::STATUS_FAIL_PARSE )
                        m_parseFail = true;
                     }
                     
                     yyval = yyvsp[-2];
                     ;
                     break;
                  }
               }
               
               // if an exception was raised in the switch statement, abort immediately
               if( m_parseFail )
                  goto yyabortlab;
               
               
               yyvsp -= yylen;
               yyssp -= yylen;
               
               
               *++yyvsp = yyval;
               
               
               yyn = yyr1[yyn];
               
               yystate = yypgoto[yyn - 12] + *yyssp;
               if (yystate >= 0 && yystate <= 26 && yycheck[yystate] == *yyssp)
                  yystate = yytable[yystate];
               else
                  yystate = yydefgoto[yyn - 12];
               
               goto yynewstate;
               
               yyerrlab:
                  
                  if (! yyerrstatus)
                     
                  {
                     ++yynerrs;
                     
                     
                     yyerror("parse error");
                  }
                  
                  goto yyerrlab1;
                  yyerrlab1:
                     
                     if (yyerrstatus == 3)
                     {
                        
                        if (yychar == 0)
                           goto yyabortlab;
                        
                        yychar = -2;
                     }
                     
                     
                     yyerrstatus = 3;
                     
                     goto yyerrhandle;
                     
                     yyerrdefault:
                        
                        
                        
                        yyerrpop:
                           
                           if (yyssp == yyss) goto yyabortlab;
                           yyvsp--;
                           yystate = *--yyssp;
                           
                           
                           
                           
                           yyerrhandle:
                              
                              yyn = yypact[yystate];
                              if (yyn == -32768)
                                 goto yyerrdefault;
                              
                              yyn += 1;
                              if (yyn < 0 || yyn > 26 || yycheck[yyn] != 1)
                                 goto yyerrdefault;
                              
                              yyn = yytable[yyn];
                              if (yyn < 0)
                              {
                                 if (yyn == -32768)
                                    goto yyerrpop;
                                 yyn = -yyn;
                                 goto yyreduce;
                              }
                              else if (yyn == 0)
                                 goto yyerrpop;
                              
                              if (yyn == 27)
                                 goto yyacceptlab;
                              
                              
                              
                              *++yyvsp = yylval;
                              
                              
                              yystate = yyn;
                              goto yynewstate;
                              
                              yyacceptlab:
                                 
                                 if (yyfree_stacks)
                                 {
                                    free(yyss);
                                    free(yyvs);
                                    
                                 }
                                 return 0;
                                 
                                 yyabortlab:
                                    
                                    if (yyfree_stacks)
                                    {
                                       if(yyss) free(yyss);
                                       if(yyvs) free(yyvs);
                                       
                                    }
                                    return 1;
                                    
                                    
}


// helper function needed by yyparse
void CTIesrFlex::__yy_memcpy(char *to, char *from, unsigned int count)
{
   register char *t = to;
   register char *f = from;
   register int i = count;
   
   while (i-- > 0)
      *t++ = *f++;
   
}


int CTIesrFlex::yywrap()
{
   return 1;
}

void CTIesrFlex::yyerror(const char *msg)
{
   //TODO: define what to do in case of an error in yy parser.
}


/*--------------------------------
 read_config

 This function reads the configuration file that defines the type of
 information in the input fixed point tree and HMM files.
 --------------------------------*/
int CTIesrFlex::read_config( char *fname )
{
   FILE *fp;
   size_t nread = 0;
   size_t wtconf = 0;
   size_t pkbyte = 0;
   
   // Attempt to open the configuration file.  If it can't be opened,
   // use defaults.
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      // Default assumptions made for input trees and HMMs
      
      // Feature vector dimensions
      m_inputFeatDim = 16;
      
      // Static feature vector dimensions
      m_inputStaticDim = 8;
      
      // Features are in 16-bit shorts
      m_bInputByteMeans = false;
      
      m_bInputByteVars = false;
      
      // Features are CMN mean normalized, and need bias added.
      // Bias is added as part of TIesrFlex converting means
      // to bytes.
      m_bInputCMN = true;
      
      // Features are not little endian
      m_bInputLittleEndian = false;
      
      // Male and female sets, denoted by :f and :m
      m_inputSets = 2;
      
      // Monophone transitions are set ("gender") independent
      m_bInputGDTrans = false;
      
      // Acoustic decision trees are set ("gender") dependent
      m_bInputGDTrees = true;
      
      return OK;
   }
   
   
   // Number of features in input mean and variance vectors
   // This may include delta and accel. in addition to static
   fread( &m_inputFeatDim, sizeof(int), 1, fp );
   
   // Static feature dimensions in input mean and variance vectors
   // This is only the number of static features.  If delta and acc
   // features are in the input, then delta and acc must also have
   // this number of dimensions.
   fread( &m_inputStaticDim, sizeof(int), 1, fp );
   
   // Flag, input feature means are in bytes
   fread( &m_bInputByteMeans, sizeof(int), 1, fp );
   
   // Flag, input feature inverse variances are in bytes
   fread( &m_bInputByteVars, sizeof(int), 1, fp );
   
   // Flag, data is mean normalized and needs mean added
   fread( &m_bInputCMN, sizeof(int), 1, fp );
   
   // Flag, data is in little endian format
   fread( &m_bInputLittleEndian, sizeof(int), 1, fp );
   
   // Number of "genders" or hmm model sets in data
   fread( &m_inputSets, sizeof(int), 1, fp );
   
   // Flag, gender-dependent transitions
   fread( &m_bInputGDTrans, sizeof(int), 1, fp );
   
   // Flag, gender-dependent trees
   nread = fread( &m_bInputGDTrees, sizeof(int), 1, fp );
   
   // incorporate the changes in the fxweight file
   // 0 for default
   // 1 for indexed weights more than one weight
   // 2 for weight clustering when each mixture class using one weight vector
   //   so the mixture index being used as the index;
   //   this also enables algorithmic methods of weight generation to be used.
   
   wtconf = fread(&m_bInputWeightConfig, sizeof(int), 1, fp);
   if (wtconf != 1)
   {
      m_bInputWeightConfig = 0;
   }
   
   pkbyte = fread(&m_bPackWtNmix, sizeof(int), 1, fp);
   if(pkbyte != 1)
   {
      m_bPackWtNmix = 0;
   }
   
   fclose( fp );
   
   if( nread != 1 )
   {
      return FAIL;
   }
   
   return OK;
}


/*--------------------------------
 read_scale

 This function reads word to byte scaling information.  This will
 only be read if the input mean and variance vector information
 is stored as bytes.  This information is output to the output
 scale.bin file.
 --------------------------------*/
int CTIesrFlex::read_scale( char *fname )
{
   FILE *fp;
   size_t nread = 0;
   size_t scaleSize;
   
   
   // The number of scales should be twice the number of features,
   // since we are getting the scales for mean and variance.  The
   // scales are in signed short.
   scaleSize =  2 * m_inputFeatDim * sizeof(short);
   scale_base = (short*) malloc( scaleSize );
   if( scale_base == NULL )
   {
      return FAIL;
   }
   
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   nread = fread( scale_base, sizeof(short), 2*m_inputFeatDim, fp );
   fclose( fp );
   
   if( nread != (size_t)(2*m_inputFeatDim) )
   {
      return FAIL;
   }
   
   return OK;
}


//--------------------------------

int CTIesrFlex::read_qs(char *fname)
{
   FILE  *fp;
   int   size;
   
   // if question data exists, free it
   if( qs_base != NULL )
      free( qs_base );
   
   // open question file and read data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   qs_base = (char *) malloc( size*sizeof(char) );
   
   if( !qs_base )
   {
      fclose( fp );
      return FAIL;
   }
   
   fread(qs_base, sizeof(char), size, fp);
   fclose( fp );
   
   return OK;
}


//---------------------
int CTIesrFlex::read_qstbl(char *fname)
{
   FILE  *fp;
   int   size;
   int idx;
   
   // if question table data exists, free it
   if( qs_offset != NULL )
      free( qs_offset );
   
   // read question offsets
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   qs_offset = (int *) malloc( size*sizeof(int) );
   
   if( !qs_offset )
   {
      fclose( fp );
      return FAIL;
   }
   fread(qs_offset, sizeof(int), size, fp);
   fclose( fp );
   
   
//   for( idx = 0; idx < size; idx++ )
//   {
//      qs_offset[idx] += (int)qs_base;
//   }
   
   return OK;
}


//--------------------
int CTIesrFlex::read_tree(char *fname)
{
   FILE  *fp;
   int   size;
   
   // if tree data exists, free it
   if( tree_base != NULL )
      free( tree_base );
   
   // read tree data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   size /= sizeof(short);
   tree_base = (short *) malloc( size*sizeof(short) );
   
   if( !tree_base )
   {
      fclose(fp);
      return FAIL;
   }
   
   fread(tree_base, sizeof(short), size, fp);
   fclose( fp );
   
   return OK;
}


//----------------------
int CTIesrFlex::read_treetbl(char *fname)
{
   FILE  *fp;
   int   size;
   int   i, j, n_state;
   int   *ptr;
   int   numPhones;
   
   // if tree table data exists, free it
   if( treetbl_base != NULL )
      free( treetbl_base );
   
   // read tree data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   size /= sizeof(int);
   treetbl_base = (int *) malloc( size*sizeof(int) );
   
   if( !treetbl_base )
   {
      fclose(fp);
      return FAIL;
   }
   
   fread(treetbl_base, sizeof(int), size, fp);
   fclose( fp );
   
//   // adjust to memory addresses within the table from offsets to
//   // pointers to each tree.  Trees may be gender-dependent.
//   numPhones = m_bInputGDTrees ?  m_inputSets*n_phone  :  n_phone;
//
//   for( i = 0; i < numPhones; i++ )
//   {
//      // start of tree table contains address of locations of trees
//      treetbl_base[i] = (int)treetbl_base +
//      ( treetbl_base[i]*sizeof(int) );
//
//
//      // each state entry for each phone changed from
//      // offset in trees data to address of tree for each phone and state
//
//      // pointer to state count
//      ptr = (int *)treetbl_base[i];
//      n_state = *ptr;
//
//      // ptr changed to point to offset in tree_base for each state.
//      // Change offset to pointer to tree_base data for this phone and state.
//      ptr++;
//      for( j = 0; j < n_state; j++ )
//      {
//         ptr[j] = (int)tree_base + ( ptr[j]*sizeof(short) );
//      }
//   }
   
   return OK;
}


//-------------------
int CTIesrFlex::read_fxtran(char *fname)
{
   FILE  *fp;
   int   size;
   
   // if transition data exists, free it
   if( tran_base != NULL )
      free( tran_base );
   
   // read transition data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   tran_base = (short *) malloc( size*sizeof(short) );
   
   if( !tran_base )
   {
      fclose(fp);
      return FAIL;
   }
   
   fread(tran_base, sizeof(short), size, fp);
   fclose( fp );
   
   return OK;
}


//------------------------
int CTIesrFlex::read_fxtroff(char *fname)
{
   FILE  *fp;
   int   size;
   
   // if transition offset data exists, free it
   if( tranoffset_base != NULL )
   {
      free( tranoffset_base );
      m_numTrans = 0;
   }
   
   if( tran_offset )
      free( tran_offset );
   
   if( tran_idx_aug )
      free( tran_idx_aug );
   
   
   
   // read transition offset data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   tranoffset_base = (int *) malloc( size*sizeof(int) );
   
   if( !tranoffset_base )
   {
      fclose(fp);
      return FAIL;
   }
   
   fread(tranoffset_base, sizeof(int), size, fp);
   fclose( fp );
   
   m_numTrans = size;
   
   // Determine the maximum number of emitting states in the input hmms
   m_maxStates = 0;
   for (int tran = 0; tran < m_numTrans; tran++ )
   {
      int offset = tranoffset_base[tran];
      int nStates = (int)( tran_base[offset] - 1);
      
      if( nStates > m_maxStates )
         m_maxStates = nStates;
   }
   
   
   // tran_offset will have one extra entry at the end
   tran_offset = (int*)malloc( (m_numTrans + 1) * sizeof(int) );
   tran_idx_aug = (int*)malloc( size * sizeof(int) );
   if( !tran_offset || !tran_idx_aug )
   {
      return FAIL;
   }
   
   
   return OK;
}

//--------------------------
int CTIesrFlex::read_fxclust(char *fname)
{
   FILE  *fp;
   int   size;
   
   // if cluster data exists, free it
   if( cluster_base != NULL )
   {
      free( cluster_base );
   }
   
   
   // read cluster (pdf) data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   cluster_base = (short *) malloc( size*sizeof(short) );
   
   if( !cluster_base )
   {
      fclose(fp);
      return FAIL;
   }
   
   fread(cluster_base, sizeof(short), size, fp);
   fclose( fp );
   
   return OK;
}

//--------------------------
//
// modified to generate the offset indices from the recursive difference, 
//
int CTIesrFlex::read_fxcloff(char *fname)
{
   FILE  *fp = NULL;
   int   size;
   int   nClusters;
   int   offset_idx =0 ;
   int n_mix, offset;
   size_t readFlag;
   int fx_offset_increment  = 1;
   int fx_clust_entries = 3;
   int status = FAIL;
   
   //switch case added to select the configureation at runtime bu reading the fxconfig file
   
   switch(m_bInputWeightConfig)
   {
      case 0:
         fx_clust_entries = 3;
         fx_offset_increment = 1;
         break;
         
      case 1:
         fx_offset_increment = 2;
         fx_clust_entries = 2;
         break;
         
      case 2:
         fx_offset_increment=1;
         fx_clust_entries = 2;
         break;
   }
   
   // if cluster offset data exists, free it
   if( clusteroffset_base != NULL )
   {
      free( clusteroffset_base );
      m_numClusters = 0;
   }
   
   if( cluster_offset )
      free( cluster_offset );
   
   if( cluster_idx_aug )
      free( cluster_idx_aug );
   
   
   // read cluster offset data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      goto EXITREAD;
   }
   
   // Read number of offsets, which is also the number of clusters
   readFlag = fread(&size, sizeof(int), 1, fp);
   if(readFlag != 1)
   {
      goto EXITREAD;
   }
   
   clusteroffset_base = (int *) malloc( size*sizeof(int) );
   if( !clusteroffset_base )
   {
      goto EXITREAD;
   }
   
   
   // Check if number of mix and weight index packed in single short in fxclust file.
   // Build the offset array dynamically (used to be read in from fxclust.off).
   if(m_bPackWtNmix == 1)
   {
      n_mix = (int) (0xff & cluster_base[0]);
      clusteroffset_base[0] = 0;
      for(offset_idx=1;offset_idx<size;offset_idx++)
      {
         offset = 1 +  n_mix*2;
         clusteroffset_base[offset_idx] = (int)(clusteroffset_base[offset_idx-1] + offset);
         n_mix = (int)(0xff & cluster_base[clusteroffset_base[offset_idx]]);
      }
   }
   else
   {
      // the data is read in through in short as compared to int
      // nClusters = fread(clusteroffset_base, sizeof(short), size, fp);
      // fclose( fp );
      // we can get rid of the fxclust.off file by generating the offset file in situ
      //post processing to generate indices from the fxclust.cb data
      //the change in offsets is done so tha offsets need not be stored
      // the difference is offsets are then added recursively to generate the offset back
      n_mix = (int) cluster_base[0];
      clusteroffset_base[0] = 0;
      for(offset_idx=1;offset_idx<size;offset_idx++)
      {
         offset = fx_offset_increment +  n_mix*fx_clust_entries;
         clusteroffset_base[offset_idx] = clusteroffset_base[offset_idx-1] + offset;
         n_mix = (int)cluster_base[clusteroffset_base[offset_idx]];
      }
   }
   
   nClusters = size ;
   
   //   if( nClusters != size )
   // {
   //  return FAIL;
   // }
   
   m_numClusters = nClusters;
   
   
   // Allocate the dynamic arrays used during model output
   cluster_offset = (int*)malloc( (nClusters+1) * sizeof(int) );
   cluster_idx_aug = (int*)malloc( nClusters * sizeof(int) );
   if( !cluster_offset || !cluster_idx_aug )
   {
      goto EXITREAD;
   }
   
   // All processing and allocations succeeded
   status = OK;
   
   EXITREAD:
      if( fp )
         fclose(fp);
      
      return status;
}


//-----------------------
int CTIesrFlex::read_mean(char *fname)
{
   FILE  *fp;
   unsigned int   nMean;
   size_t elementSize;
   int vectorSize;
   
   // Free any exisng  mean data
   if( mean_base != NULL )
   {
      free( mean_base );
      m_numMeans = 0;
   }
   
   if( mean_idx_aug )
      free( mean_idx_aug );
   
   
   // Determine sizes of input and output vectors
   elementSize = m_bInputByteMeans ? sizeof( unsigned char ) :
      sizeof( short );
      
      vectorSize = elementSize * m_inputFeatDim;
      
      // Open the mean file and get number of mean vectors
      fp = fopen(fname, "rb");
      if ( fp == NULL )
      {
         return FAIL;
      }
      
      fread(&nMean, sizeof(unsigned int), 1, fp);
      
      
      // Allocate space to hold the mean vectors
      mean_base = malloc( nMean * vectorSize );
      if( !mean_base )
      {
         fclose(fp);
         return FAIL;
      }
      
      
      // Read in the vector data, stepping over unused dimensions
      size_t nRead = fread( mean_base, vectorSize, nMean, fp );
      
      fclose( fp );
      
      if( nRead != nMean )
         return FAIL;
      
      m_numMeans = nMean;
      
      // Allocate space to track usage of mean vectors
      mean_idx = (int*)malloc( nMean *sizeof(int) );
      mean_idx_aug = (int*)malloc( nMean * sizeof(int) );
      if( !mean_idx || !mean_idx_aug )
      {
         return FAIL;
      }
      
      return OK;
}

int CTIesrFlex::read_weight(char *fname)
{
   FILE *fp;
   unsigned int nWts;
   size_t elementSize;
   size_t readFlag;
   
   
   m_bInputByteWgts = false;
   
   //if weight data exists, free it
   if( weight_base != NULL )
   {
      free(weight_base);
      m_numWeights = 0;
   }
   
   //find if the weights are written in byte or short
   elementSize = m_bInputByteWgts ? sizeof( unsigned char ) :
      sizeof( short );
      
      fp = fopen(fname, "rb");
      if ( fp == NULL )
      {
         return FAIL;
      }
      //find the total number of weight vectors
      readFlag = fread(&nWts, sizeof(unsigned int), 1, fp);
      if(readFlag != 1)
      {
         return FAIL;
         
      }
      
      //read in the dimension of the weight Vectors
      fread(&m_lengthWtVec, sizeof(short), 1, fp);
      
      
      //allocate space to weights
      weight_base =(short *) malloc( nWts * elementSize );
      
      if( !weight_base )
      {
         fclose(fp);
         return FAIL;
      }
      size_t nRead = fread( weight_base, elementSize, nWts, fp );
      fclose( fp );
      
      if( nRead != nWts )
         return FAIL;
      
      return OK;
      
}
//---------------------------
int CTIesrFlex::read_var(char *fname)
{
   FILE  *fp;
   unsigned int   nVar;
   size_t elementSize;
   int vectorSize;
   
   // if inverse variance data exists, free it
   if( var_base != NULL )
   {
      free( var_base );
      m_numVars = 0;
   }
   
   if( var_idx_aug )
   {
      free(var_idx_aug );
   }
   
   
   // Determine sizes of input and output vectors
   elementSize = m_bInputByteVars ? sizeof( unsigned char ) :
      sizeof( short );
      
      vectorSize = elementSize * m_inputFeatDim;
      
      // Open the mean file and get number of mean vectors
      fp = fopen(fname, "rb");
      if ( fp == NULL )
      {
         return FAIL;
      }
      
      fread(&nVar, sizeof(unsigned int), 1, fp);
      
      
      // Allocate space to hold the mean vectors
      var_base = malloc( nVar * vectorSize );
      if( !var_base )
      {
         fclose(fp);
         return FAIL;
      }
      
      
      // Read in the vector data, stepping over unused dimensions
      size_t nRead = fread( var_base, vectorSize, nVar, fp );
      
      fclose( fp );
      
      if( nRead != nVar )
         return FAIL;
      
      m_numVars = nVar;
      
      // Allocate space to track usage of variance vectors during model output
      // var_idx = (int*)malloc( nVar * sizeof(int) );
      var_idx_aug = (int*)malloc( nVar * sizeof(int) );
      if( !var_idx_aug )
         return FAIL;
      
      
      return OK;
}


//-----------------------------
int CTIesrFlex::read_gconst(char *fname)
{
   FILE  *fp;
   int   size;
   
   // if gconst data exists, free it
   if( gconst_base != NULL )
      free( gconst_base );
   
   // read gconst data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&size, sizeof(int), 1, fp);
   gconst_base = (short *) malloc( size*sizeof(short) );
   
   if( !gconst_base )
   {
      fclose(fp);
      return FAIL;
   }
   
   fread(gconst_base, sizeof(short), size, fp);
   fclose( fp );
   
   return OK;
}


//-------------------------------
int CTIesrFlex::read_sil(char *fname)
{
   FILE  *fp;
   int nRead;
   
   // if silence hmm data exists, free it
   if( sil_hmm != NULL )
      free( sil_hmm );
   
   // read silence hmm data
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   // Number of states in silence model
   nRead = fread(&sil_n_state, sizeof(short), 1, fp);
   if( nRead != 1 )
      return FAIL;
   
   sil_hmm = (short *) malloc( sil_n_state*sizeof(short) );
   if( !sil_hmm )
   {
      fclose(fp);
      return FAIL;
   }
   
   // Transition index and cluster data for the silence model
   nRead = fread(sil_hmm, sizeof(short), sil_n_state, fp);
   fclose( fp );
   
   if( nRead != sil_n_state )
      return FAIL;
   
   return OK;
}



//-----------------------------------------------
void CTIesrFlex::free_codebook()
{
   if( qs_base )
   {
      free( qs_base );
      qs_base = NULL;
   }
   
   if( qs_offset )
   {
      free( qs_offset );
      qs_offset = NULL;
   }
   
   
   if( tree_base )
   {
      free( tree_base );
      tree_base = NULL;
   }
   
   if( treetbl_base )
   {
      free( treetbl_base );
      treetbl_base = NULL;
   }
   
   if( tran_base )
   {
      free( tran_base );
      tran_base = NULL;
   }
   
   if( tranoffset_base )
   {
      free( tranoffset_base );
      tranoffset_base = NULL;
      m_numTrans = 0;
   }
   
   if( tran_offset )
   {
      free( tran_offset );
      tran_offset = NULL;
   }
   
   if( tran_idx_aug )
   {
      free(tran_idx_aug );
      tran_idx_aug = NULL;
   }
   
   if( cluster_base )
   {
      free( cluster_base );
      cluster_base = NULL;
   }
   
   if( clusteroffset_base )
   {
      free( clusteroffset_base );
      clusteroffset_base = NULL;
      m_numClusters = 0;
   }
   
   if( cluster_offset )
   {
      free( cluster_offset );
      cluster_offset = NULL;
   }
   
   //free the weight array if used
   if(weight_base)
   {
      free(weight_base);
      weight_base = NULL;
   }
   
   if( cluster_idx_aug )
   {
      free( cluster_idx_aug );
      cluster_idx_aug = NULL;
   }
   
   
   if( mean_base )
   {
      free( mean_base );
      mean_base = NULL;
      m_numMeans = 0;
   }
   
   if( mean_idx )
   {
      free( mean_idx );
      mean_idx = NULL;
   }
   
   if( mean_idx_aug )
   {
      free( mean_idx_aug );
      mean_idx_aug = NULL;
   }
   
   if( var_base )
   {
      free( var_base );
      var_base = NULL;
      m_numVars = 0;
   }
   
   if( var_idx_aug )
   {
      free( var_idx_aug );
      var_idx_aug = NULL;
   }
   
   if( gconst_base )
   {
      free( gconst_base );
      gconst_base = NULL;
   }
   
   if( scale_base )
   {
      free( scale_base );
      scale_base = NULL;
   }
   
   if( sil_hmm )
   {
      free( sil_hmm );
      sil_hmm = NULL;
   }
}


/*----------------------------------------------------------------
 process_optional

 Create a new empty node named "", which implments an optional node
 branch.  The argument to this function specifies the node that
 should be in parallel as the optional branch.
 ----------------------------------------------------------------*/
int CTIesrFlex::process_optional( int aNode )
{
   /* Need empty node for optional.  Paranext node is the node that
    is in parallel with this empty node.*/
   return create_node("", -1, aNode, 0);
}


/*----------------------------------------------------------------
 create_node

 Fill a node with the information for a node, and return the index to the node.  If the
 dynamic node array is full, allocate additional array space.
 ----------------------------------------------------------------*/
int CTIesrFlex::create_node(char *name, int aNextNode, int aParaNextNode, int flag)
{
   
   // Check if all nodes in the dynamic node array used.
   if( node_idx >= m_nodeMax )
   {
      // Allocate NODE_ALLOC more nodes in the dynamic node array.
      // This will throw an exception if the expansion fails.
      node = reinterpret_cast<Gnode*>
      ( ExpandArray( reinterpret_cast<void*>(node), sizeof(Gnode), NODE_ALLOC, &m_nodeMax ) );
   }
   
   
   // Fill the information for the new node.
   node[ node_idx ].name = name;
   node[ node_idx ].next = aNextNode;
   node[ node_idx ].paranext = aParaNextNode;
   node[ node_idx ].flag = flag;
   
   node_idx ++;
   
   // Now return the node index, not a pointer to the node
   // return &node[ node_idx - 1 ];
   return ( node_idx - 1 );
}

/*----------------------------------------------------------------
 attach_at_end_sub

 Attach node aNode2 to the end of all paths starting from node aNode1
 that have not been visited yet.
 ----------------------------------------------------------------------*/
void CTIesrFlex::attach_at_end_sub( int aNode1, int aNode2 )
{
   
   // If node is a null node, or node has been visited, then
   // do not need to continue further.
   if ( aNode1 == -1 || node[aNode1].flag ) return;
   
   /* Mark node as visited, so it won't be expanded again */
   node[aNode1].flag = 1;
   
   
   if ( node[aNode1].next == -1 )
   {
      // Found end of a next path, attach Node2 to end
      node[aNode1].next = aNode2;
   }
   else
   {
      // Continue working down the next path
      attach_at_end_sub( node[aNode1].next, aNode2 );
   }
   
   // Follow the next path of the paranext node of Node1 to add Node2
   attach_at_end_sub( node[aNode1].paranext, aNode2 );
}



/*----------------------------------------------------------------
 attach_at_end

 Attach node aNode2 as the next node to the end of all paths starting
 from node aNode1.
 ----------------------------------------------------------------*/
void CTIesrFlex::attach_at_end( int aNode1, int aNode2 )
{
   clear_visit_flag();
   
   attach_at_end_sub( aNode1, aNode2 );
   
}


/*----------------------------------------------------------------
 attach_at_para_end_tree

 Function to create a tree structured grammar.

 have uniq siblings
 @param n1 the starting node
 @param n_cmp the node to be uniqed

 ----------------------------------------------------------------*/
void CTIesrFlex::attach_at_para_end_tree( int an1Node, int an_cmpNode)
{
   short bFound = 0;
   
   while ( an1Node != -1 )
   {
      if( strcmp( node[an1Node].name, node[an_cmpNode].name ) == 0 )
      {
         /* treenization */
         attach_at_para_end( node[an1Node].next, node[an_cmpNode].next );
         node[an_cmpNode].next = -1;
         node[an_cmpNode].paranext = -1;
         node[an_cmpNode].flag = 1;
         bFound = 1;
         break;
      }
      
      if( node[an1Node].paranext != -1 )
         an1Node = node[an1Node].paranext;
      else
         break;
   }
   
   if (!bFound)
      node[an1Node].paranext = an_cmpNode;
}


/* ---------------------------------------------------------------------------
 attach_at_para_end

 Attach node aNode2 to the end of the parallel path starting at aNode1.
 parallel path
 ---------------------------------------------------------------------- */
void CTIesrFlex::attach_at_para_end( int aNode1, int aNode2 )
{
   // Find the end of the paranext path for aNode1
   while ( node[aNode1].paranext != -1 )
      aNode1 = node[aNode1].paranext;
   
   // Add aNode2 at the end of the paranext path
   node[aNode1].paranext = aNode2;
}


/* ---------------------------------------------------------------------------
 clear_visit_flag

 Clear all existing node flags.  These are used to keep track of
 nodes visited while recursing down the tree of nodes.
 ---------------------------------------------------------------------- */
void CTIesrFlex::clear_visit_flag()
{
   int   i;
   
   for ( i = 0; i < m_nodeMax; i++) node[i].flag = 0;
}


/*----------------------------------------------------------------
 ClearNodeNetFlags

 This function clears only the nodes corresponding to a node network.
 The input argument is the top node of the network.  All nodes under
 the top node will be cleared.  This function may result in node
 flags being cleared more than once, since the flags obviously can
 not be used to determine if a node has already been visited.

 ----------------------------------------------------------------*/
void CTIesrFlex::ClearNodeNetFlags( int aNode )
{
   if( aNode == -1 )
      return;
   
   // Breadth first search to clear network node flags
   ClearNodeNetFlags( node[aNode].paranext );
   
   // Then depth first search to clear node flags
   ClearNodeNetFlags( node[aNode].next );
   
   node[aNode].flag = 0;
   
   return;
}



/* ---------------------------------------------------------------------------
 clear duplicate flag
 ---------------------------------------------------------------------- */
void CTIesrFlex::clear_sentnode_visit_flag()
{
   int   i;
   
   for ( i = 0; i < m_sentnodeMax; i++) sentnode[i].flag = 0;
   
}


/* ---------------------------------------------------------------------------
 duplicate_node_net_copy

 Make a duplicate of a node-network so that it can be inserted in place
 of a non-terminal reference in another rule.
 ---------------------------------------------------------------------- */
int CTIesrFlex::duplicate_node_net_copy( int aNode )
{
   int n_new, r_node;
   
   // If invalid (NULL) node, then just return
   if ( aNode == -1 ) return -1;
   
   
   // If node not flagged, duplicate the node
   if ( node[aNode].flag == 0 )
   {
      
      n_new = create_node(node[aNode].name, -1, -1, 0);
      node[aNode].flag = n_new;
      
   } else
   {
      /* Node has been duplicated, stop, avoid duplicate */
      return( node[aNode].flag );
   }
   
   /* Follow the network, to duplicate breadth first */
   
   /* Node pointer may change by adding new nodes in recursive calls, 
   so must ensure that return location of node index is valid.  This
   appears important for linux compiler. */
   // node[n_new].paranext = duplicate_node_net_copy( node[aNode].paranext );
   r_node = duplicate_node_net_copy( node[aNode].paranext );
   node[n_new].paranext = r_node;
   
   
   /* The follow the network to duplicate by depth */

   /* Node pointer may change by adding new nodes in recursive calls, 
   so must ensure that return location of node index is valid. This 
   appears important for linux compiler. */
   // node[n_new].next = duplicate_node_net_copy( node[aNode].next );
   r_node = duplicate_node_net_copy( node[aNode].next );
   node[n_new].next = r_node;
   
   
   return( n_new );
}


/* ---------------------------------------------------------------------------
 duplicate a node-network
 ---------------------------------------------------------------------- */
int CTIesrFlex::duplicate_node_net(int aNode )
{
   /* use flag this time to keep track of the duplicate node of a node */
   // clear_visit_flag();
   
   // Clear only the visit flags for this node network corresponding to
   // a rule.  Do not clear all visit flags, because they are being used
   // to determine whether a node has already been tested to see if the
   // node should undergo rule node network substitution.
   ClearNodeNetFlags( aNode );
   
   return( duplicate_node_net_copy( aNode ) );
}


/* ---------------------------------------------------------------------------
 create rule
 ---------------------------------------------------------------------- */
int CTIesrFlex::create_rule(char *nonterm, int aNode )
{
   
   // Now we are allocating rule array dynamically
   // demand( rule_idx < MAX_RULE, "Error: Exceed MAX_RULE");
   
   // Check if an unused entry is available in the dynamic rule array.
   if( rule_idx >= m_ruleMax )
   {
      // Allocate RULE_ALLOC more entries.  This will throw an exception if array expansion fails.
      rule = reinterpret_cast<Grule*>
      ( ExpandArray( reinterpret_cast<void*>(rule), sizeof(Grule), RULE_ALLOC, &m_ruleMax ) );
   }
   
   
   // Fill the next rule with the non-terminal rule name and starting node of the rule.
   rule[ rule_idx ].name = nonterm;
   rule[ rule_idx ].node = aNode;
   
   rule_idx++;
   
   
   #ifdef TIESRFLEXDEBUG
   /* debug: print all possible expansions for diagnostic */
   /*
    char buf[ MAX_STR ];
    printf("RULE( %s ):\n", nonterm);
   
    buf[0] = '\0';
   
    expand(aNode, buf);
    */
   #endif
   
   
   /* return rule pointer */
   
   // Now return rule index, not pointer to the rule
   return( rule_idx - 1 );
}


/* ---------------------------------------------------------------------------
 replace_old_with_new

 Replace occurrences of an old node index in the dynamic node array
 next and paranext fields with a new index.  Also replace
 occurrences of the old node index in all rules with the new node
 index.
 ---------------------------------------------------------------------- */
void CTIesrFlex::replace_old_with_new( int aOldNode, int aNewNode )
{
   int   i;
   
   for (i = 0; i < node_idx; i++)
   {
      if ( node[i].next == aOldNode ) node[i].next = aNewNode;
      if ( node[i].paranext == aOldNode ) node[i].paranext = aNewNode;
   }
   
   for (i = 0; i < rule_idx; i++)
   {
      if ( rule[i].node == aOldNode ) rule[i].node = aNewNode;
   }
}


/*---------------------------------------------------------------------------
 referencing

 Determine if the rule given by the input rule index argument is
 referencing any other rule.  If not, then this rule is made up only of
 terminals.
 ---------------------------------------------------------------------- */
int CTIesrFlex::referencing(int idx)
{
   int   i;
   
   for ( i = 0; i < depend_idx; i++ )
   {
      if ( depend[i].referencing == idx )
         return 1;
   }
   
   return 0;
   
}


/* ---------------------------------------------------------------------------
 referenced_sub

 Determine if a name is referenced in a node network or if the node network
 has already been checked as indicated by the node flag.
 ---------------------------------------------------------------------- */
int CTIesrFlex::referenced_sub(char *name, int aNode )
{
   
   // If the node is invalid (NULL) or the node network has
   // already been checked, return
   if ( aNode == -1 || node[aNode].flag )
      return 0;
   
   /* visited, don't expand it again */
   node[aNode].flag = 1;
   
   
   // If node name matches, the name is referenced and function can return
   // immediately without any further checking
   if ( strcmp( node[aNode].name, name ) == 0 )
      return 1;
   
   // Follow all next and paranext nodes down the network
   else if ( referenced_sub(name, node[aNode].paranext ) ||
   referenced_sub(name, node[aNode].next) )
      return 1;
   
   // No reference found
   else return 0;
   
}


/*----------------------------------------------------------------
 referenced

 Find if a name is referenced in a node network.
 ----------------------------------------------------------------*/
int CTIesrFlex::referenced(char *name, int aNode )
{
   clear_visit_flag();
   
   return referenced_sub( name, aNode );
}


/* ---------------------------------------------------------------------------
 substitute non-terminal
 ---------------------------------------------------------------------- */
int  CTIesrFlex::substitute_nonterm( int aNode, int aRule )
{
   int n_new, paranext, next;
   
   if ( aNode == -1 ) return -1;
   
   if ( node[aNode].flag ) return aNode;
   
   /* breadth first */
   
   paranext = substitute_nonterm( node[aNode].paranext, aRule );
   
   /* then depth */
   
   next = substitute_nonterm( node[aNode].next, aRule );
   
   /* test if it is the nonterm in rule */
   if ( strcmp( node[aNode].name, rule[aRule].name ) == 0 )
   {
      /* duplicate network for this context */
      n_new = duplicate_node_net( rule[aRule].node );
      
      /* connect with parent network */
      attach_at_end( n_new, next );
      
      attach_at_para_end( n_new, paranext );
      
      /* update all pointers from the old node to the new node */
      
      replace_old_with_new( aNode, n_new );
      
      node[n_new].flag = 1;
      
      return( n_new );
      
   }
   
   else
   {
      
      node[aNode].next = next;
      
      node[aNode].paranext = paranext;
      
      node[aNode].flag = 1;
      
      return( aNode );
   }
}


/* ---------------------------------------------------------------------------
 add to the sentnode list when not found
 ---------------------------------------------------------------------- */
void CTIesrFlex::add_sentnode(int aNode)
{
   int   i;
   
   /* If node already added, return */
   for (i = 0; i < sentnode_idx; i++)
   {
      if ( aNode == sentnode[ i ].node )
         return;
   }
   
   /* Add a sentnode corresponding to this node */
   // demand( sentnode_idx < MAX_SENTNODE, "Error: Exceed MAX_SENTNODE");
   
   // Check if all sentnodes in the dynamic array are in use.
   if( sentnode_idx >= m_sentnodeMax )
   {
      // Allocate SENTNODE_ALLOC more sentnodes in the dynamic sentnode array.
      // This will throw an exception if the expansion fails.
      sentnode = reinterpret_cast<Sentnode*>
      ( ExpandArray( reinterpret_cast<void*>(sentnode),
      sizeof(Sentnode), SENTNODE_ALLOC, &m_sentnodeMax ) );
   }
   
   sentnode[ sentnode_idx ].node = aNode;
   sentnode[ sentnode_idx ].stop = 0;
   sentnode[ sentnode_idx ].n_next = 0;
   sentnode[ sentnode_idx ].nextMax = 0;
   sentnode[ sentnode_idx ].next = NULL;
   
   sentnode_idx ++;
}


/* ---------------------------------------------------------------------------
 recursively traverse the network and add sentence node
 ---------------------------------------------------------------------- */
void CTIesrFlex::traverse_and_add_sentnode(int aNode )
{
   if ( aNode == -1 || node[aNode].flag == 1 )
      return;
   
   /* visited, don't expand again */
   node[aNode].flag = 1;
   
   /* add sentnode for sequential */
   add_sentnode( node[aNode].next );
   traverse_and_add_sentnode( node[aNode].next );
   
   /* no sentnode for parallel,
    because it is the same start sentnode and just alternative transition */
   traverse_and_add_sentnode( node[aNode].paranext );
}


/* ---------------------------------------------------------------------------
 create sentence node list
 ---------------------------------------------------------------------- */
void CTIesrFlex::create_sentnode_list(int aNode )
{
   /* start node, only one */
   add_sentnode( aNode );
   if ( aNode == -1 )
      /* accommodate NULL grammar */
      sentnode[ 0 ].stop = 1;
   else
      sentnode[ 0 ].stop = 0;
   
   /* end node, may have more others */
   add_sentnode( -1 );
   sentnode[ 1 ].stop = 1;
   
   clear_visit_flag();
   
   traverse_and_add_sentnode( aNode );
}


/* ---------------------------------------------------------------------------
 add to word list when not found
 ---------------------------------------------------------------------- */
void CTIesrFlex::add_word(const char *s)
{
   int   i;
   
   /* If word already added, return */
   for (i = 0; i < word_idx; i++)
   {
      if ( strcmp( word[i], s ) == 0 )
         return;
   }
   
   
   // demand( word_idx < MAX_WORD, "Error: Exceed MAX_WORD");
   
   // Check if all word elements in dynamic array are used
   if( word_idx >= m_wordMax )
   {
      // Allocate WORD_ALLOC more words in the dynamic word array.
      // This will throw an exception if the expansion fails.
      word = reinterpret_cast<Word_t*>
      ( ExpandArray( reinterpret_cast<void*>(word),
      sizeof(Word_t), WORD_ALLOC, &m_wordMax ) );
   }
   
   strcpy( word[ word_idx ], s );              /* not found, add */
   word_idx ++;
}


/* ---------------------------------------------------------------------------
 recursively create word list
 ---------------------------------------------------------------------- */
void CTIesrFlex::create_word_list( int aNode )
{
   if ( aNode == -1 || node[aNode].flag == 1 )
      return;
   
   /* visited, don't expand again */
   node[aNode].flag = 1;
   
   if ( node[aNode].name[0] != '\0' )
      add_word( node[aNode].name );
   
   create_word_list( node[aNode].next );
   create_word_list( node[aNode].paranext );
}


/* ---------------------------------------------------------------------------
 debug, parallel node can not be a sentence node
 ---------------------------------------------------------------------- */
void CTIesrFlex::not_in_sentnode_list( int aNode )
{
   int   i;
   
   for (i = 0; i < sentnode_idx; i++)
      if ( aNode == sentnode[ i ].node )
         demand(0, "Error: parallel node in sentence node list\n");
}



/*----------------------------------------------------------------
 check_parallel_node

 For debugging purposes, confirm that no paranode is a sentnode.
 ----------------------------------------------------------------*/
void CTIesrFlex::check_parallel_node( int aNode )
{
   if ( aNode == -1 || node[aNode].flag == 1 )
      return;
   
   /* visited, don't expand again */
   node[aNode].flag = 1;
   
   // Check that sentnodes do not reference para nodes
   if ( node[aNode].paranext != -1 )
      not_in_sentnode_list( node[aNode].paranext );
   
   check_parallel_node( node[aNode].next );
   check_parallel_node( node[aNode].paranext );
}


/* ---------------------------------------------------------------------------
 find sentence node index, 1 based
 ---------------------------------------------------------------------- */
int CTIesrFlex::find_sentnode_idx( int aNode )
{
   int   i;
   
   for (i = 0; i < sentnode_idx; i++)
   {
      if ( sentnode[i].node == aNode ) return( i + 1 );
   }
   
   demand(0, "Error: sentence node not found\n");
   
   return 1;
}


/* ---------------------------------------------------------------------------
 find word index, 1 based, 1 reserved for empty name
 ---------------------------------------------------------------------- */
int CTIesrFlex::find_word_idx(char *s)
{
   int   i;
   
   if ( *s == '\0' ) return 1;
   
   for (i = 0; i < word_idx; i++)
   {
      if ( strcmp( word[i], s ) == 0 ) return( i + 2 );
   }
   demand(0, "Error: word not found\n");
   return 1;
}


/* ---------------------------------------------------------------------------
 create one sentence level transition, word associated with transition
 ---------------------------------------------------------------------- */
void CTIesrFlex::create_tran(int src, int dst, int type, int next)
{
   
   
   // demand( senttran_idx < MAX_SENTTRAN,
   // "Error: Exceed max sentence transitions");
   
   // Check if all senttran structures in dynamic array are used
   if( senttran_idx >= m_senttranMax )
   {
      // Allocate SENTTRAN_ALLOC more structures in the dynamic senttran array.
      // This will throw an exception if the expansion fails.
      senttran =  reinterpret_cast<Senttran*>
      ( ExpandArray( reinterpret_cast<void*>(senttran),
      sizeof(Senttran), SENTTRAN_ALLOC, &m_senttranMax ) );
   }
   
   senttran[ senttran_idx ].src = src;
   senttran[ senttran_idx ].dst = dst;
   senttran[ senttran_idx ].type = type;
   senttran[ senttran_idx ].next = next;
   
   senttran_idx ++;
}


/* ---------------------------------------------------------------------------
 keep looking until seeing a non-empty node
 the src stays the same through out the recursion
 ---------------------------------------------------------------------- */
void CTIesrFlex::skip_empty_node(int src, int aNode )
{
   int   type, dst;
   
   /* can be a stop node */
   if ( aNode == -1 )
   {
      sentnode[ src - 1 ].stop = 1;
      return;
   }
   
   while ( aNode != -1 )
   {
      
      /* Check for empty (option) node */
      if ( ( type = find_word_idx( node[aNode].name ) ) == 1 )
      {
         skip_empty_node( src, node[aNode].next );
         
      }
      
      /* non-empty node */
      else
      {
         dst = find_sentnode_idx( node[aNode].next );
         
         create_tran(src, dst, type, 0);
      }
      
      /* next transition from the same src */
      
      aNode = node[aNode].paranext;
   }
}


/* ---------------------------------------------------------------------------
 create all transitions, but next is not set
 ---------------------------------------------------------------------- */
void CTIesrFlex::expand_all_trans( int aNode )
{
   int   src, dst, type;
   
   /* avoid duplicate */
   if ( aNode == -1 || node[aNode].flag == 1 )
      return;
   
   /* start sentnode */
   src = find_sentnode_idx( aNode );
   
   /* don't do it again */
   node[aNode].flag = 1;
   
   while ( aNode != -1 )
   {
      
      /* Check for empty (option) node */
      if ( ( type = find_word_idx( node[aNode].name ) ) == 1 )
      {
         skip_empty_node( src, node[aNode].next );
         
      }
      
      /* non-empty node */
      else
      {
         dst = find_sentnode_idx( node[aNode].next );
         create_tran(src, dst, type, 0);
      }
      
      /* recursive expand */
      expand_all_trans( node[aNode].next );
      
      /* next transition from the same src */
      aNode = node[aNode].paranext;
   }
}


/* ---------------------------------------------------------------------------
 set next in all trans
 ---------------------------------------------------------------------- */
void CTIesrFlex::connect_all_trans()
{
   int   i, j, tran;
   
   for (i = 0; i < sentnode_idx; i++)
   {
      
      tran = 0;
      
      for (j = 0; j < senttran_idx; j++)
      {
         
         /* src of tran is this sentnode */
         if ( senttran[ j ].src == i + 1 )
         {
            senttran[ j ].next = tran;
            
            /* link all the trans from this sentnode */
            tran = j + 1;
         }
         
         sentnode[ i ].tran = tran;   /* assign this list of trans to sentnode */
      }
   }
}


/* ---------------------------------------------------------------------------
 create all transitions
 ---------------------------------------------------------------------- */
void CTIesrFlex::create_all_trans( int aNode )
{
   clear_visit_flag();     /* avoid duplicate */
   
   expand_all_trans( aNode );  /* create all transitions, but ->next not set yet */
   
   connect_all_trans();    /* set senttran[].next */
   
}


#ifdef TIESRFLEXDEBUG

/* ---------------------------------------------------------------------------
 expand all paths using original structure,
 where word is associated with node
 ---------------------------------------------------------------------- */
void CTIesrFlex::expand(int aNode, char *buf)
{
   char   buf_local[ MAX_STR ];
   
   if ( aNode == -1 ) return;
   
   strcpy(buf_local, buf);
   
   /* breadth first */
   
   expand( node[aNode].paranext, buf_local );
   
   /* current word */
   
   strcat(buf_local, node[aNode].name);
   if ( node[aNode].name[0] != '\0' ) strcat(buf_local, " ");
   
   /* then depth */
   
   if ( node[aNode].next == -1 )
   {
      
      printf("%s\n", buf_local);
      
   } else
   {
      
      expand( node[aNode].next, buf_local );
   }
}


/* ---------------------------------------------------------------------------
 expand all paths using fsa2 structure,
 where word is associated with transition
 ---------------------------------------------------------------------- */
void CTIesrFlex::expand_fsa2( int aSentNode, char *buf)
{
   char   buf_local[ MAX_STR ];
   int    tran;
   
   //if ( n->stop ) printf("%s\n", buf);  /* end of path, print */
   
   tran = sentnode[aSentNode].tran;
   
   while ( tran )
   {
      
      strcpybuf_local, buf);
      strcat(buf_local, word[ senttran[ tran - 1 ].type - 2 ]);
      strcat(buf_local, " ");
      
      expand_fsa2( senttran[ tran - 1 ].dst - 1, buf_local );
      
      tran = senttran[ tran - 1 ].next;  /* next tran from this src node */
   }
}

#endif


/* ---------------------------------------------------------------------------
 create one sym in sym[]
 ---------------------------------------------------------------------- */

int CTIesrFlex::create_sym(int hmm_code, int word_code, int n_next, int *next)
{
   // Check if all symbols in array used
   if( sym_idx >= m_symMax )
   {
      // Allocate SYM_ALLOC more sym entries in the dynamic sym array.
      // This will throw an exception if the expansion fails.
      sym = reinterpret_cast<Symnode*>
      ( ExpandArray( reinterpret_cast<void*>(sym), sizeof(Symnode),
      SYM_ALLOC, &m_symMax ) );
   }
   
   sym[ sym_idx ].hmm = hmm_code;
   sym[ sym_idx ].word = word_code;
   sym[ sym_idx ].next = NULL;
   sym[ sym_idx ].n_next = 0;
   sym[ sym_idx ].nextMax = 0;
   
   // Add transition indices to symbol next list
   AddNext( &sym[sym_idx].next, &sym[sym_idx].n_next, &sym[sym_idx].nextMax,
   n_next, next );
   /*
    int   i;
   
    sym[ sym_idx ].n_next = n_next;
    for ( i = 0; i < n_next; i++ )
    sym[ sym_idx ].next[i] = next[i];
    */
   
   sym_idx ++;
   return( sym_idx - 1 );
   
}

/* ---------------------------------------------------------------------------
 build sym[]

 from an fsa2, where word are associated with transition;
 convert it to sym net, where sym is a phone

 each sym is followed by a list of syms (phone associated with sym)

 do phonetic pronunciation lookup

 insert optional silence between all the words (not between syms)

 silence's hmm_idx = 0, but there are more than one silence sym
 ---------------------------------------------------------------------- */

void CTIesrFlex::create_sym_net( int aSentNode )
{
   int    i;
   int    tran;
   char*  pron;
   int    c, l, r;
   int    hmm_code, word_code;
   int    dst;
   int    idx;
   
   //Errors error;
   
   /* If node already visited, return */
   if ( sentnode[aSentNode].flag )
      return;
   
   /* set visited */
   sentnode[aSentNode].flag = 1;
   
   /* init, should already be initialized by sentnode creation */
   /* check to see if this can be removed */
   sentnode[aSentNode].n_next = 0;
   sentnode[aSentNode].nextMax = 0;
   sentnode[aSentNode].next = NULL;
   
   /* first transition with aSentNode as the source */
   tran = sentnode[aSentNode].tran;
   
   /* loop thru all transitions */
   while ( tran )
   {
      
      /* depth first to build next array */
      
      /* Access requires conversion from
       one-based tran and sentnode indices to zero based */
      dst = senttran[ tran - 1 ].dst - 1;
      create_sym_net( dst );
      
      /* expand this word into sym (phone) */
      
      // Old way was to look up pronunciation again, even though already looked up
      // in create_hmms function call.
      // error = LookupPronunciation( word[ senttran[ tran - 1 ].type - 2 ], pron );
      // demand( error == ErrNone, "create_sym_net: lookup pronunciation failed\n" );
      
      // New way is to get stored pronunciation created during create_hmms call.
      // type is one based, and sil is first "word".
      pron = m_wordProns[ senttran[ tran - 1 ].type - 2 ];
      
      for (i = pron[0] - 1; i >= 0; i--)
      {
         
         c = pron[ i + 1 ];
         
         // If using monophones, or the word is "_SIL*" as indicated by
         // c = -1, then set dummy left and right context to -1,
         // otherwise, set left and right triphone context.
         if( m_bInputMonophone || c == -1 )
         {
            l = -1;
            r = -1;
         }
         else
         {
            l = i == 0 ? n_phone : pron[ i ];
            r = i == (pron[0] - 1) ? n_phone : pron[ i + 2 ];
         }
         
         // Handle _SIL* models code uniquely
         if( c == -1 )
            hmm_code = 0;
         else
            hmm_code = find_hmm_idx(c, l, r);
         
         
         // If this is the last phone of the word, then determine the
         // word code to output.  Now the "_SIL*" words will have a word
         // code so no checking is necessary for it.
         // if(  i == (pron[0] - 1) && c != -1 )
         if(  i == (pron[0] - 1) )
         {
            word_code = (senttran[ tran - 1 ].type - 2);
         }
         else
         {
            word_code = MCODE;
         }
         
         /* word end sym */
         if ( i == (pron[0] - 1) )
         {
            
            create_sym(hmm_code, word_code, sentnode[dst].n_next, sentnode[dst].next);
            
            /* stop at this word? */
            if ( sentnode[dst].stop )
            {
               idx = sym_idx - 1;
               
               AddNext( &stop_next, &n_stop_next, &m_stopMax, 1, &idx );
            }
         }
         
         else
         {
            /* not word end, followed by one sym */
            
            // create_sym(hmm_code, word_code, 0, NULL);
            // create_sym increments sym_idx at end of the function, so now
            // sym_idx-1 is the index created by the latest call to create_sym
            // sym[ sym_idx - 1 ].next[ 0 ] = sym_idx - 2;
            // sym[ sym_idx - 1 ].n_next = 1;
            
            int prior_sym = sym_idx - 1;
            create_sym( hmm_code, word_code, 1, &prior_sym );
         }
         
         
         /* next[] array  for the preceding sym in the sym_net */
         if ( i == 0 )
         {
            // Add first sym (hmm) for word to source sentnode list of next syms
            int begin_sym = sym_idx - 1;
            AddNext( &sentnode[aSentNode].next, &sentnode[aSentNode].n_next,
            &sentnode[aSentNode].nextMax, 1, &begin_sym );
         }
      }
      
      tran = senttran[ tran - 1 ].next;  /* next tran from this src node */
   }
   
   
   /* optional silence before this group of words */
   if( m_bAutoSilence )
   {
      // Create silence symbol that goes to all other word starting
      // symbols from this sentnode.  If m_bAutoSilence has been set,
      // then the "_SIL" word is the first word in the list.
      // create_sym(0, MCODE, sentnode[aSentNode].n_next, sentnode[aSentNode].next);
      create_sym(0, 0, sentnode[aSentNode].n_next, sentnode[aSentNode].next);
      
      /* Add silence self loop */
      idx = sym_idx - 1;
      AddNext(  &sym[ sym_idx - 1 ].next, &sym[sym_idx-1].n_next,
      &sym[ sym_idx - 1 ].nextMax, 1, &idx );
      
      /* Add this silence in parallel with this group of words as another
       symbol starting from this sentnode. */
      AddNext( &sentnode[aSentNode].next, &sentnode[aSentNode].n_next,
      &sentnode[aSentNode].nextMax, 1, &idx );
      
      /* If sentnode is a stop node, then the loop silence is a stop symbol */
      if ( sentnode[aSentNode].stop )
      {
         idx = sym_idx - 1;
         
         AddNext( &stop_next, &n_stop_next, &m_stopMax, 1, &idx );
      }
   }
}


/* ---------------------------------------------------------------------------
 is this an end sym
 ---------------------------------------------------------------------- */
int CTIesrFlex::is_end_sym(int sym_code)
{
   int   i;
   
   for ( i = 0; i < n_stop_next; i++ )
   {
      if ( stop_next[i] == sym_code ) return 1;
   }
   
   return 0;
   
}

#ifdef TIESRFLEXDEBUG
/* ---------------------------------------------------------------------------
 construct sym name, only used for debug by expand sym net
 requires access to phone list of dictionary, not supplied by the
 dictionary class at this time.
 ---------------------------------------------------------------------- */

void CTIesrFlex::construct_sym_name(char *buf, int sym_code, int phone_or_word)
{
   /*   int   hmm_code;
   
    hmm_code = sym[ sym_code ].hmm;
   
    if ( hmm_code == 0 ) {
   
    strcpy( buf, "sil" );
   
    } else {
   
    if ( phone_or_word == 1 ) {
   
    if ( sym[ sym_code ].word == MCODE ) {
    buf[0] = '\0';
    } else {
    strcpy( buf, word[ sym[ sym_code ].word ] );
    }
   
    } else if ( phone_or_word == 0 ) {
   
    strcpy( buf, phone[ hmm[ hmm_code ].c ] );
   
    } else {
   
    demand(0, "Error: must be 0 (phone) or 1 (word)\n");
    }
    }
    */
}


/* ---------------------------------------------------------------------------
 debug, expand sym net, not used at present time
 ---------------------------------------------------------------------- */

void CTIesrFlex::expand_sym_net(int n_next, int *next, char *buf, int phone_or_word)
{
   //   int   i;
   //   char  sym_name[ MAX_STR ];
   //   char  buf_local[ MAX_STR ];
   //
   //   for ( i = 0; i < n_next; i++ ) {         /* loop thru next[] */
   //
   //      strcpy( buf_local, buf );           /* reset */
   //      construct_sym_name( sym_name, next[i], phone_or_word );
   //      strcat( buf_local, " " );
   //      strcat( buf_local, sym_name );      /* append this sym str */
   //
   //      if ( is_end_sym( next[i] ) )           /* print if sentence end */
   //         printf("%s\n", buf_local );
   //
   //      expand_sym_net( sym[ next[i] ].n_next, /* depth expand */
   //         sym[ next[i] ].next,
   //         buf_local,
   //         phone_or_word );
   //   }
}

#endif

/* ---------------------------------------------------------------------------
 find hmm idx
 ---------------------------------------------------------------------- */
int CTIesrFlex::find_hmm_idx(int c, int l, int r)
{
   int   i;
   
   for ( i = 0; i < hmm_idx; i++ )
   {
      if ( hmm[i].c == c && hmm[i].l == l && hmm[i].r == r )
         return i;
   }
   
   /* should not happen */
   
   demand(0, "Error: triphone not in the list\n");
   return 1;
   
}


/* ---------------------------------------------------------------------------
 add to hmm[] if new
 ---------------------------------------------------------------------- */
void CTIesrFlex::add_hmm_entry(int c, int l, int r)
{
   int   i;
   
   /* Check if hmm is already in the hmm table */
   for ( i = 0; i < hmm_idx; i++ )
   {
      if ( hmm[i].c == c && hmm[i].l == l && hmm[i].r == r )
         return;
   }
   
   /* Add new HMM to HMM table */
   
   // Check to see if all hmm entries used
   if( hmm_idx >= m_hmmMax )
   {
      // Allocate HMM_ALLOC more hmm entries in the dynamic hmm array.
      // This will throw an exception if the expansion fails.
      hmm = reinterpret_cast<Hmmtbl*>
      ( ExpandArray( reinterpret_cast<void*>(hmm), sizeof(Hmmtbl),
      HMM_ALLOC, &m_hmmMax ) );
   }
   
   hmm[ hmm_idx ].c = c;
   hmm[ hmm_idx ].l = l;
   hmm[ hmm_idx ].r = r;
   
   hmm_idx++;
}


/* ---------------------------------------------------------------------------
 lookup pronunciation, build hmm[]
 ---------------------------------------------------------------------- */
void CTIesrFlex::create_hmm()
{
   int   j;
   int   c, l, r;           /* center, left, right */
   char  pron[ MAX_PRON ];
   
   int baseIndex;
   // char  pronstr[ MAX_STR ];
   Errors error;
   
   /* hmm[0] is sil */
   add_hmm_entry( -1, -1, -1 );
   
   // Flag indicating multipronunciations exist
   int doMulti = ( word_idx > m_baseWordIndex );
   
   // Word index for starting of multiple words, which come after the base words
   // in the word table
   int multiIndex = m_baseWordIndex;
   
   
   // Loop over all base words, getting their pronunciations and triphone hmms
   int silLen = strlen( SIL );
   for ( baseIndex = 0; baseIndex < m_baseWordIndex; baseIndex++ )
   {
      
      // Handle "_SIL*" words independently.  It is a synonym for the silence model.
      if( strncmp( word[baseIndex], SIL, silLen ) == 0 )
      {
         // The silence model is already in the hmm list as hmm[0]
         m_wordProns[baseIndex][0] = 1;
         m_wordProns[baseIndex][1] = -1;
         
         continue;
      }
      
      
      // Get base pronunciation, which may be the first dictionary pronunciation
      // or may be the default rule pronunciation
      //m_dict->GetPron(word[i], pron, pronstr );
      error = LookupBasePronunciation( word[baseIndex], pron );
      demand( error == ErrNone, "create_hmm: lookup pronunciation failed\n" );
      
      //  For OMAP application, do not allow printing of pronunciations
      /*  if ( print_pron ) {
       printf("%s\t", word[i]);
       for ( j = 0; j < pron[0]; j++ ) {
       printf("%s ", phone[ pron[ j + 1 ] ]);
       }
       printf("\n");
       }
       */
      
      
      // Store size of pronunciation of base word indexed by baseIndex
      m_wordProns[baseIndex][0] = pron[0];
      
      /* Add hmms making up the word to the hmm[] list */
      for ( j = 0; j < pron[0]; j++ )
      {
         
         c = pron[ j + 1 ];
         
         // If using monophones, set dummy left and right context to -1,
         // otherwise, set left and right triphone context.
         if( m_bInputMonophone )
         {
            l = -1;
            r = -1;
         }
         else
         {
            l = j == 0 ? n_phone : pron[ j ];
            r = j == (pron[0] - 1) ? n_phone : pron[ j + 2 ];
         }
         
         add_hmm_entry(c, l, r);
         
         // Store pronunciation of base word indexed by baseIndex
         m_wordProns[baseIndex][j+1] = c;
      }
      
      
      // If multiple pronunciations exist, and there are still more
      // to process, get any multipronunciations for the base word
      // that may have been put in the word table
      if( doMulti && multiIndex < word_idx )
      {
         
         // Loop over all multipronunciation words that match this word as long as any
         // multipronunciation words in the word table remain to be processed
         error = LookupMultiPronunciation( word[baseIndex], word[multiIndex], pron );
         
         // Check for failure
         demand( error != ErrFail, "create_hmm: lookup pronunciation failed\n" );
         
         
         while( multiIndex < word_idx && error != ErrNoMatch  )
         {
            // Store size of pronunciation of multi word indexed by multiIndex
            m_wordProns[multiIndex][0] = pron[0];
            
            
            /* add to the hmm[] list */
            for ( j = 0; j < pron[0]; j++ )
            {
               
               c = pron[ j + 1 ];
               
               
               // If using monophones, set dummy left and right context to -1,
               // otherwise, set left and right triphone context.
               if( m_bInputMonophone )
               {
                  l = -1;
                  r = -1;
               }
               else
               {
                  l = j == 0 ? n_phone : pron[ j ];
                  r = j == (pron[0] - 1) ? n_phone : pron[ j + 2 ];
               }
               
               add_hmm_entry(c, l, r);
               
               // Store pronunciation of multi word indexed by multiIndex
               m_wordProns[multiIndex][j+1] = c;
            }
            
            
            // Get next matching multipronunciation as long as any remain
            multiIndex++;
            if( multiIndex < word_idx )
               error = LookupMultiPronunciation( word[baseIndex], word[multiIndex], pron );
         }
      }
   }
}


/* ---------------------------------------------------------------------------
 create dependency list
 ---------------------------------------------------------------------- */
void CTIesrFlex::create_dependency_list()
{
   int   i, j;
   
   depend_idx = 0;
   
   for ( j = 0; j < rule_idx; j++)
   {
      
      for ( i = 0; i < rule_idx; i++ )
      {
         
         if ( referenced(rule[i].name, rule[j].node) )
         {
            
            demand( i != j, "Error: self referencing in grammar");
            
            
            // demand( depend_idx < MAX_DEPEND, "Error: Exceed MAX_DEPEND");
            if( depend_idx >= m_dependMax )
            {
               // Allocate DEPEND_ALLOC more structures in the dynamic depend array.
               // This will throw an exception if the expansion fails.
               depend =  reinterpret_cast<Depend*>
               ( ExpandArray( reinterpret_cast<void*>(depend),
               sizeof(Depend), DEPEND_ALLOC, &m_dependMax ) );
            }
            
            depend[ depend_idx ].referenced = i;
            depend[ depend_idx ].referencing = j;
            
            depend_idx++;
         }
      }
   }
}


/* ---------------------------------------------------------------------------
 if there are any rules unreferenced?
 ---------------------------------------------------------------------- */
void CTIesrFlex::check_unreferenced()
{
   int   i, j, cnt;
   
   for ( i = 1; i < rule_idx; i++ )
   {
      
      cnt = 0;
      for ( j = 0; j < depend_idx; j++ )
      {
         if ( i == depend[j].referenced ) cnt++;
      }
      
      // TODO: for OMAP generate some kind of warning
      //if ( cnt == 0 )
      /* printf("Error: rule %s not referenced\n", rule[i].name) */;
   }
}


/* ---------------------------------------------------------------------------
 cyclic definition in grammar?
 ---------------------------------------------------------------------- */
void CTIesrFlex::check_cyclic_grammar()
{
   int   i, cnt;
   int depend_idx_save;
   Depend *depend_save;
   
   
   // Allocate space to hold a temporary copy of the dependency list.
   depend_save = (Depend*)malloc( depend_idx * sizeof( Depend ) );
   if( !depend_save )
      throw "Memory fail";
   
   
   /* save dependency list */
   for ( i = 0; i < depend_idx; i++ )
   {
      depend_save[i].referenced = depend[i].referenced;
      depend_save[i].referencing = depend[i].referencing;
   }
   depend_idx_save = depend_idx;
   
   
   /* check cyclic definition in the grammar */
   while ( depend_idx > 0 )
   {
      
      cnt = 0;
      
      for ( i = 0; i < depend_idx; i++ )
      {
         
         if ( ! referencing( depend[i].referenced ) )
         {
            
            /* the referenced is not referencing others, substitute and erase */
            depend[i].referenced = -1;
            depend[i].referencing = -1;
            
            cnt++;
         }
      }
      
      // If all rules are referencing other rules, then a cycle exists somewhere.
      // Must deallocate the temporary storage space prior to throwing an
      // exception.
      if( !cnt )
      {
         free(depend_save);
      }
      
      demand( cnt, "Error: cyclic definition in the grammar");
      
      /* compact dependency list */
      
      cnt = 0;
      for ( i = 0; i < depend_idx; i++ )
      {
         if ( depend[i].referenced != -1 )
         {
            depend[cnt].referenced = depend[i].referenced;
            depend[cnt].referencing = depend[i].referencing;
            cnt++;
         }
      }
      
      depend_idx = cnt;
   }
   
   
   /* restore dependency list */
   for ( i = 0; i < depend_idx_save; i++ )
   {
      depend[i].referenced = depend_save[i].referenced;
      depend[i].referencing = depend_save[i].referencing;
   }
   depend_idx = depend_idx_save;
   
   free( depend_save );
}


/* ---------------------------------------------------------------------------
 first rule is the only start rule
 ---------------------------------------------------------------------- */

void CTIesrFlex::check_one_start_rule()
{
   int   i;
   
   demand( rule[0].name == NULL
   || strcmp( start_name, rule[0].name ) == 0,
   "Error: first rule is not the start symbol");
   
   for( i = 1; i < rule_idx; i++ )
      demand( strcmp( start_name, rule[i].name ),
      "Error: allow only one start rule");
   
}


/* ---------------------------------------------------------------------------
 process grammar
 ---------------------------------------------------------------------- */
void CTIesrFlex::process_grammar()
{
   
   /* --------------------------------------------------
    now we have read all the rules,
    assume the first rule is the grammar,
    other rules are nonterminal expansion
    -------------------------------------------------- */
   
   check_one_start_rule();
   
   /* --------------------------------------------------
    create dependency list
    -------------------------------------------------- */
   
   create_dependency_list();
   
   /* --------------------------------------------------
    check for unreferenced rule
    -------------------------------------------------- */
   
   check_unreferenced();
   
   /* --------------------------------------------------
    check cyclic definition in the grammar
    -------------------------------------------------- */
   
   check_cyclic_grammar();
   
   /* --------------------------------------------------
    expand all non-terminals in the start rule
    -------------------------------------------------- */
   
   // While dependencies exist, and the start rule is still referencing
   // some other rule
   while ( depend_idx && depend[0].referencing == 0 )
   {
      
      /* expand the referenced non-terminal symbol in the start rule */
      
      clear_visit_flag();
      
      rule[ 0 ].node =
      substitute_nonterm( rule[ 0 ].node, depend[ 0 ].referenced );
      
      /* create dependency list again */
      
      create_dependency_list();
   }
   
   /* --------------------------------------------------
    debug: print all possible expansions for diagnostic
    Check if the node[] structure is correct
    -------------------------------------------------- */
   /*
    char  buf[ MAX_STR ];
    buf[0] = '\0';
   
    expand( rule[0].node, buf);
    */
   
   /* --------------------------------------------------
    turn it into fsa2, where words are associated with transition
    -------------------------------------------------- */
   
   create_sentnode_list( rule[0].node );
   
   /* printf("Sentence node count: %d\n", sentnode_idx); */
   
   clear_visit_flag();
   check_parallel_node( rule[0].node );   /* debug */
   
   clear_visit_flag();
   
   // If silence will automatically be added between words, then
   // add the SIL word to the word list, as the first word in the list.
   if( m_bAutoSilence)
   {
      add_word( SIL );
   }
   
   create_word_list( rule[0].node );
   
   /* printf("Word count: %d\n", word_idx); */
   
   create_all_trans( rule[0].node );
   
   /* printf("Sent Tran count: %d\n", senttran_idx); */
   
   /* --------------------------------------------------
    debug: print all possible expansions for diagnostic
    Check if the fsa2 (sentnode[], senttran[]) is correct?
    -------------------------------------------------- */
   /*
    buf[0] = '\0';
   
    expand_fsa2( &sentnode[ 0 ], buf );
    */
   
}


//----------------------------------------------
// Placeholder function for now - not operational
int CTIesrFlex::download_net()
{
   /* --------------------------------------------------
    TODO: output wordlist
    -------------------------------------------------- */
   
   // TODO: tell DSP word info is complete
   
   /* --------------------------------------------------
    TODO: output net
    -------------------------------------------------- */
   
   //TODO: Tell DSP .net info is complete
   
   return OK;
}

//------------------------------------
// Placeholder function for now - not operational
int CTIesrFlex::download_hmm2phone()
{
   
   // TODO: download hmm to monophone index mapping
   // TODO: Tell DSP hmm to monphone index info is complete
   
   return OK;
   
}


//---------------------------------------------
// Placeholder function for now - not operational
int CTIesrFlex::download_gtm()
{
   // TODO: download gtm structures to memory
   // TODO: Tell DSP gtm location is loaded
   
   return OK;
   
}


//-----------------------------------------
int CTIesrFlex::output_int16(FILE *fp, int i32)
{
   // output a 16-bit, big endian or little endian to a file,
   // checking for overflow
   
   short i16;
   int failed;
   
   failed = 0;
   
   if ( ( i32 != MCODE ) &&                 /* only exception */
   ( i32 < -32768 || i32 > 65535 ) )
   {
      return -1;
   }
   
   i16 = i32;
   
   if ( little_endian )
   {
      failed |= ( fputc( 0xff & i16, fp) == EOF );
      i16 = i16 >> 8;
      failed |= ( fputc( 0xff & i16, fp) == EOF );
   }
   else
   {
      failed |= ( fputc( ( i16 >> 8 ) & 0xff, fp ) == EOF );
      failed |= ( fputc( i16 & 0xff, fp ) == EOF );
   }
   
   return failed;
}


//------------------------------------------------
int CTIesrFlex::output_sym( FILE *fp, int hmm_code, int word_code, int n_next, int *next)
{
   // output one symbol
   int   i;
   
   // track if output_sym fails
   int failed = false;
   
   failed |= output_int16( fp, hmm_code );
   
   i = word_code == MCODE ? n_next : n_next | 0x8000;
   failed |= output_int16( fp, i );
   
   for (i = 0 ; i < n_next; i++)
   {
      failed |= output_int16( fp, next[i] );
   }
   
   if ( word_code != MCODE )
   {
      failed |= output_int16( fp, word_code );
   }
   
   return failed;
}


//-------------------------------------
int CTIesrFlex::output_net(char *dirname, char *filename)
{
   // This function outputs the wordlist and the grammar network
   
   int   i;
   short cnt = 0;
   FILE  *fp;
   char fname[FILENAME_MAX];
   
   // track any failure to write correctly
   int failed =  false;
   
   
   //output wordlist
   strcpy( fname, dirname );
   strcat( fname, "/word.lis" );
   
   fp = fopen( fname, "w");
   if ( fp == NULL )
      return -1;
   
   for ( i = 0; i < word_idx; i++ )
   {
      /* loop thru all words */
      failed |= ( fprintf( fp, "%s\n", word[i] ) < 0 );
   }
   
   fclose( fp );
   
   if( failed )
      return failed;
   
   
   // output net
   strcpy( fname, dirname );
   //   strcat( fname, "/net.bin" );
   strcat( fname, filename);
   
   fp = fopen( fname, "wb" );
   if ( fp == NULL )
      return -1;
   
   
   failed |= output_int16( fp, cnt );       // # of short, dummy, fill at end
   failed |= output_int16(fp, hmm_idx * m_inputSets); // n_hmm for all genders
   failed |= output_int16(fp, sym_idx);     // n_sym
   failed |= output_int16(fp, word_idx);    // n_word
   failed |= output_int16(fp, m_inputSets);  // n_set
   failed |= output_int16(fp, hmm_idx);     // n_hmm_set
   
   cnt = 5 +                             /* 5 n_* */
   2 +                                 /* offset pointer to start and stop */
   sym_idx;                            /* offset pointer to all sym */
   
   failed |= output_int16( fp, cnt );     /* start offset */
   
   cnt += 2 + sentnode[ 0 ].n_next;
   
   failed |= output_int16( fp, cnt );     // stop offset
   
   cnt += 2 + n_stop_next;
   
   for (i = 0; i < sym_idx; i++)
   {
      
      failed |= output_int16( fp, cnt );            /* n_sym offset */
      
      cnt += sym[i].word == MCODE ? 2 + sym[i].n_next : 3 + sym[i].n_next;
   }
   
   failed |= output_sym( fp, 0, MCODE,
   sentnode[ 0 ].n_next, sentnode[ 0 ].next ); /* start */
   
   failed |= output_sym( fp, 0, MCODE,
   n_stop_next, stop_next );                   /* stop */
   
   for (i = 0; i < sym_idx; i++)
   {                         /* all */
      failed |= output_sym( fp, sym[i].hmm, sym[i].word, sym[i].n_next, sym[i].next );
   }
   
   rewind( fp );
   failed |= output_int16( fp, cnt );              /* # of short */
   
   //net_size = cnt;
   
   fclose( fp );
   
   return failed;
   
}


//---------------------------------------------
int CTIesrFlex::output_hmm2phone(char *dirname )
{
   // output HMM to phone map
   
   FILE  *fp;
   int   i;
   char fname[FILENAME_MAX];
   
   // track any failure to write correctly
   int failed = false;
   
   strcpy( fname, dirname );
   strcat( fname, "/hmm2phone.bin" );
   
   fp = fopen( fname, "wb");
   if ( fp == NULL )
      return -1;
   
   for ( i = 0; i < hmm_idx; i++)
      failed |= output_int16( fp, hmm[ i ].c );
   
   fclose( fp );
   
   return failed;
}


/*----------------------------------------------------------------
 output_gtm

 This function outputs all files needed by the TIesrSI Engine in order
 to recognize a grammar.  Files are output to the directory specified as
 the input argument.

 This code has been updated to handle an arbitrary number of HMM
 sets, usually used to separate between male and female HMM
 sets. Note that all HMM sets will be identical in number
 and triphone definition since the grammar network is always
 set-independent.  The code has also been updated to include the
 possibility of either set-independent or set-dependent transition
 matrices.

 ----------------------------------------------------------------*/
int CTIesrFlex:: output_gtm( char *dirname )
{
   // Output all of the generalized tied HMM model information for this grammar
   
   int   i, j, offset, n_state;
   short *pshort;
   char  buf[ FILENAME_MAX ];
   int tempidx;
   
   short *cluster = NULL;
   int* hmm_offset = NULL;
   
   // zero-based count of output HMMs
   int ihmm;
   
   // Index to track hmm sets
   int hmmSet;
   
   // Transition set offset
   int tranSetOffset;
   
   FILE* fp_scale;
   int scaleIndex;
   
   // track failure to write gtm files
   int failed = false;
   
   
   // Initialize output array data indices
   mean_idx_cnt = 0;
   
   var_idx_cnt = 0;
   
   cluster_idx_cnt = 0;
   cluster_offset[0] = 0;
   
   tran_idx_cnt = 0;
   tran_offset[0] = 0;
   
   
   // Initialize index usage cross-reference arrays.  This eliminates the need to do a
   // linear search to determine if a codebook element has already been output to the
   // model binary files.
   for (int i=0; i< m_numMeans; i++)
   {
      mean_idx_aug[i]=-1;
   }
   
   for (int i=0; i< m_numClusters; i++)
   {
      cluster_idx_aug[i]=-1;
   }
   
   for (int i=0; i< m_numTrans; i++)
   {
      tran_idx_aug[i]=-1;
   }
   
   for (int i=0; i< m_numVars; i++)
   {
      var_idx_aug[i]=-1;
   }
   
   // Dynamic allocation of hmm_offset array.  This data is local
   // to this function, so does not need to be a class variable
   // There will be one more offset than number of models output.  The last
   // offset contains the total size of output data in int.
   hmm_offset = (int*)malloc( (hmm_idx * m_inputSets + 1) * sizeof(int) );
   if( !hmm_offset )
   {
      failed = 1;
      goto failure;
   }
   
   // First hmm offset is after pointers to hmm data offsets
   hmm_offset[0] = hmm_idx * m_inputSets;
   
   // Dynamic allocation of cluster array
   cluster = (short*)malloc( m_maxStates * sizeof(short) );
   if( !cluster )
   {
      failed = 1;
      goto failure;
   }
   
   
   /* --------------------------------------------------
    open GTM files
    -------------------------------------------------- */
   
   strcpy( buf, dirname);
   strcat( buf, "/hmm.bin" );
   fp_hmm = fopen( buf, "wb" );
   failed = ( fp_hmm == NULL );
   failed |= output_int16( fp_hmm, 0 );        /* # of short, dummy */
   
   
   /* Was for male & female.  Now for each HMM set. */
   for (i = 0; i < hmm_idx * m_inputSets; i++)
   {
      // Will eventually be offset in hmm.bin to each hmm's info
      failed |= output_int16( fp_hmm, 0 );     /* offset, dummy */
   }
   if( failed ) goto failure;
   
   
   strcpy( buf, dirname );
   strcat( buf, "/mu.bin" );
   fp_mean = fopen( buf, "wb" );
   failed |= ( fp_mean == NULL );
   failed |= output_int16( fp_mean, mean_idx_cnt );      /* dummy */
   if( failed ) goto failure;
   
   
   strcpy( buf, dirname );
   strcat( buf, "/var.bin" );
   fp_var = fopen( buf, "wb" );
   failed |= ( fp_var == NULL );
   failed |= output_int16( fp_var, var_idx_cnt );        /* dummy */
   if( failed ) goto failure;
   
   
   strcpy( buf, dirname );
   strcat( buf, "/tran.bin" );
   fp_tran = fopen( buf, "wb" );
   failed |= ( fp_tran == NULL );
   failed |= output_int16( fp_tran, 0 );                 /* dummy */
   if( failed ) goto failure;
   
   strcpy( buf, dirname );
   strcat( buf, "/pdf.bin" );
   fp_pdf = fopen( buf, "wb" );
   failed |= ( fp_pdf == NULL );
   if( failed ) goto failure;
   
   strcpy( buf, dirname );
   strcat( buf, "/mixture.bin" );
   fp_mixture = fopen( buf, "wb" );
   failed |= ( fp_mixture == NULL );
   failed |= output_int16( fp_mixture, 0 );              /* dummy */
   if( failed ) goto failure;
   
   strcpy( buf, dirname );
   strcat( buf, "/gconst.bin" );
   fp_gconst = fopen( buf, "wb" );
   failed |= ( fp_gconst == NULL );
   failed |= output_int16( fp_gconst, var_idx_cnt );     /* dummy */
   if( failed ) goto failure;
   
   
   // If input HMM mean or variance in byte, then output scale.bin
   // file immediately.  Output scales for mean and variance for
   // all dimensions.  If mean or variance is in short, the user
   // may still be requesting output in byte, in which case the
   // scale file will be output again after the conversion.
   if( m_bInputByteMeans || m_bInputByteVars )
   {
      strcpy( buf, dirname );
      strcat( buf, "/scale.bin" );
      fp_scale = fopen( buf, "wb" );
      failed |= ( fp_scale == NULL );
      if( failed ) goto failure;
      
      
      for( scaleIndex=0; scaleIndex < 2*m_inputFeatDim; scaleIndex++ )
      {
         failed |= output_int16( fp_scale, scale_base[scaleIndex] );
      }
      fclose( fp_scale );
      if( failed ) goto failure;
   }
   
   
   /* zero-based count of output hmms */
   ihmm = 0;
   
   /* Loop over all hmm sets, usually male and female */
   for( hmmSet = 0; hmmSet < m_inputSets; hmmSet++ )
   {
      /*  first hmm of each set is silence */
      failed |= process_one_hmm( (int) sil_n_state,
      (int) sil_hmm[ sil_n_state - 1 ],
      sil_hmm );
      if( failed ) goto failure;
      
      hmm_offset[ihmm+1] = hmm_offset[ihmm] + sil_n_state;
      ihmm++;
      
      /* Remaining hmms of set, hmm[ 1 .. hmm_idx - 1 ] */
      for ( i = 1; i < hmm_idx; i++ )
      {
         
         tranSetOffset =  m_bInputGDTrans ?  hmm[i].c*m_inputSets + hmmSet : hmm[i].c;
         offset = tranoffset_base[ tranSetOffset ];
         
         pshort = &tran_base[ offset ];
         n_state = (int) *pshort;
         
         // Now cluster[] array dynamically allocated to max states in hmm
         //demand( n_state < MAX_STATE, "Error; exceed MAX_STATE\n");
         //failed |= !(n_state < MAX_STATE );
         //if( failed ) goto failure;
         
         // Determine the pdf cluster for each state of the hmm[i]
         for ( j = 0; j < n_state - 1; j++ )
         {
            
            /* decision tree classification */
            
            failed |= get_cluster_idx( hmm[i].c, hmm[i].l, hmm[i].r, j,
            hmmSet, &tempidx );
            if( failed ) goto failure;
            cluster[j] = (short) tempidx;
         }
         
         failed |= process_one_hmm( n_state, tranSetOffset, cluster );
         if( failed ) goto failure;
         
         hmm_offset[ihmm + 1] = hmm_offset[ihmm] + n_state;
         ihmm++;
      }
   }
   
   /* --------------------------------------------------
    output pdf (mixture, cluster) offset
    -------------------------------------------------- */
   
   failed |= output_int16( fp_pdf, cluster_idx_cnt );
   for ( i = 0; i < cluster_idx_cnt; i++ )
      failed |= output_int16( fp_pdf, cluster_offset[ i ] );
   
   if( failed ) goto failure;
   
   /* --------------------------------------------------
    output size
    -------------------------------------------------- */
   
   // Size in number of mean vectors
   rewind( fp_mean );
   failed |= output_int16( fp_mean, mean_idx_cnt );
   
   // Size in number of variance vectors
   rewind( fp_var );
   failed |= output_int16( fp_var, var_idx_cnt );
   
   // Size in number of gconst values (same as number of variance vectors)
   rewind( fp_gconst );
   failed |= output_int16( fp_gconst, var_idx_cnt );
   
   // Size of data in int
   rewind( fp_mixture );
   failed |= output_int16( fp_mixture, cluster_offset[ cluster_idx_cnt ] );
   
   // Size of data in int
   rewind( fp_tran );
   failed |= output_int16( fp_tran, tran_offset[ tran_idx_cnt ] );
   
   // Size of data in int
   rewind( fp_hmm );
   failed |= output_int16( fp_hmm, hmm_offset[ hmm_idx * m_inputSets ] );
   
   // Output offsets to each hmm in the output hmm binary file
   for (i = 0; i < hmm_idx * m_inputSets; i++)
   {
      failed |= output_int16( fp_hmm, hmm_offset[i] );
   }
   
   
   // dim.bin holds the
   // number of static mfcc elements in the feature vector.
   FILE *fp;
   strcpy( buf, dirname);
   strcat( buf, "/dim.bin" );
   fp = fopen( buf, "wb" );
   failed |= ( fp == NULL );
   failed |= output_int16( fp, m_inputStaticDim );
   fclose( fp );
   
   #ifdef OFFLINE_CLS
   /* save RAM mean to ROM mean index mapping */
   strcpy( buf, dirname);
   strcat( buf, "/o2amidx.bin" );
   fp = fopen( buf, "wb" );
   failed |= ( fp == NULL );
   failed |= output_int16( fp, mean_idx_cnt - 1);
   for (i=0;i<mean_idx_cnt;i++)
      failed |= output_int16( fp, mean_idx[i]);
   fclose( fp );
   #endif
   
   
   failure:
      
      /* --------------------------------------------------
       close files
       -------------------------------------------------- */
      
      fclose( fp_hmm );
      fclose( fp_mean );
      fclose( fp_var );
      fclose( fp_tran );
      fclose( fp_pdf );
      fclose( fp_mixture );
      fclose( fp_gconst );
      
      // Free the hmm_offset data
      if( hmm_offset )
         free( hmm_offset );
      
      // Free cluster array
      if( cluster )
         free( cluster );
      
      /*
       printf("dim = %d, n_tran = %d, n_mean = %d, n_variance = %d, "
       "n_cluster = %d\n",
       DIM, tran_idx_cnt, mean_idx_cnt, var_idx_cnt, cluster_idx_cnt);
      
       i = net_size +
       hmm_offset[ hmm_idx * 2 ] +
       mean_idx_cnt * DIM +
       var_idx_cnt * DIM +
       tran_offset[ tran_idx_cnt ] +
       cluster_idx_cnt +
       var_idx_cnt +
       cluster_offset[ cluster_idx_cnt ];
      
       printf("Vocabulary = %d, Total model size = %d words\n", word_idx, i);
       */
      
      return failed;
}


//-------------------------------
int CTIesrFlex::process_one_hmm(int n_state, int idx, short *cluster)
{
   
   int   i, offset, size;
   int   old_cnt;                        /* if add to idx list, update offset */
   int   new_idx;
   short *pshort;
   
   // track any failure
   int failed = false;
   
   /* --------------------------------------------------
    transition
    -------------------------------------------------- */
   
   offset = tranoffset_base[ idx ];
   pshort = &tran_base[ offset ];
   
   //demand( n_state == *pshort, "Error: tran dim mismatch\n");
   failed |=  !(n_state == *pshort);
   if( failed ) return failed;
   
   old_cnt = tran_idx_cnt;
   
   if ( tran_idx_aug[ idx ] == -1 )
   {
      new_idx = tran_idx_cnt;
      tran_idx_cnt++;
      tran_idx_aug[ idx ] = new_idx;
   }
   else
      new_idx=tran_idx_aug[ idx ];
   
   
   if ( tran_idx_cnt > old_cnt )
   {       /* add, update offset */
      
      size = n_state * n_state;           /* transition matrix size */
      
      /* output */
      
      failed |= output_int16( fp_tran, n_state );
      for (i = 1; i < size; i++)
         failed |= output_int16( fp_tran, (int) tran_base[ offset + i ] );
      
      
      /* update offset for next tran entry.  The last entry in the tran_offset
       array points to just past the location of the transitions. */
      tran_offset[ tran_idx_cnt ] = tran_offset[ old_cnt ] + size;
   }
   
   /* --------------------------------------------------
    hmm
    -------------------------------------------------- */
   
   failed |= output_int16( fp_hmm, (int) tran_offset[ new_idx ] );   /* tran offset */
   
   for ( i = 0; i < n_state - 1; i++ )
   {
      
      old_cnt = cluster_idx_cnt;
      
      if ( cluster_idx_aug[ (int) cluster[i] ] == -1 )
      {
         
         new_idx=cluster_idx_cnt;
         cluster_idx_cnt++;
         
         cluster_idx_aug[ (int) cluster[i] ]= new_idx;
      }
      else
         new_idx=cluster_idx_aug[ (int) cluster[i] ];
      
      if ( cluster_idx_cnt > old_cnt )
      {
         
         offset =  clusteroffset_base[ cluster[i] ];
         
         if(m_bPackWtNmix == 1)
         {
            size = (int) (0xff & cluster_base[ offset ]);                /* n_mix */
         }
         
         else
         {
            size = cluster_base[ offset ];
            
         }
         
         size = size * 3 + 1;
         
         /* output */
         
         failed |= process_one_cluster( (int) cluster[i] );
         if( failed ) return failed;
         
         /* update offset for next cluster entry (i.e. mixture, pdf) */
         
         cluster_offset[ cluster_idx_cnt ] = cluster_offset[ old_cnt ] + size;
      }
      
      failed |= output_int16( fp_hmm, new_idx );                      /* pdf_idx */
   }
   
   return failed;
}


//-----------------------------
int CTIesrFlex::find_idx_idx(int *list, int *n_list, int max, int idx, int *new_idx)
{
   /*---------------------------------------------------------------------------
    find the new index (codebook used in small vocabulary) of an
    index (complete codebook)
    if not found, add it to the index list
    ---------------------------------------------------------------------- */
   
   int   i;
   int failed = false;
   
   for ( i = 0; i < *n_list; i++ )
   {
      if ( list[ i ] == idx )
      {
         *new_idx = i;    /* found */
         return failed;
      }
   }
   
   //demand( *n_list < max, "Error: exceed max index list\n");
   failed |= !( *n_list < max );
   if( failed ) return failed;
   
   list[ *n_list ] = idx;                 /* not found, add to the idx list */
   *n_list += 1;
   
   *new_idx = ( *n_list - 1 );
   return failed;
}



/*----------------------------------------------------------------
 process one cluster

 output to various GTM files
 ----------------------------------------------------------------*/
int CTIesrFlex::process_one_cluster(int idx)
{
   int   i, offset, n_mix;
   int   new_idx, old_cnt, wt_idx, wt_idx_temp;  //wt_idx temp is used when the data is packed
   short * pshort;
   int failed = false;
   int offset_inc = 3; // for wt, mean idx, var idx
   
   offset = clusteroffset_base[idx];
   if(m_bPackWtNmix != 1) //if weight index and nummix are not packed together
   {
      n_mix = (int) cluster_base[ offset ];
   }
   else //read the last 8 bits as nummix and the first  8 as wt_index
   {
      n_mix = (int)(0xff & cluster_base[offset]);
      wt_idx_temp = (int)( (0xff00 & cluster_base[offset]) >> 8 ) ;
   }
   
   failed |= output_int16( fp_mixture, n_mix );
   if( failed ) return failed;
   
   offset++;
   
   
   
   //code to read the weights file and add it on to the current exisiting structure
   //when weights are not explicitly specified weights are VQ'ed
   if(m_bInputWeightConfig != 0) 
   {
      if(m_bPackWtNmix == 1)
      {
         wt_idx = wt_idx_temp * m_lengthWtVec;
         offset_inc = 2;
         offset--;
      }
      else if(m_bInputWeightConfig == 2)
      {
         wt_idx = (n_mix-1) * m_lengthWtVec ;
         offset--;
         offset_inc = 2;
      }
      else
      {
         wt_idx = cluster_base[offset] * m_lengthWtVec; //read the weight index
         offset_inc = 2;
      }
      
   }
   
   /* loop thru all Gaussian components */
   
   for ( i = 0; i < n_mix; i++, offset += offset_inc )
   {
      
      /* --------------------------------------------------
       wgt
       -------------------------------------------------- */
      if(m_bInputWeightConfig == 0) //if weights explicitly specified
      {
         
         pshort = &cluster_base[ offset ];
         failed |= output_int16( fp_mixture, (int)* pshort );
         if( failed ) return failed;
      }
      else   // if weights are codebook entries
      {
         pshort = & weight_base[wt_idx+i] ;
         failed |= output_int16( fp_mixture, (int)* pshort );
         if( failed ) return failed;
      }
      
      /* --------------------------------------------------
       mean
       -------------------------------------------------- */
      
      old_cnt = mean_idx_cnt;
      
      
      if ( mean_idx_aug[cluster_base[ offset + 1 ] ] == -1 )
      {
         new_idx= mean_idx_cnt;
         
         // mean_idx is used by VQHMM
         mean_idx[mean_idx_cnt] = cluster_base[offset+1];
         
         mean_idx_cnt++;
         mean_idx_aug[cluster_base[ offset + 1 ] ]= new_idx;
      }
      else
         new_idx=mean_idx_aug[cluster_base[ offset + 1 ] ];
      
      
      
      /* If mean vector of this index has not been output, then
       output it, adding bias if necessary */
      if ( mean_idx_cnt > old_cnt )
      {
         unsigned int meanIndex = cluster_base[ offset + 1 ];
         
         /* If mean vector is in short and is CMN, then add bias */
         if( !m_bInputByteMeans && m_bInputCMN )
         {
            short* fvector = (short*)mean_base + meanIndex*m_inputFeatDim;
            add_mean( fvector, m_inputStaticDim, 1);
         }
         
         failed |= output_feature_vector( fp_mean, mean_base, meanIndex, m_bInputByteMeans );
         
      }
      
      failed |= output_int16( fp_mixture, new_idx );
      if( failed ) return failed;
      
      /* --------------------------------------------------
       var
       -------------------------------------------------- */
      
      old_cnt = var_idx_cnt;
      
      if ( var_idx_aug[ cluster_base[ offset + 2 ] ] == -1 )
      {
         new_idx=var_idx_cnt;
         var_idx_cnt++;
         var_idx_aug[ cluster_base[ offset + 2 ] ]= new_idx;
      }
      else
         new_idx=var_idx_aug[ cluster_base[ offset + 2 ] ];
      
      
      if ( var_idx_cnt > old_cnt )
      {
         unsigned int varIndex = cluster_base[offset + 2];
         
         failed |= output_feature_vector( fp_var, var_base, varIndex, m_bInputByteVars );
         
         failed |= output_int16(fp_gconst, (int) gconst_base[varIndex] );
         
         if( failed ) return failed;
      }
      
      failed |= output_int16( fp_mixture, new_idx );
   }
   
   return failed;
}


/*----------------------------------------------------------------
 get_cluster_idx

 Given the center, left, and right context of a triphone, and the
 state of the triphone, and the gender, determine the pdf cluster
 index corresponding to the hmm from the acoustic decision trees and
 questions.
 ----------------------------------------------------------------*/
int CTIesrFlex::get_cluster_idx(int c, int l, int r, int s, int gender, int *index)
{
   int    n_state, idx;
   int    base_offset;
   Modelnode model;
   // Treenode  *ptmptnode;
   int    failed = false;
   int clusterOffset;
   int tranSetOffset;
   int phnIndex;
   int hmmSet;
   int emitStates;
   short *tree_ptr;
   
   // If input is monophones, cluster index must be obtained
   // directly from data since no decision tree information exists.
   // This assumes monophone cluster data is in a specific order.  Cluster data
   // is in the order of monophones according to the monophone list,
   // and all clusters (states) for one set (gender) are together sequentially
   // in the fxclust.cb file, and the gender-independent sil clusters are at
   // the end of fxclust.cb.
   if( m_bInputMonophone )
   {
      clusterOffset = 0;
      for( phnIndex = 0; phnIndex < c; phnIndex++ )
      {
         for( hmmSet = 0; hmmSet < m_inputSets; hmmSet++ )
         {
            // Find number of emitting states for preceding monophones and sets
            tranSetOffset =  m_bInputGDTrans ?  m_inputSets*phnIndex + hmmSet  :  phnIndex;
            emitStates = tran_base[ tranoffset_base[ tranSetOffset ] ] - 1;
            
            clusterOffset += emitStates;
         }
      }
      
      for( hmmSet = 0; hmmSet < gender; hmmSet++ )
      {
         // Find number of emitting states for desired phone but prior sets
         tranSetOffset =  m_bInputGDTrans ?  m_inputSets*c + hmmSet  :  c;
         emitStates = tran_base[ tranoffset_base[ tranSetOffset ] ] - 1;
         
         clusterOffset += emitStates;
      }
      
      *index = clusterOffset + s;
      
      return false;
   }
   
   
   /* triphone state info */
   
   model.c = c;     /* center phone idx */
   model.l = l;     /* left phone */
   model.r = r;     /* right phone */
   model.s = s;     /* state idx, 0 based */
   
   /* read tree and map */
   
   // i_ptr = (int *) treetbl_base[ model.c * m_inputSets + gender ];
   base_offset = treetbl_base[model.c * m_inputSets + gender];
   
   // n_state = *i_ptr;
   n_state = treetbl_base[ base_offset++ ];

   // demand( s < n_state, "Error: state idx too large\n");
   failed |= !( s < n_state );
   if( failed ) return failed;
   
   tree_ptr = (short *) ( tree_base + treetbl_base[ base_offset + s ] );
   
   // cur_node = 0;
   
   failed |= ParseTree( &model, tree_ptr, &idx );
   
   *index = idx;
   
   return failed;
   
   /*
    Now replaced by direct processing of acoustic decision tree
    without building the tree structure.
   
    failed |= build_tree( &ptmptnode);
    if( failed ) return failed;
   
    failed |= classify(tree, &model, &idx);
    if( failed ) return failed;
   
    *index = idx;
   
    return failed;
    */
}

/*----------------------------------------------------------------
 ParseTree

 Given a model set, and a pointer to the acoustic decision tree,
 determine the cluster index of the cluster corresponding to the
 present context-dependent triphone state by in-place processing of
 the acoustic decision tree.

 ----------------------------------------------------------------*/
int CTIesrFlex::ParseTree( Modelnode const* apModel,  short const* aTopTreeNode,
int* aClusterIndex )
{
   int failed = false;
   int intree = true;
   int yesAnswer;
   short const* tree = aTopTreeNode;
   
   // This loop should continue until a cluster is found.  The only
   // reason not to find a cluster is an invalid tree.
   while( intree )
   {
      // Test if this tree node is terminal.  If so, then the cluster
      // index has been found.
      if( *tree & 0x8000 )
      {
         *aClusterIndex = *tree & 0x7FFF;
         return failed;
      }
      
      
      // Tree node is a question node, so ask the question
      yesAnswer = yes_to_question( apModel, *tree);
      
      // Advance in the tree node list to the "yes" node of the present
      // question, which immediately follows in the tree node list.
      tree++;
      
      // If the answer to the context question was no, advance in the
      // tree node list to the tree node corresponding to a no answer.
      // Note that tree starts at a node which is the "yes" node, and
      // ends up at the no node.
      if( !yesAnswer )
      {
         tree = SkipToNoNode( tree );
      }
   }
   
   // If the tree is valid, should never get here
   failed = true;
   return failed;
}


/*----------------------------------------------------------------
 SkipToNoNode

 This function goes down the list of nodes in the encoded tree
 until it finds the corresponding "no" node to the "yes" node
 which is input to this function.  Note that this function
 must be called with a node that is a "yes" node.

 ----------------------------------------------------------------*/
short const* CTIesrFlex::SkipToNoNode( short const* aYesTreeNode )
{
   short const* noNode;
   
   // Check if the starting "yes" node is a terminal node.
   // If so, the no node is immediately after it in the tree;
   if( *aYesTreeNode & 0x8000 )
   {
      noNode = ++aYesTreeNode;
   }
   else
   {
      // Since the yes node was not terminal, decend the yes node,
      // and return the node after all tree nodes in the yes node tree,
      // which is the no node at the same level as the yes node.
      noNode = DescendQuestionNode( aYesTreeNode );
   }
   
   return noNode;
}


/*----------------------------------------------------------------
 DescendQuestionNode

 This function decends from a question node in the acoustic decision
 tree to return the next node that comes after all the binary tree
 nodes at and below the question node node.  At the present time,
 local variables are used to hold yes, no and next nodes for
 debugging. This could be simplified to a single node pointer.
 The function now uses recursive calls to decend the tree

 ----------------------------------------------------------------*/
short const* CTIesrFlex::DescendQuestionNode( short const* aQuestionNode )
{
   short const* yesNode;
   short const* noNode;
   short const* nextNode;
   
   // Go to the yes node corresponding to the input question node,
   // which immediately follows the question node in the tree node
   // list.
   yesNode = aQuestionNode;
   yesNode++;
   
   // First decend the yes node of this present question node
   
   // Check if the yes node is a terminal node.
   if( *yesNode & 0x8000 )
   {
      // If yes node is terminal, the node immediately
      // following the yes node in the node list is the no node,
      noNode = yesNode;
      noNode++;
   }
   else
   {
      // If yes node is a question, decend the yes node and
      // return the next node after the yes node portion of the
      // tree, which is the no node for the current question.
      noNode = DescendQuestionNode( yesNode );
   }
   
   
   // Now decend the no node
   
   
   // Check if the no node is a terminal node.
   if( *noNode & 0x8000 )
   {
      // If terminal,  the next node after the no node is
      // the next node after the current question node.
      nextNode = noNode;
      nextNode++;
   }
   else
   {
      // Descend the node corresponding to this no node question
      // and return the next node after the no node portion of
      // the tree, which is the next node after the current
      // question node.
      nextNode = DescendQuestionNode( noNode );
   }
   
   return nextNode;
}


/*----------------------------------

 These functions no longer used, since tree parsing is done directly from
 the encoded tree in memory.

 //--------------------------------------
 int CTIesrFlex::build_tree( Treenode **pptnode )
 {
 Treenode *node;
 short word;
 int failed = false;

 word = *tree_offset;

 if ( ( word & 0x8000 ) == 0 )
 {
 // question
 failed |= create_tree_node(0, (int) word, &node );
 if( failed ) return failed;

 tree_offset++;

 failed |= build_tree( &node->yes );
 if( failed ) return failed;

 tree_offset++;

 failed |= build_tree( &node->no );
 if( failed ) return failed;

 *pptnode = node;

 return failed;
 }
 else
 {
 // leaf
 failed |= create_tree_node(1, (int) ( word & 0x7fff ), &node );

 *pptnode = node;
 return failed;
 }
 }


 //--------------------------------
 int CTIesrFlex::create_tree_node(int is_leaf, int value, Treenode **pptnode)
 {
 int failed = false;

 //demand( cur_node < MAX_TREENODE, "Error: exceed MAX_TREENODE\n");
 failed = !( cur_node < MAX_TREENODE );
 if( failed ) return failed;

 tree[ cur_node ].qs_frame = (is_leaf << 16) + value;
 tree[ cur_node ].yes = 0;
 tree[ cur_node ].no = 0;

 cur_node++;
 *pptnode = ( &tree[ cur_node - 1 ] );

 return failed;
 }


 //---------------------------------
 int CTIesrFlex::classify( Treenode *node, Modelnode *model, int *phnidx)
 {
 int failed = false;
 int idx;

 //demand( (int) tree, "Error: NULL tree input to classify()\n");
 failed = !( (int) tree );
 if( failed ) return failed;

 if ( ( node->qs_frame & 0x10000 ) == 0x10000 )
 {         // leaf node
 *phnidx = ( node->qs_frame & 0xffff );
 return failed;
 }
 else if ( yes_to_question(model, node->qs_frame) )
 {   // yes
 failed |= classify(node->yes, model, &idx );
 if(failed ) return failed;

 *phnidx = idx;
 return failed;
 }
 else
 {                                                 // no
 failed |= classify(node->no, model, &idx);
 if( failed ) return failed;

 *phnidx = idx;
 return failed;
 }

 return failed;
 }

 --------------------------------*/



//-----------------------------
int CTIesrFlex::yes_to_question( Modelnode const* model, int qs)
{
   unsigned char *c_ptr;
   int  i, lr, idx, qs_len;
   
//   c_ptr = (unsigned char *) qs_offset[ qs ];
   c_ptr = (unsigned char *) ( qs_base + qs_offset[ qs ] );

   qs_len = (int) *c_ptr;
   c_ptr++;
   for (i = 0; i < qs_len; i++)
   {
      lr = c_ptr[i] & 0x80;        /* right: 0x80, left: 0 */
      idx = c_ptr[i] & 0x7f;       /* phone index */
      if ( lr )
      {
         if ( idx == model->r ) return 1;
      } else
      {
         if ( idx == model->l ) return 1;
      }
   }
   return 0;
}


/*----------------------------------------------------------------
 demand

 Check for a condition being true, and if not throw a character pointer
 exception.
 ----------------------------------------------------------------*/
void CTIesrFlex::demand(int condition, char *msg)
{
   if( !condition )
   {
      // exit(1);
      
      // The demand function is only used within the
      // parse_grammar public member function. We must not exit, so we
      // raise an exception. Note that this implies that the call to
      // the parse_grammar function must be guarded.
      
      // old WinCE way of throwing an exception
      //RaiseException( CTIesrFlex::STATUS_FAIL_PARSE, 0, 0, NULL );
      
      // throw CTIesrFlex::STATUS_FAIL_PARSE;
      throw msg;
   }
   return;
}


//-----------------------------------------------
// This function is defined since the MS eMbedded C++ 3.0
// Smartphone emulator returns non-zero if c is EOF.
//
int CTIesrFlex::myisalpha(int c)
{
   
   if( c == EOF )
      return 0;
   
   return( isalpha(c) );
}


//------------------------------------------------------
// This function is defined since WCE does not support
// rewind.
//
void CTIesrFlex::rewind(FILE *fp)
{
   fseek( fp, 0L, SEEK_SET );
   clearerr( fp );
}


/*====================================================
 Functions below this line convert from
 16-bit mfcc to 8-bit mfcc.  This is necessary to accommodate
 the TIesr ability to use only 8-bit model features.
 ===========================================================*/



/*----------------------------------------------------------------
 model_bit_16_to_8

 This function is called to convert the mean and variance vectors of
 grammar files already output by TIesrFlex into 8-bit feature mean
 and variance files.  The user may select mean and/or variance to
 convert to byte.  If the file format is already byte, then
 conversion is not performed for that file.  However, scaling must be
 read in order to output the proper scaling information.


 ----------------------------------------------------------------*/
int CTIesrFlex::model_bit_16_to_8(char *source_dir, char *dest_dir,
short endien, short add_mean_vec,
int aByteMeans, int aByteVars )
{
   short *max_scale;
   short n_mfcc, n_dim, *vec, nbr;
   char fname[ FILENAME_MAX ];
   int failed;
   short dim;
   
   // Ignore - function does not need endien, just retain for compatibility
   //little_endian = endien;
   
   /* read the dimension of mfcc  */
   sprintf(fname, "%s/%s", source_dir, "dim.bin");
   n_mfcc = load_n_mfcc( fname );
   if( n_mfcc == 0 )
      return FAIL;
   
   
   /* Allocate scale memory */
   n_dim = n_mfcc * 2;
   max_scale = (short *) malloc( sizeof(short) * 2 *n_dim );
   if( max_scale == NULL )
      return FAIL;
   
   /* Initialize scale memory by reading from memory or setting to zeros. */
   sprintf(fname, "%s/%s", source_dir, "scale.bin");
   failed = load_scales( fname, max_scale, n_dim );
   if( failed )
      goto failure;
   
   /* If byte means desired, and input mean data is in short,
    convert short means to bytes.  This also sets the scaling for
    the byte means in the max_scale array.  */
   if( aByteMeans && ! m_bInputByteMeans )
   {
      /* load all mean vectors */
      sprintf(fname, "%s/%s", source_dir, "mu.bin" );
      vec = load_16( fname, n_mfcc , &nbr);
      if( vec == NULL )
         goto failure;
      
      /* add mean vector correction to word features if needed */
      if ( add_mean_vec ) add_mean(vec, n_mfcc, nbr);
      
      // Convert to byte mean vectors and output them */
      sprintf(fname, "%s/%s", dest_dir, "mu.bin" );
      failed = output_obs(vec, nbr, n_dim, n_mfcc, max_scale, fname);
      free(vec);
      if( failed )
         goto failure;
   }
   
   /* Input data is in short, and short is requested, so scale not used for means. */
   else if ( !aByteMeans && !m_bInputByteMeans )
   {
      /* Initialize scales to zero as placeholders.  They will not be used
       in TIesr recognizer. */
      for( dim = 0; dim < n_dim; dim++ )
      {
         max_scale[dim] = 0;
      }
   }
   
   
   /* If byte inverse variance desired, and input data in short, convert to byte
    which also loads the scale array. */
   if( aByteVars && ! m_bInputByteVars )
   {
      /* load all variance vectors */
      sprintf(fname, "%s/%s", source_dir, "var.bin");
      vec = load_16( fname, n_mfcc , &nbr);
      if( vec == NULL )
         goto failure;
      
      
      /* Convert to byte variances and output them */
      sprintf(fname, "%s/%s", dest_dir, "var.bin");
      failed = output_obs(vec, nbr, n_dim, n_mfcc, max_scale + n_dim, fname);
      free(vec);
      if(failed )
         goto failure;
   }
   
   /* Input var data is in short, and short desired, so zero the scale. */
   else if( !aByteVars && !m_bInputByteVars )
   {
      /* Initialize scales to zero as placeholders.  They will not be used
       in TIesr recognizer. */
      for( dim = 0; dim < n_dim; dim++ )
      {
         max_scale[n_dim + dim] = 0;
      }
   }
   
   
   /* Output the scale information */
   sprintf(fname, "%s/%s", dest_dir, "scale.bin");
   failed = save_scales(fname,  max_scale, n_dim);
   if( failed )
      goto failure;
   
   free( max_scale );
   return OK;
   
   failure:
      free(max_scale);
      return FAIL;
      
}

//---------------------------------------
// Find MFCC dimension
//
short CTIesrFlex::load_n_mfcc(char *fname)
{
   FILE  *fp;
   short n_mfcc;
   
   fp = fopen(fname, "rb");
   if (!fp)
   {
      return 0;
      //fprintf(stderr,"can't open %s\n",fname);
      //exit(0);
   }
   
   fread(&n_mfcc, sizeof(short), 1, fp);
   fclose( fp );
   return n_mfcc;
}

//--------------------------------------
// Read all vectors from file into memory
//
short * CTIesrFlex::load_16(char *fname, short n_mfcc, short *nbr)
{
   FILE  *fp;
   int n_dim = 2 * n_mfcc;
   short *b;
   
   //printf("reading from \"%s\"\n", fname);
   fp = fopen(fname, "rb");
   if( fp == NULL )
      return NULL;
   
   fread(nbr, sizeof(short), 1, fp);
   //printf("size = %d\n", *nbr);
   b = (short *) malloc( *nbr * n_dim * sizeof(short) );
   if( b == NULL )
   {
      fclose( fp );
      return NULL;
   }
   
   fread(b, sizeof(short),  *nbr * n_dim, fp);
   fclose( fp );
   return b;
}

//--------------------------------
//  Add mean vector obtained from WAVES digits parked_trn _d7_2*
//
void CTIesrFlex::add_mean(short *vec, int n_mfcc, int nbr)
{
   int i, d;
   short *pv = vec;
   long tmp;
   for (i=0; i<nbr; i++, pv += n_mfcc * 2)
   {
      for (d=0; d<n_mfcc; d++)
      {
         tmp = (long) pv[d] + (long) mean_bias[d];
         pv[d] = long2short(tmp);
         if (tmp != pv[d])
         {
            /*fprintf(stderr,"vector %d clipped at dim %d: %ld to %d\n",
             i, d, tmp, pv[d]); */
         }
      }
   }
}

//------------------------------------------------
// Output 8-bit observations
//
int CTIesrFlex::output_obs(short obs[], short size, int n_dim, int n_mfcc, short max_scale[], char fname[])
{
   short *mu, i;
   FILE *pf;
   int failed;
   
   calc_max(obs, size, n_dim, n_mfcc, max_scale);
   
   pf = fopen(fname, "wb");
   if (!pf)
   {
      return FAIL;
      //printf("failed to open %s\n",  fname);
      //exit(0);
   }
   else
   {
      //printf("writing to \"%s\"\n", fname);
   }
   
   
   /* Output number of vectors */
   failed = write_vec(&size, 1, pf);
   if( failed )
   {
      fclose(pf);
      return failed;
   }
   
   for (mu = obs, i = 0; i < size; i++, mu += n_dim)
   {
      vector_packing(mu, (unsigned short *) mu, max_scale, n_mfcc);
      failed = write_vec(mu, n_mfcc, pf);
      if( failed )
      {
         fclose(pf);
         return failed;
      }
   }
   fclose(pf);
   return OK;
}

//-----------------------------------------------------
//
void CTIesrFlex::calc_max(short mu[], int nbr, int n_dim, int n_mfcc, short max_mu[])
{
   int j, i;
   for (j = 0; j < n_dim; j++) max_mu[j] = 0;
   for (i = 0; i < nbr; i++)
   {
      for (j = 0; j < n_dim; j++)
      {
         short a_mu = mu[i * n_dim + j ];
         if (a_mu < 0) a_mu = -a_mu;
         if (max_mu[j] < a_mu) max_mu[j] = a_mu;
      }
   }
   scale_max(max_mu, n_mfcc, n_dim);
}

//------------------------------------------------
//
void CTIesrFlex::scale_max(short max_mu[], int n_mfcc, int n_dim)
{
   int i, j;
   
   /*
    for (i = 0; i < 2; i++) {
    for (j = 0; j < n_mfcc; j++)
    printf("%6d", max_mu[j+i*n_mfcc]);
    printf("\n");
    }
    */
   
   for (j = 0; j < n_dim; j++)
   {
      i = 0;
      while (!(0x4000 & max_mu[j]))
      {
         max_mu[j] <<= 1;
         i++;
      }
      max_mu[j] = i;
   }
   
   /*
    for (i = 0; i < 2; i++) {
    for (j = 0; j < n_mfcc; j++)
    printf("%6d", max_mu[j+i*n_mfcc]);
    printf("\n");
    }
    */
}

//--------------------------------
//
int CTIesrFlex::write_vec(short *vec, short nbr, FILE *fp)
{
   int i;
   int failed;
   for (i=0; i<nbr; i++)
   {
      failed = output_word(vec[i], fp);
      if( failed )
         return failed;
   }
   
   return OK;
}


//--------------------------------------------
//
int CTIesrFlex::output_word(short value, FILE *fp)
{
   int failed;
   
   failed = output_int16( fp, value );
   
   return failed;
}

//-----------------------------------------------
//
void CTIesrFlex::vector_packing(short v_in[], unsigned short v_out[], short max_scale[], short D)
{
   short j;
   char tmp1, tmp2;
   
   for (j = 0; j<D; j++)
   {
      tmp1 = nbr_coding(v_in[j], max_scale[j], 0)>>8;
      tmp2 = nbr_coding(v_in[j + D], max_scale[j + D], 0)>>8;
      v_out[j] = (tmp1 << 8) + (tmp2 & 0xff);
   }
   
}

//----------------------------------------------------
//
short CTIesrFlex::nbr_coding(short o_mu, short max_mu, int prt)
{
   short a_mu = o_mu,  sign = 0;
   int tmp;
   short clipped = 0;
   
   if (a_mu<0)
   { /* make a positiver number */
      a_mu = -a_mu;
      sign = 1;
   }
   tmp = (a_mu << max_mu); /* shift */
   if (tmp > 32767)
   {
      a_mu = 32767;
      clipped = 1;
   }
   else a_mu = (short) tmp;
   if (a_mu < 0x7f00) a_mu += (1<<7);
   /*  else printf("a_mu = %x, no rounding\n", a_mu); */
   a_mu &= 0xff00;     /* 8 bits precision coding */
   #ifndef COMPACT
   a_mu >>= max_mu; /* restore */
   if (sign) a_mu = - a_mu;
   if (prt)
   {
      /* printf("ZZ=%d, %5d %5d, %5d %9.5f\n", max_mu, a_mu, o_mu, a_mu - o_mu,
       o_mu == 0? 0.: (a_mu - o_mu)/(float)(o_mu>0? o_mu:-o_mu)*100.); */
   }
   #else
   if (sign) a_mu = - a_mu;
   #endif
   
   /*
    if (clipped)
    fprintf(stderr,"clipping: original = %x (%d), actual = %x (%d) scale = %d)\n",
    o_mu, o_mu, a_mu>> max_mu, a_mu>> max_mu, max_mu );
    */
   return a_mu;
}

//-----------------------------------------
//
int CTIesrFlex::save_scales(char *fname, short scale[], short n_dim)
{
   FILE  *fp;
   int failed;
   
   fp = fopen(fname, "wb");
   if (!fp)
   {
      return FAIL;
      //fprintf(stderr,"can't open %s\n",fname);
      //exit(0);
   }
   failed = write_vec(scale, n_dim * 2, fp);
   fclose( fp );
   
   if( failed )
      return failed;
   
   return OK;
}

/*----------------------------------------------------------------
 load_scales

 Try to load byte scale factors.  If the file does not exist, then
 initialize scale with all zeros.  Otherwise read from file, and
 report if failure to do so.
 ----------------------------------------------------------------*/
int CTIesrFlex::load_scales(char *fname, short scale[], short n_dim)
{
   FILE  *fp;
   int dim;
   size_t nread;
   
   
   /* Open file and initialize scales if no file */
   fp = fopen(fname, "rb");
   if (!fp)
   {
      for(dim=0; dim < 2*n_dim; dim++)
         scale[dim] = 0;
      
      return OK;
   }
   
   
   /* Read scales */
   nread = fread( scale, sizeof(short), 2*n_dim, fp );
   fclose( fp );
   
   if( nread != (unsigned int)2*n_dim )
      return FAIL;
   
   return OK;
}


/*----------------------------------------------------------------
 output_feature_vector

 This function outputs a single feature vector based on its index in
 the input data.
 ----------------------------------------------------------------*/
int CTIesrFlex::output_feature_vector( FILE* fp, void *aFeatures,
unsigned int aIndex,
int aByteFeatureVector)
{
   int failed = OK;
   int feat;
   
   // Feature vector elements are bytes
   if( aByteFeatureVector )
   {
      int nSubVectors = m_inputFeatDim/m_inputStaticDim;
      unsigned short featWord;
      unsigned char *features = (unsigned char *)aFeatures;
      unsigned char *fvector = features + aIndex*m_inputFeatDim;
      
      // Byte features.  Presently TIesr Engine expects bytes according to
      // interlaced static and delta.  Can't handle static by itself, or
      // static, delta, and accel.
      if( nSubVectors != 2 )
      {
         return FAIL;
      }
      
      for ( feat = 0; feat < m_inputStaticDim; feat++ )
      {
         featWord = fvector[feat];
         featWord <<= 8;
         featWord |= fvector[feat + m_inputStaticDim];
         failed |=  output_int16( fp, (int)featWord );
      }
   }
   
   else
   {
      short *features = (short *)aFeatures;
      short *fvector = features + aIndex*m_inputFeatDim;
      for ( feat = 0; feat < m_inputFeatDim; feat++ )
      {
         failed |= output_int16( fp, (int)fvector[feat] );
      }
   }
   
   return failed;
}


/*----------------------------------------------------------------
 output_config

 This function outputs the configuration data for the model set
 just written out.
 ----------------------------------------------------------------*/
int CTIesrFlex::output_config( const char* aDirName, short aByteMeans, short aByteVars )
{
   FILE* fp;
   char  buf[ FILENAME_MAX ];
   int failed = 0;
   
   /* Open config file */
   strcpy( buf, aDirName );
   strcat( buf, "/config.bin" );
   fp = fopen( buf, "wb" );
   if( !fp )
      return FAIL;
   
   failed |= output_int16( fp, aByteMeans );
   failed |= output_int16( fp, aByteVars );
   failed |= output_int16( fp, little_endian );
   
   
   fclose(fp);
   
   if( failed )
      return FAIL;
   
   return OK;
}


/*----------------------------------------------------------------
 AddMultiplePronunciations

 This function adds multiple pronunciations to the fsa2 grammar network
 created by yyparse and process_grammar.  The multiple pronunciations are
 determined by the dictionary.  The user may choose to include the default
 rule pronunciation as a valid pronunciation, and to specify the maximum
 number of pronunciations allowed for a word.

 This function adds multiple pronunciations by adding new words to the word
 list and new transitions between sent_nodes.  The base word is given by the
 original word list.  If additional pronunciations are available from
 TIesrDict, then new words are added as the base word, followed by "+N",
 where N is >=1, indicating the word is the Nth multiple pronunciation.  For
 example, if the word "Marilyn" has three possible pronunciations, then these
 would be represented by the words in the word list of "Marilyn",
 "Marilyn+1", and "Marilyn+2".

 ----------------------------------------------------------------*/
CTIesrFlex::Errors CTIesrFlex::AddMultiplePronunciations( )
{
   CTIesrDict::Errors dictError;
   int src;
   int dst;
   int next;
   int newTranIndex;
   
   
   // Save the indices of the base number of words and transitions that
   // will be added to.
   int baseSentTranIndex = senttran_idx;
   
   
   //  Should not have called function, no multiple pronunciations wanted.
   if( m_maxPronunciations <= 1 )
      return ErrNone;
   
   
   // Add multiple pronunciations for each base word
   for( int wrdIndex = 0; wrdIndex < m_baseWordIndex; wrdIndex++ )
   {
      unsigned int numPron;
      
      
      // If the word is a _SIL* word, then continue, since this is a dummy for the
      // silence model, and it does not have more pronunciations.
      if( strncmp( word[wrdIndex], SIL, strlen(SIL)) == 0 )
      {
         continue;
      }
      
      // Find how many pronunciations are available for the word in the
      // dictionary, and also include the default pronunciation if the user
      // wants it.
      dictError = m_dict->GetNumberEntries( word[wrdIndex], &numPron );
      if( dictError != CTIesrDict::ErrNone )
      {
         return ErrDict;
      }
      if( m_bIncludeRulePron )
         numPron++;
      
      
      // If only one pronunciation is available for the word, or no pronunciation
      // then no need to add new pronunciations.
      if( numPron <= 1 )
         continue;
      
      
      // Add the new pronunciations to word list and senttran up to the number
      // available or the maximum number.
      int maxPron = (numPron > m_maxPronunciations)  ?  m_maxPronunciations  :  numPron;
      
      for( int newPron = 1; newPron < maxPron; newPron++ )
      {
         char newWord[MAX_WORDLEN];
         char suffix[MAX_WORDLEN];
         int newWordIndex;
         
         // Create the new "word"
         strcpy( newWord, word[wrdIndex] );
         sprintf( suffix, "+%d", newPron );
         strcat( newWord, suffix );
         
         // Try to add the new word to the word list
         try
         {
            add_word( newWord );
         }
         catch( char* exc )
         {
            m_parseFail = true;
         }
         if( m_parseFail )
            return ErrFail;
         
         // Zero-based word index
         newWordIndex = word_idx - 1;
         
         
         // Search all base transitions for the base word, and add a new
         // transition with the new word.  Note that the new word index
         // is offset by 2, since the senttran uses one-based word indices and
         // the first word is by definition "", so all other words start at
         // index 2.
         for( int trnIndex = 0; trnIndex < baseSentTranIndex; trnIndex++ )
         {
            // Check if transition matches base word.  If so, add a new
            // senttran transition for the new word.
            if( senttran[trnIndex].type == wrdIndex + 2 )
            {
               // The source and destination sent_nodes for this transition,
               // and next transition in linked list.
               src = senttran[trnIndex].src;
               dst = senttran[trnIndex].dst;
               next = senttran[trnIndex].next;
               // Insert the new transition after this senttran
               try
               {
                  create_tran( src, dst, newWordIndex + 2, next );
               }
               catch( char* exc )
               {
                  m_parseFail = true;
               }
               if( m_parseFail )
                  return ErrFail;
               
               // Connect the original senttran to the new senttran to fix up
               // the linked list. Note that senttran indices are one-based in
               // the senttran next field.
               newTranIndex = senttran_idx - 1;
               senttran[trnIndex].next = newTranIndex + 1;
            }
         }
      }
   }
   
   // Finished adding new multiple pronunciation words successfully.
   return ErrNone;
}


/*----------------------------------------------------------------
 LookupBasePronunciation

 This function looks up pronunciations for base "words" in the grammar.  A word
 may be a simple character string, such as "Marilyn", or it may be a
 character string with a suffix "+N" indicating that this is one of several
 pronunciations desired.  A base word does not have "+N" in the word.
 The function uses class member variables to determine how to interpret the
 word and puts the pronunciation for the word in aPron.  The reason for
 distinguishing between base words and multi-pronunciation words is that the
 lookup of rule pronunciations is faster if the pronunciation of the base
 word is looked up followed immediately by all multi-pronunciations of the
 same word.

 ----------------------------------------------------------------*/
CTIesrFlex::Errors CTIesrFlex::LookupBasePronunciation( const char* aWord, char* aPron )
{
   char* plusChr;
   char wrd[MAX_WORDLEN];
   CTIesrDict::Errors dictError;
   
   // Modifiable copy of word
   strcpy( wrd, aWord );
   
   // Is this a multi-pronunciation word?  If plusChr is NULL, this indicates
   // this is the base word, and not a multi-pronunciation additional word.
   // If this is a multi-pronunciation word, then this call is in error.
   plusChr = strrchr( wrd, '+' );
   if( plusChr )
   {
      return ErrNotBase;
   }
   
   
   // Get the desired pronunciation.  These should not cause an error, since
   // the count of dictionary words was done when the extra
   // multi-pronunciation words were put in the senttran transitions.  So
   // there should not be a request for an instance of the word not in the
   // dictionary.
   
   // User wants rule pronunciation for base word
   if( m_maxPronunciations == 0 || m_bIncludeRulePron )
   {
      dictError = m_dict->GetPronEntry( wrd, 0, aPron );
   }
   
   // User wants first dictionary pronunciation for base word
   else
   {
      dictError = m_dict->GetPronEntry( wrd, 1, aPron );
      
      // TIesrFlex may want the first dictionary pronunciation,
      // but no dictionary pronunciation exists.  This is not an
      // error, just have to use rule pronunciation.
      if( dictError == CTIesrDict::ErrNotInDictionary )
         dictError = CTIesrDict::ErrNone;
   }
   
   // This is the base word.  No "+N" suffix implies multiCount=0.
   // The next N for this base word, if it exists, should be N=1.
   m_multiCount = 0;
   
   
   return (dictError == CTIesrDict::ErrNone)  ?  ErrNone : ErrDict;
}


/*----------------------------------------------------------------
 LookupMultiPronunciation

 ----------------------------------------------------------------*/
CTIesrFlex::Errors CTIesrFlex::LookupMultiPronunciation( const char* aWord,
const char* aMultiWord,
char* aPron )
{
   char* plusChr;
   int multiCount;
   char wrd[MAX_WORDLEN];
   CTIesrDict::Errors dictError;
   
   // Modifiable copy of multiword
   strcpy( wrd, aMultiWord );
   
   // If looking up a multi-word pronunciation, then this must be the
   // next pronunciation in order of the presently used base word.
   plusChr = strrchr( wrd, '+' );
   if( plusChr )
   {
      // Determine which multi-pronunciation word this is
      multiCount = atoi( plusChr+1 );
      
      // Change wrd so it is a pointer to the base word string only
      *plusChr = '\0';
      
      // Ensure this multiword matches the base word.  If it does not, then
      // There is no error, but the words don't match, so no word should be
      // returned.
      if( strcmp( wrd, aWord) != 0 )
      {
         return ErrNoMatch;
      }
      
      // Ensure this is the next word in multi-word order
      if( multiCount != m_multiCount + 1 )
         return ErrFail;
   }
   else
   {
      // Word does not have "+N", and therefore is not multipronunciation
      // word.  Something is wrong.
      return ErrFail;
   }
   
   
   // If this is a multi-pronunciation word but only single pronunciations are
   // desired, then something is wrong.  This should not happen.
   if( m_maxPronunciations <= 1 )
   {
      aPron[0] = 0;
      return ErrFail;
   }
   
   
   // Get the next desired dictionary pronunciation in order.  These should not cause
   // an error, since the count of dictionary words was done when the extra
   // multi-pronunciation words were put in the senttran transitions.  So
   // there should not be a request for an instance of the word not in the
   // dictionary.
   dictError = m_dict->GetNextEntry( aPron );
   
   // Successfully looked up next sequential multiword for base word
   if( dictError == CTIesrDict::ErrNone )
      m_multiCount++;
   
   
   return (dictError == CTIesrDict::ErrNone)  ?  ErrNone : ErrDict;
}


/*----------------------------------------------------------------
 ExpandArray

 Expand the size of an array of items which may be either structures
 or simple types. This function may also be called to do the initial
 allocation.  aArray is a pointer to the present allocation.
 aItemSize is the size of an item in the array.  aNumAlloc is the
 number by which the array should be increased, and aNumInArray is a
 pointer to a counter that holds the present number of items.
 ----------------------------------------------------------------*/
void* CTIesrFlex::ExpandArray( void* aArray, int aItemSize, int aNumAlloc, int* aNumInArray )
{
   void* newArray;
   int newNumInArray;
   
   // Number of array objects to allocate
   newNumInArray = aNumAlloc + *aNumInArray;
   
   // Check if this is the first allocation
   if( *aNumInArray == 0 )
   {
      newArray = malloc( aItemSize*newNumInArray );
   }
   else
   {
      newArray = realloc( aArray, aItemSize*newNumInArray );
   }
   
   // If allocation was unsuccessful, throw a memory error
   if( !newArray )
      throw "Memory Allocation Error";
   
   // Allocation was successfull, so update number of objects in array,
   // and provide a pointer to the newly allocated array.
   *aNumInArray = newNumInArray;
   return newArray;
}


/*----------------------------------------------------------------
 FreeGrammarData

 This function frees all grammar data array structures used during
 creation of the grammar.  It is a private convenience function to the
 class.
 ----------------------------------------------------------------*/
void CTIesrFlex::FreeGrammarData(void)
{
   // Delete any preexisting grammar text copy
   delete [] cfg;
   cfg = NULL;
   
   // Free any preexisting parse word tokens and token array data
   if( token )
   {
      for( int tok = 0; tok < token_idx; tok++ )
         free( token[tok] );
      
      free( token );
      token = NULL;
      token_idx = 0;
      m_tokenMax = 0;
   }
   
   // Free any preexisting rule structures
   if( rule )
   {
      free( rule );
      rule = NULL;
      rule_idx = 0;
      m_ruleMax = 0;
   }
   
   // Free any preexisting node structures
   if( node )
   {
      free( node );
      node = NULL;
      m_nodeMax = 0;
      node_idx = 0;
   }
   
   // Free any preexisting dependency structures
   if( depend )
   {
      free( depend );
      depend = NULL;
      depend_idx = 0;
      m_dependMax = 0;
   }
   
   // Free any preexisting sentnode structures
   if( sentnode )
   {
      // Free any existing array of next nodes
      int snode;
      for( snode = 0; snode < sentnode_idx; snode++ )
      {
         if( sentnode[snode].next )
         {
            free( sentnode[snode].next );
         }
      }
      
      // Free all the sentnodes
      free( sentnode );
      sentnode = NULL;
      sentnode_idx = 0;
      m_sentnodeMax = 0;
   }
   
   // Free any preexisting senttran structures
   if( senttran )
   {
      free( senttran );
      senttran = NULL;
      senttran_idx = 0;
      m_senttranMax = 0;
   }
   
   // Free any preexisting words
   if( word )
   {
      free( word );
      word = NULL;
      word_idx = 0;
      m_wordMax = 0;
   }
   
   // Free any hmm entry array
   if( hmm )
   {
      free( hmm );
      hmm = NULL;
      hmm_idx = 0;
      m_hmmMax = 0;
   }
   
   // Free any symbol structure array
   if( sym )
   {
      // Free any existing array of next symbols in the sym structure
      int idxsym;
      for( idxsym = 0; idxsym < sym_idx; idxsym++ )
      {
         if( sym[idxsym].next )
         {
            free( sym[idxsym].next );
         }
      }
      
      // Free all sym structures
      free( sym );
      sym = NULL;
      sym_idx = 0;
      m_symMax = 0;
   }
   
   // Free stop symbol list
   if( stop_next )
   {
      free( stop_next );
      n_stop_next = 0;
      m_stopMax = 0;
   }
}


/*----------------------------------------------------------------
 AddNext

 Add integers to the end of an integer list, expanding the list
 size if necessary.
 ----------------------------------------------------------------*/
void CTIesrFlex::AddNext( int **apList, int *apNumUsed, int* apNumMax,
int aNumToAdd, int* aAddList )
{
   int idxAdd;
   
   int numUsed = *apNumUsed;
   int numMax = *apNumMax;
   int *aList = *apList;
   
   
   // Initial allocation if called when the list is empty
   // if aList is NULL numUsed should be zero.
   if( aList == NULL )
   {
      // Allocate NEXT_ALLOC more integers to the list
      // This will throw an exception if the expansion fails.
      *apList =  reinterpret_cast<int*>
      ( ExpandArray( reinterpret_cast<void*>(*apList),
      sizeof(int), NEXT_ALLOC, apNumMax ) );
      
      *apNumUsed = 0;
      
      aList = *apList;
      numMax = *apNumMax;
      numUsed = 0;
   }
   
   
   // Add new integers to the list, expanding the list as necessary
   for( idxAdd = 0; idxAdd < aNumToAdd; idxAdd++ )
   {
      
      if( numUsed >= numMax )
      {
         // Allocate NEXT_ALLOC more integers to the list
         // This will throw an exception if the expansion fails.
         *apList =  reinterpret_cast<int*>
         ( ExpandArray( reinterpret_cast<void*>(*apList),
         sizeof(int), NEXT_ALLOC, apNumMax ) );
         
         aList = *apList;
         numMax = *apNumMax;
      }
      
      aList[numUsed] = *aAddList++;
      numUsed++;
   }
   
   *apNumUsed = numUsed;
}


//This function makes in the means from the compressed mean files and populates the mean_base array
// this function has also the additional task of allocating space for the mean_base array
//
//-----------------------------------------------------------------------------------------------------------
int CTIesrFlex::makemean(char * dirname)
{
   short * mean_centr=NULL , * error_codebook =NULL, *dummy_mean=NULL ;
   unsigned short * mean_assoc=NULL, * enc_errors=NULL;
   unsigned int nEntries=0; //number of clusters,number of means
   unsigned short err_encoding_levels=0, tempp=0, nClust=0, nMean=0, nbits=4;
   FILE  *fp;
   size_t elementSize;
   int vectorSize;
   char fname[FILENAME_MAX];
   
   //read the mean centroids*****************************************
   
   // Determine sizes of input and output vectors
   elementSize =   sizeof( short );
   vectorSize = elementSize * m_inputFeatDim;
   
   // Open the mean file and get number of clusters
   strcpy(fname, dirname);
   strcat(fname, "cls_centr.bin");
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&nClust, sizeof(short), 1, fp);
   fread(&tempp, sizeof(short), 1, fp);
   // Allocate space to hold the centroids vectors
   mean_centr =(short *) malloc( nClust * vectorSize );
   if( !mean_centr )
   {
      fclose(fp);
      return FAIL;
   }
   
   // Read in the vector data, stepping over unused dimensions
   size_t nRead = fread( mean_centr, vectorSize, nClust, fp );
   
   fclose( fp );
   fp = NULL;
   
   if( nRead != nClust )
   {
      free(mean_centr);
      return FAIL;
   }
   
   // mean cluster reading ends here ************************************************
   //mean assocaition reading starts here*******************************************
   
   
   
   // Open the ROM2cls.bin file and get the assocaitions
   strcpy(fname, dirname);
   strcat(fname, "ROM2cls.bin");
   fp = fopen(fname, "rb");
   if ( fp == NULL )
   {
      return FAIL;
   }
   
   fread(&nMean, sizeof(short), 1, fp);
   fread(&tempp, sizeof(short), 1, fp);
   // Allocate space to hold the centroids vectors
   mean_assoc =(unsigned short *) malloc( nMean * sizeof(short) );
   if( !mean_assoc )
   {
      fclose(fp);
      return FAIL;
   }
   
   // Read in the vector data, stepping over unused dimensions
   nRead = fread( mean_assoc, sizeof(short), nMean, fp );
   
   fclose(fp);
   fp = NULL;
   
   if( nRead != nMean )
   {
      free(mean_centr);
      free(mean_assoc);
      return FAIL;
   }
   //****************************** reading of mean association ends here
   //***************************read error_codebook
   
   // first entry in the n levels and the total number of entries is nlevels*ndimensio
   
   // Determine sizes of input and output vectors
   elementSize =   sizeof( short );
   
   // Open the mean file and get number of clusters
   FILE *fp1 ;
   strcpy(fname, dirname);
   strcat(fname, "fxerrcb.bin");
   fp1 = fopen(fname, "rb");
   if ( fp1 == NULL )
   {
      return FAIL;
   }
   
   fread(&err_encoding_levels, sizeof(short), 1, fp1);
   
   // Allocate space to hold the centroids vectors
   error_codebook =(short *) malloc( err_encoding_levels * elementSize * m_inputFeatDim );
   if( !error_codebook )
   {
      fclose(fp);
      return FAIL;
   }
   //set encoding level info here
   if (err_encoding_levels == 16)
      nbits = 4;
   if (err_encoding_levels == 256 )
      nbits = 8;
   
   
   // Read in the vector data, stepping over unused dimensions
   nRead = fread( error_codebook, elementSize, err_encoding_levels*m_inputFeatDim, fp1 );
   fclose( fp1 );
   fp1 = NULL;
   
   if( nRead !=(unsigned int) err_encoding_levels*m_inputFeatDim )
   {
      free(mean_centr);
      free(mean_assoc);
      free(error_codebook);
      return FAIL;
   }
   //***********************error_codebook reading ends here
   //***********************read the encoded errors now :)
   
   // Determine sizes of input and output vectors
   elementSize =   sizeof( short );
   
   // Open the mean file and get number of clusters
   strcpy(fname, dirname);
   strcat(fname, "fxenc.bin");
   fp1 = fopen(fname, "rb");
   if ( fp1 == NULL )
   {
      return FAIL;
   }
   
   fread(&nEntries, sizeof(int), 1, fp1);
   
   // Allocate space to hold the centroids vectors
   enc_errors =(unsigned short *) malloc( nEntries * elementSize );
   if( !enc_errors )
   {
      fclose(fp1);
      return FAIL;
   }
   
   // Read in the vector data, stepping over unused dimensions
   nRead = fread( enc_errors , elementSize , nEntries, fp1 );
   
   fclose( fp1 );
   fp1 = NULL;
   
   if( nRead != nEntries )
   {
      free(mean_centr);
      free(mean_assoc);
      free(error_codebook);
      free(enc_errors);
      return FAIL;
   }
   //***********************all file reading concludes here *****************************
   //allocate space for the mean vectors and the associated vectors
   if( mean_base != NULL )
   {
      free( mean_base );
   }
   
   if( mean_idx_aug )
      free( mean_idx_aug );
   
   m_bInputByteMeans = false;
   
   // Determine sizes of input and output vectors
   elementSize = m_bInputByteMeans ? sizeof( unsigned char ) :
      sizeof( short );
      
      vectorSize = elementSize * m_inputFeatDim;
      
      // Allocate space to hold the mean vectors
      mean_base = malloc( nMean * vectorSize );
      if( !mean_base )
      {
         free(mean_centr);
         free(mean_assoc);
         free(error_codebook);
         free(enc_errors);
         return FAIL;
      }
      
      //assign the dummy mean the address of mean base
      dummy_mean = (short *)mean_base;
      //here goes in the mean generation routine
      unsigned int mean_centr_idx, mean_centr_offset, mean_cb_index=0, error_enc_count=0, error_enc_index, error_temp, prev_error_idx=0;
      unsigned short error_cb_offset;
      short temp;
      for(unsigned int mean_vector_count=0; mean_vector_count < nMean; mean_vector_count++)
      {
         //read in the cluster centroid mean
         mean_centr_idx =(int)mean_assoc[mean_vector_count];
         for(int dimension_count=0; dimension_count < m_inputFeatDim; dimension_count++)
         {
            //calculate the index of the encoded error array
            error_enc_index = (int)error_enc_count/nbits;
            if(error_enc_count == 0)
            {
               error_temp = enc_errors[error_enc_index];
            }
            if(error_enc_index > prev_error_idx)
            {
               //if the index has been incremented
               error_temp = enc_errors[error_enc_index];
               prev_error_idx = error_enc_index;
            }
            
            //read in the relevant bits into error_cb_offset this will now contain a number between 0-15
            if(nbits == 4)
               error_cb_offset =(unsigned short) (0x000f&error_temp);
            else
               error_cb_offset =(unsigned short) (0x00ff&error_temp);
            
            //shift the bits so that the next number can be read
            error_temp =(error_temp >> nbits);
            
            //calculate the dimension of the mean cluster centroid to be read
            mean_centr_offset = m_inputFeatDim*mean_centr_idx + dimension_count;
            //calculate the entry  of the entry from the error codebook to be read
            error_cb_offset = dimension_count*err_encoding_levels +(int)error_cb_offset;
            //add the error and the man centroid to obtain the error
            temp = error_codebook[error_cb_offset] + mean_centr[mean_centr_offset];
            
            dummy_mean[mean_cb_index++] =   temp;
            //increment the counter
            error_enc_count++;
         }
      }
      
      m_numMeans = nMean;
      
      // Allocate space to track usage of mean vectors
      mean_idx = (int*)malloc( nMean *sizeof(int) );
      mean_idx_aug = (int*)malloc( nMean * sizeof(int) );
      if( !mean_idx || !mean_idx_aug )
      {
         return FAIL;
      }
      
      free(error_codebook);
      free(mean_centr);
      free(mean_assoc);
      free(enc_errors);
      return OK;
}


