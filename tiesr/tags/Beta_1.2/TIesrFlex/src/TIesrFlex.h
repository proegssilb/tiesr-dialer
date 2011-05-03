/*=======================================================================
 TIesrFlex.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header defines the user interface to the TIesrFlex API which
 creates grammar network and HMM model binary files corresponding to
 an input grammar.  The network and HMM files are placed in a user
 specified directory.  This API uses the TIesrDict API to determine
 how to pronounce a word.  If the word is not in the dictionary, the
 pronunciation defined by the TIesrDT API is used.

 All words defined to TIesrFlex by the input grammar string must be spelled
 using alphabetic characters a-z and A-Z.  For example, use  "ONE" not "1",
 or "PERCENT" not "%".

 The sequence of usage of this API is to first instantiate an
 instance of the TIesrFlex object.

 Then call the LoadLanguage method to setup the dictionary for a
 language and load the HMM model information that will be used to
 create the HMM model set and network for a grammar to be recognized.

 After this loading is done, call ParseGrammar with the cfg grammar
 string that defines the grammar to be recognized.  The cfg grammar
 string has stringent requirements, which are documented elsewhere,
 and ensure that the grammar string actually defines a finite length
 regular grammar.

 After parsing the grammar, call OutputGrammar to have TIesrFlex
 determine and output TIesr format HMM models and associated
 parameters files, a TIesr grammar network file, and a word list.
 This can be output to a user selected directory.  During output, the
 user may select byte or word mean and variance outputs for HMMs, if
 the original HMM data loaded was in word format.

======================================================================*/

#ifndef _TIESRFLEX_H
#define _TIESRFLEX_H

#if defined (WIN32) || defined (WINCE)

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the TIESRFLEX_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// TIESRFLEX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef TIESRFLEX_EXPORTS
#define TIESRFLEX_API __declspec(dllexport)
#else
#define TIESRFLEX_API __declspec(dllimport)
#endif

#else
#define TIESRFLEX_API
#endif


#include <cstdio>

// The CTIesrFlex class contains a CTIesrDict pointer
#include <TIesrDict.h>


// tiesr_config belongs to another API and really should not be included in TIesrFlex
// #include <tiesr_config.h>

// These defines specify how TIesrFlex was built

// Used to include code only used for debugging
// #defined TIESRFLEXDEBUG


// Use a compact representation of mfcc's
#define COMPACT

// Offline VQ classes for VQ JAC supported as part of the model set
#define OFFLINE_CLS

// Network optimization is not enabled at this time
// #define USE_NET_OPTIMIZATION

//----------------------------------------------------------------
// CTIesrFlex
//
// The TIesrFlex API class
//
class TIESRFLEX_API CTIesrFlex
{
   public:
      
      // Enumeration of errors returned by the API
      enum Errors
      {
         ErrNone,
         ErrFail,
         ErrDict,
         ErrMemory,
         ErrSize,
         ErrType,
         ErrNoMatch,
         ErrNotBase,
         ErrNoOptimize
      };

      
   public:

      // Public access to the CGrammar object
      
      /*  Default constructor */
      CTIesrFlex(void);
      
      /* Default destructor */
      ~CTIesrFlex(void);
      

      /*----------------------------------------------------------------
       LoadLanguage
      
       This function loads all of the information from an input
       directory structure that allows TIesrFlex to create a grammar
       network and model set to recognize a grammar.  The base
       directory is at direct/language, which contains the decision
       trees.  The dictionary should be located at
       direct/language/dict_ident, and the HMM model information at
       direct/language/hmm_ident. The user may set little_endian to
       true in order to output model information in little endian
       format. Set addClosure to zero if closure phones (bcl, dcl, gcl,
       kcl, pcl, tcl) are not to be automatically added in the dictionary.
       ----------------------------------------------------------------*/
      Errors LoadLanguage( const char *direct, const char *language,
      const char *dict_ident, const char *hmm_ident,
      const int lit_end, const int addClosure );
      
   
      /*----------------------------------------------------------------
       Parse Grammar
      
       Parse a grammar and create the information in order to create HMM
       models.  This function creates a grammar network and a list of all of
       the needed HMMs for the grammar.  The user may specify multiple
       pronunciation options. aMaxPronunciations is the maximum number of
       pronunciations for a word that are allowed.  If this is set to one,
       then the first dictionary pronunciation will be used if available,
       otherwise the default rule pronunciation will be used as the single
       pronunciation, regardless of the setting of aIncludeRulePron.  If
       aMaxPronunciations is greater than one, then multiple pronunciations
       for a word are allowed, and if aIncludeRulePron is non-zero, then the
       default rule pronunciation will be the first pronunciation used.  As
       a special case, if aMaxPronunciations is set to zero, then only the
       the default rule pronunciation is used, regardless of the value of
       aIncludeRulePron.  If aAutoSilence is non-zero, then TIesrFlex will
       insert optional silence between each word, and at the grammar beginning
       and ending.  If aAutoSilence is zero, then no silence model is automatically
       added.
       
       The user may manually insert silence using the special words
       "_SIL*" where * represents a sequence of alphabetic characters, - , or _.
       (Note: case is important for this special word! It must be "_SIL*", 
       not "_Sil*" or "_sil*", etc. )  Multiple silence words can be defined,
       such as "_SIL_A" or _SIL_B".  These will all be interpreted as the
       _SIL word.  This can be advantageous in tagging when different phrases
       are recognized.
       ----------------------------------------------------------------*/
      Errors ParseGrammar( const char * cfg_string,
      unsigned int aMaxPronunciations = 1,
      int aIncludeRulePron = 0,
      int aAutoSilence = 1 );
      
      /*----------------------------------------------------------------
       DownloadGrammar
      
       This function is now just a placeholder for downloading a grammar
       to a DSP for recognition.  It is not used at this time.
       ----------------------------------------------------------------*/
      Errors DownloadGrammar();
   

      /*----------------------------------------------------------------
       OutputGrammar

       Output HMMs and grammar network to a set of files in a directory.
       You may choose to output byte HMM mean or variance vectors, if the
       loaded HMM data contained short word mean and variance data.
       ----------------------------------------------------------------*/
      Errors OutputGrammar( char *aDirName,
              int aByteMeans=1, int aByteVars=1, int aOptimize = 0  );


      enum ErrorCodes { OK, FAIL };
      enum ExceptionCodes { STATUS_FAIL_PARSE = 0xE0000001 };


   private:


      // The one and only special "word" for placing the silence model
      // manually within the grammar.
      static char const *const SIL;


      // Private constants used in the processing of a grammar within
      // the TIesrFlex class.  It would be better to be able to
      // specify and allocate sizes dynamically.


      // Increased values to expand capabilities for SIND

      // Now implemented to dynamically allocate TOKEN_ALLOC char* pointers
      // at a time.  Tokens are no longer kept in a static array of words,
      // but are dynamically stored by malloc within yylex.
      //static const int MAX_TOKEN = 1024;
      static const int TOKEN_ALLOC = 20;

      // Maximum nodes in the grammar
      // static const int  MAX_NODE = 1024;
      // Now implemented to dynamically allocate NODE_ALLOC node structures
      // at a time
      static int const NODE_ALLOC = 20;

      // Maximum number of rules in the grammar
      // static const int  MAX_RULE =  128;
      // Now implemented to dynamically allocate RULE_ALLOC rule structures
      // at a time
      static const int RULE_ALLOC = 5;

      // Now implemented to dynamically allocate SENTNODE_ALLOC
      // Sentnode structures at a time.  Each Sentnode structure has
      // an array that specifies which Sentnodes the Sentnode may next
      // transition to.  This is also dynamically allocated now by
      // expanding the array by SENTNODENEXT_ALLOC entries at a time.
      // static const int MAX_SENTNODE = 1024;
      static const int SENTNODE_ALLOC = 20;
      static const int SENTNODENEXT_ALLOC = 10;

      // Now implemented to dynamically allocate WORD_ALLOC word array elements
      // at a time.
      // static const int MAX_WORD = 1024;
      static const int WORD_ALLOC = 20;

      // Maximum number of characters in a word, including +N and \0
      static const int MAX_WORDLEN = 32;

      typedef char Word_t[MAX_WORDLEN];

      // Now implemented to dynamically allocate SENTTRAN_ALLOC structures
      // at a time.
      // static const int MAX_SENTTRAN = 1024;
      static const int SENTTRAN_ALLOC = 20;

      // Now implemented to dynamically allocate SENTTRAN_ALLOC structures
      // at a time.
      // static const int MAX_DEPEND = 32;
      static const int DEPEND_ALLOC = 10;

      // Now implemented to dynamically allocate HMM_ALLOC structures
      // at a time.
      // static const int MAX_HMM = 4096;
      static int const HMM_ALLOC = 50;

      // context-expanded HMM
      // static const int MAX_SYM = 1024;
      // static const int MAX_SYM = 5000;
      // Now implemented to dynamically allocate SYM_ALLOC structures
      // at a time.
      static int const SYM_ALLOC = 200;

      // Next lists withing Sentnode and Symbol structures are now
      // dynamically allocated
      // static const int  MAX_NEXT = 256;
      static int const NEXT_ALLOC = 5;
            
      // string lengths - no longer used
      // static const int MAX_STR = 256;

      // specific code value used in grammar parsing
      static const int MCODE = 65534;

      // used for tree processing
      // static const int MAX_PHONE = 100;

      // Maximum number of phones in a word + 1 (for number of phones) 
      static const int MAX_PRON = 32;

      // Treenode structure no longer used
      // static const int MAX_TREENODE = 270;

      // used for gtm clustering
      // static const int  MAX_STATE = 10;

      // Now dynamic allocation makes these parameters not necessary
      // static const int  MAX_MEAN = 10000; //32767;  depends on modal, actually 7000 is enough
      // static const int  MAX_VAR = 10000; //32767;  actually 500 is enough
      // static const int  MAX_TRAN = 1024;
      // static const int  MAX_CLUSTER = 10000; //16384; 7000 is enough


      // Private structures utilized within the TIesrFlex class
      // internal functions.

      typedef char Pron_t[MAX_PRON];

      typedef struct gnod
      {
         char   *name;
         int    next;
         int    paranext;
         int    flag;
      } Gnode;

      
      typedef struct grul
      {
         char  *name;
         int   node;
      } Grule;
      
      
      typedef struct snttrn
      {
         int   src;
         int   dst;
         int   type;
         int   next;
      } Senttran;


      typedef struct sntnd
      {
         int    node;              /* Grammar node index corresponding to this Sentnode */
         int    tran;              /* 1 based, 0 means NULL */
         int    stop;              /* stop node? */
         int    flag;              /* to support GMHMM net format */
         int    n_next;
         int    *next;
         int    nextMax;
      } Sentnode;


      typedef struct symnd
      {
         int   hmm;               /* sym map to hmm, many to one */
         int   word;              /* word end sym has a word index, otherwise MCODE */
         int   n_next;            /* # of sym that follows this sym */
         int   *next;
         int   nextMax;
      } Symnode;


      typedef struct hmmtb
      {            /* hmm's center, left, right monophone index */
         int   c;
         int   l;
         int   r;
      } Hmmtbl;
      

      typedef struct dpnd
      {
         int   referenced;
         int   referencing;
      } Depend;


      // Treenode structure no longer used.  Decision tree parsing now
      // is done directly using encoded tree data in memory.
      // These structures are used within the question list processing
      // to develop the triphones.  Eventually, they should be moved
      // to a separate question tree processing class.
      /*
      typedef struct tree_node
      {
	    int   qs_frame;
	    struct tree_node *yes;
	    struct tree_node *no;
      } Treenode;
      */

      /* model structure.  Model indices are zero based */
      typedef struct model_node
      {
         int c;      /* center model */
         int l;      /* left model */
         int r;      /* right model */
         int s;      /* state index, 0 1 2 3 ... */
      } Modelnode;

   
   private:

      // Private TIesrFlex implementing functions 


      // Dynamic structure array allocation support for parsing
      void* ExpandArray( void* aArray, int aItemSize, int aNumAlloc, int* aNumInArray );

      // file input - for device, data would be in ROM.
      // we would just want to get pointers to data
      int read_config( char *fname );
      int read_scale( char *fname );
      int read_sil( char *fname );
      int read_gconst( char *fname );
      int read_var( char *fname );
      int read_mean( char *fname );
      int read_fxcloff( char *fname );
      int read_fxclust( char *fname );
      int read_weight(char *fname); // incorporate  VQbased weight quantization method
      int read_fxtroff( char *fname );
      int read_fxtran( char *fname );
      int read_treetbl( char *fname );
      int read_tree( char *fname );
      int read_qstbl( char *fname );
      int read_qs( char *fname );
 
      //************************************************************************
      //added to facilitate mean quantization
      int makemean(char * dirname);
   
      //********************************************************************
      // If codebook data loaded to memory, this will remove allocation
      void free_codebook();

      // priviate functions to implement grammar/model creation

      // initialize a grammar parse
      void init();
   
      // The yacc parser - I actually used GNU 'bison' to derive the C code
      int yyparse();
      void __yy_memcpy (char *to, char *from, unsigned int count);

      //int yylex(int *yylval);
      int yylex(long *yylval);

      void yyerror(const char *msg);
      int yywrap();
      
      // support functions
      int output_int16( FILE *fp, int i32 );
      int output_sym( FILE * fp, int hmm_code, int word_code, int n_next, int *next );
      int download_gtm();
      int download_hmm2phone();
      int download_net();
      void process_grammar();
      void check_one_start_rule();
      void check_cyclic_grammar();
      void check_unreferenced();
      void create_dependency_list();
      void create_hmm();
      void add_hmm_entry(int c, int l, int r);
      int find_hmm_idx(int c, int l, int r);
      void expand_sym_net(int  n_next, int  *next, char *buf, int  phone_or_word);
      void construct_sym_name(char *buf, int sym_code, int phone_or_word );
      int is_end_sym( int sym_code );
      void create_sym_net( int aSentNode );
      int create_sym(int hmm_code, int word_code, int n_next, int *next);

#ifdef TIESRFLEXDEBUG
      void expand_fsa2(int aSentNode, char *buf);
      void expand( int aNode, char *buf);
#endif

      void create_all_trans( int aNode );
      void connect_all_trans();
      void expand_all_trans( int aNode );
      void skip_empty_node( int src, int aNode );
      void create_tran(int src, int dst, int type, int next);
      int find_word_idx( char *s );
      int find_sentnode_idx( int aNode );
      void check_parallel_node( int aNode );
      void not_in_sentnode_list( int aNode );
      void create_word_list( int aNode );
      void add_word( const char *s );
      void create_sentnode_list( int aNode );
      void traverse_and_add_sentnode( int aNode );
      void add_sentnode( int aNode );
      int substitute_nonterm( int aNode, int aRule );
      int referenced(char *name, int aNode );
      int referenced_sub(char *name, int aNode );
      int referencing( int idx );
      void replace_old_with_new( int aOldNode, int aNewNode );
      int create_rule( char *nonterm, int aNode );
      int duplicate_node_net( int aNode );
      int duplicate_node_net_copy( int aNode );
      void clear_sentnode_visit_flag();
      void clear_visit_flag();

      // Clear visit flags for a node network starting at aNode
      void ClearNodeNetFlags( int aNode );

      void attach_at_para_end( int aNode1, int aNode2 );
      void attach_at_para_end_tree( int aNode1, int aNode2 );
      void attach_at_end( int aNode1, int aNode2 );
      void attach_at_end_sub( int aNode1, int aNode2 );
      int create_node(char *name, int aNextNode, int aParaNextNode, int flag);
      int process_optional( int aNode );
      int output_gtm( char *dirname );
      int output_hmm2phone( char *dirname );
      int output_net( char *dirname, char* filename );
      int process_one_cluster( int idx);
      int find_idx_idx( int *list, int *n_list, int max, int idx, int *new_idx);
      int process_one_hmm( int n_state, int idx, short *cluster );

      /*
       No longer used, since tree parsing occurs directly using encoded
       decision tree data loaded in memory.
       int classify( Treenode *node, Modelnode *model, int *phnidx );
       int create_tree_node( int is_leaf, int value, Treenode **pptnode );
       int build_tree( Treenode **pptnode );
       */

      int get_cluster_idx( int c, int l, int r, int s, int gender, int *index );
      int yes_to_question( Modelnode const* model, int qs );
      void demand( int condition, char *msg );
 
      // functions to convert from 16-bit to 8-bit features
      int model_bit_16_to_8( char *source_dir, char *dest_dir, short endien, short add_mean_vec, 
			     int aByteMeans, int aByteVars);
      short load_n_mfcc( char *fname );
      short * load_16( char *fname, short n_mfcc, short *nbr);
      void add_mean(short *vec, int n_mfcc, int nbr);
      int output_obs(short obs[], short size, int n_dim, int n_mfcc, short max_scale[], char fname[]);
      void calc_max(short mu[], int nbr, int n_dim, int n_mfcc, short max_mu[]);
      void scale_max(short max_mu[], int n_mfcc, int n_dim);
      int write_vec(short *vec, short nbr, FILE *fp);
      int output_word( short value, FILE *fp );
      void vector_packing(short v_in[], unsigned short v_out[], short max_scale[], short D);
      short nbr_coding(short o_mu, short max_mu, int prt);
      int save_scales(char *fname, short scale[], short n_dim);

      // Handling for byte/word feature outputs
      int load_scales(char *fname, short scale[], short n_dim);
      int output_feature_vector( FILE* fp, void *aFeatures, 
				 unsigned int aIndex, int aByteFeatureVector );
      int output_config( const char* aDirName, short aByteMeans, short aByteVars );


      // Functions for implementing multiple pronunciations

      // This function adds multiple pronunciations to the fsa2 grammar
      // network created by yyparse and process_grammar.  The multiple
      // pronunciations are determined by the dictionary.  Class member
      // variables define if the default rule pronunciation is a valid
      // pronunciation, and specify the maximum number of pronunciations
      // allowed for a word.
      Errors AddMultiplePronunciations( );


      // This function looks up the pronunciation of the first, that is,
      // the base word, for each word in the grammar.  A
      // word may be a simple character string, or it may be a character
      // string with a suffix "+N" indicating that this is one of several
      // multiple pronunciations desired.  A base word will not have the
      // "+N" suffix. The function uses class member variables to determine
      // how to interpret the word and puts the pronunciation for the word in aPron.
      // The reason for distinguishing between base words and multi-pronunciation
      // words is that the lookup of pronunciations is faster if the
      // pronunciation of the base word is looked up followed immediately
      // by looking up pronunciations of all multi-pronunciations of the same word.
      Errors LookupBasePronunciation( const char* aWord, char* aPron );


      // This function looks up the pronunciation of multi-pronunciation
      // words following the lookup of the base word pronunciation. This function
      // must only be called after calling LookupBasePronunciation for the base
      // word.   Multipronunciation words for a base word consist of the 
      // character string for the base word with an additional suffix
      // string "+N" indicating that this is one of several
      // multiple pronunciations for the base word.  This function should be
      // called for each N in order, following the lookup of the base pronunciation
      // word.  If the multi-word does not match the base word, this function
      // returns with a status indicating the mismatch.  Also, if the "+N" is out
      // of order, this function will return status indicating the lack of order.
      Errors LookupMultiPronunciation( const char* aWord, const char* aMultiWord, char* aPron );


      // Helper function that frees data structures created during parse and model output.
      void FreeGrammarData( void );

      // Helper function that adds integers to an integer list
      void AddNext( int **aList, int *aNumUsed, int *aNumMax, int aNumToAdd, int* aAddList );


      // New functions for processing the acoustic decision trees.
      // These functions implement processing the decision tree
      // directly without constructing a tree structure in memory.
      int ParseTree( Modelnode const* aModel, short const* aTopTreeNode, 
		     int* aClusterIndex );

      short const* SkipToNoNode( short const* aYesTreeNode );

      short const* DescendQuestionNode( short const* aQuestionNode );


      // WCE compatibility functions
      int myisalpha( int c);
      void rewind( FILE *fp );

#ifdef OFFLINE_CLS
      short LoadVQHMM(const char* dirname);
      short SaveVQHMM(const char* dirname);
      void  FreeVQHMM();
#endif
  
   private:

      // class variables

      unsigned int m_parseFail;
   
      // TIesrDict object pointer used to determine phonetic
      // pronunciation of words in grammar.  The CTIesrFlex class
      // contains a CTIesrDict class via this pointer.
      CTIesrDict *m_dict;

      // Output format should be little endian
      int little_endian;

      // Number of monophones in dictionary
      int n_phone;
   
   
      // Variables defining configuration of input acoustic decision
      // tree and HMM information.  This should be normally obtained 
      // from the fxconfig.bin file.

      // Flag, input data is monophone with no acoustic decision trees
      int m_bInputMonophone;

      // Number of total features (static + delta + acc)
      int m_inputFeatDim;

      // Number of static features
      int m_inputStaticDim;

      // Flag, input feature mean elements are in bytes
      int m_bInputByteMeans;

      // Flag, input feature inverse variance elements are in bytes
      int m_bInputByteVars;
      
      //Flag, input weight vectros are in bytes
      int m_bInputByteWgts;

          //Flag, the type of vector configureation that is used
      int m_bInputWeightConfig;

          //flag ,enable byte packing mode in fxclust.cb
          //the wt_index and the number of mixtures
      int m_bPackWtNmix;
      
      // Flag, input mean vectors are mean normalized and needs mean added
      int m_bInputCMN;
   
      // Flag, data is in little endian format
      int m_bInputLittleEndian;

      // Number of "genders" or hmm model sets in data
      int m_inputSets;

      // Flag, gender-dependent transitions
      int m_bInputGDTrans;
   
      // Flag, gender-dependent trees
      int m_bInputGDTrees;
      


      // Context sensitive phonetic HMM database information
      // Would be pointers to locations in Flash in embedded device
      // Loaded from files in this implementation.
      char * qs_base;
      int * qs_offset;
      short * tree_base;
      int * treetbl_base;

      short * tran_base;
      int * tranoffset_base;
      int m_numTrans;
      int m_maxStates;

      short * weight_base;
      int m_numWeights;
      short m_lengthWtVec;
      
      void * mean_base;
      int   m_numMeans;
        
      void * var_base;
      int  m_numVars;

      short * gconst_base;

      short * scale_base;

      short * cluster_base;

      int * clusteroffset_base;
      int m_numClusters;

      short sil_n_state;
      short * sil_hmm;
      
      // variables and static constant tables for grammar yy parsing
      // The grammar parser is derived from yacc and lex source code
      // and is used within the CGrammar class.
      
      char * cfg_char;
      char * cfg;
     
      static const short yycheck[];
      static const short yytable[];
      static const short yypgoto[];
      static const short yydefgoto[];
      static const short yydefact[];
      static const short yyr2[];
      static const short yyr1[];
      static const char yytranslate[];
      static const short yypact[];
   
   
      // variables for grammar creation
      int hmm_idx;
      int sym_idx;
      int word_idx;
      int depend_idx_save;
      int depend_idx;
      int senttran_idx;
      int sentnode_idx;
      int rule_idx;
      int node_idx;
      int n_stop_next;
      
      // Statically allocated array variables for grammar creation.
      // Could we create these on heap for duration of parsing,
      // then delete all except those necessary to download the
      // grammar?  It would be nice to keep parsed grammar around,
      // so it could be reloaded without parsing again, if desired.
      char start_name[ MAX_WORDLEN ];

      // Dynamically allocate words now.
      Word_t *word;
      int m_wordMax;

      
      // Token is now an array that dynamically keeps track of the 
      // tokens that need to be freed
      char** token;
      int token_idx;
      int m_tokenMax;

      // Dynamically allocate stop_next array now
      int *stop_next;
      int m_stopMax;
  
      // Dynamically allocate nodes now.
      // Gnode node[MAX_NODE];
      Gnode *node;
      int m_nodeMax;

      // Dynamically allocate rules now.
      // Grule rule[MAX_RULE];
      Grule *rule;
      int m_ruleMax;

      // Dynamically allocate sentnodes now.
      Sentnode *sentnode;
      int m_sentnodeMax;

      // Dynamically allocate senttran now.
      Senttran *senttran;
      int m_senttranMax;

      // Dynamically allocate dependencies now
      Depend *depend;
      int m_dependMax;

      // Dynamically allocated HMM table entries
      Hmmtbl *hmm;
      int m_hmmMax;

      // Dynamically allocated Symbols which are grammar context-expanded HMMs
      Symnode *sym;
      int m_symMax;

      // These are global variables that are used in output_gtm and
      // its related functions.  At some time in the future, these
      // variables should be dynamically created within output_gtm,
      // and deleted after use.  There is no reason to keep them
      // around when not outputting a grammar.

      // Now managed by dynamic allocation
      // int   mean_idx[ MAX_MEAN ];
      int   *mean_idx;
      int   mean_idx_cnt;
      int   *mean_idx_aug;

      // Now managed by dynamic allocation
      // int   var_idx[ MAX_VAR ];
      // int   *var_idx;
      int   var_idx_cnt;
      int   *var_idx_aug;


      // Now managed by dynamic allocation
      // int   cluster_idx[ MAX_CLUSTER ];
      // Now cluster_idx not used
      // int   *cluster_idx;
      // int   cluster_offset[ MAX_CLUSTER ];
      int  *cluster_offset;
      int   cluster_idx_cnt;
      int  *cluster_idx_aug;

      // Now managed by dynamic allocation
      // tran_idx array is not used
      // int   tran_idx[ MAX_TRAN ];
      // int   *tran_idx;
      // int   tran_offset[ MAX_TRAN ];
      int   *tran_offset;
      int   tran_idx_cnt;
      int   *tran_idx_aug;

      int   net_size;


#ifdef OFFLINE_CLS
      /* vector-quantized mean vector information */
      short *pROM2Cls; /* pointer to ROM to cluster mapping table */
      short vq_n_cs; /* number of clusters */
      short vq_nbr_dim; /* dimension of centroid */
      short *vq_centroid; /* centroid */
#endif

      FILE  *fp_hmm;
      FILE  *fp_mean;
      FILE  *fp_var;
      FILE  *fp_tran;
      FILE  *fp_pdf;
      FILE  *fp_mixture;
      FILE  *fp_gconst;

      // These are global variables associated with processing
      // output_gtm that are part of the tree processing.  

      // Treenode structure not used any longer, since parsing of the 
      // tree is performed directly on the encoded tree in memory.
      // tree structure
      // Treenode tree[ MAX_TREENODE ];

      // short count converted into short pointer
      // short *tree_offset;
      // int   cur_node;

      // Variable to hold autosilence option;
      int m_bAutoSilence;

      // Variables that define multiple pronunciation options that are in effect
      int m_bIncludeRulePron;
      unsigned int m_maxPronunciations;

      // word_idx prior to adding multiple pronunciations
      int m_baseWordIndex;

      // Variable to verify sequence of next multi-pronunciation word
      int m_multiCount;

      // Pointer to struct that holds pronunciations
      Pron_t* m_wordProns;

};


#endif // _TIESRFLEX_H
