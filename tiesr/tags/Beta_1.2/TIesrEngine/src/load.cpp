/*=======================================================================
 load.cpp

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 Load models from files. For embedded devices, one may wish to
 change this code to define pointers to the data in memory,
 rather than reading files.

======================================================================*/

/* Headers required by Windows OS */
#if defined (WIN32) || defined (WINCE)
#include <windows.h>
#endif

#include "tiesr_config.h"
#include "mfcc_f_user.h"
#include "pack_user.h"
#include "load_user.h"

//char *malloc( int );


/*--------------------------------*/
//
// MS WCE does not support rewind function
//
static void myrewind( FILE *fp )
{
   fseek(fp, 0L, SEEK_SET);
   clearerr(fp);
}


/*--------------------------------*/
TIESRENGINECOREAPI_API TIesrEngineStatusType load_scales( char *fname, gmhmm_type *gv )
{
   FILE  *fp;
   size_t nread;
   
   fp = fopen(fname, "rb");
   if (!fp)
   {
      PRT_ERR(fprintf(stderr, "can't open %s\n", fname));
      PRT_ERR(exit(0));
      return eTIesrEngineModelFileFail;
   }
   
   
   gv->scale_mu = (short *) malloc( sizeof(short) * gv->n_mfcc * 2 );
   if( gv->scale_mu == NULL )
   {
      fclose(fp);
      return eTIesrEngineModelFileFail;
   }
   
   
   nread = fread(gv->scale_mu, sizeof(short), gv->n_mfcc*2, fp);
   if( nread != (size_t)gv->n_mfcc*2 )
   {
      free(gv->scale_mu);
      gv->scale_mu = NULL;
      fclose(fp);
      return eTIesrEngineModelFileFail;
   }
   
   
   gv->scale_var =(short *) malloc( sizeof(short) * gv->n_mfcc * 2 );
   if( gv->scale_var == NULL )
   {
      fclose(fp);
      return eTIesrEngineModelFileFail;
   }
   
   
   nread = fread( gv->scale_var, sizeof(short), gv->n_mfcc*2, fp);
   if( nread != (size_t)gv->n_mfcc*2 )
   {
      free(gv->scale_var);
      gv->scale_var = NULL;
      fclose(fp);
      return eTIesrEngineModelFileFail;
   }
   
   
   fclose( fp );
   return eTIesrEngineSuccess;
}


/*--------------------------------*/
/*
 ** return 0 if file open fail
 */
TIESRENGINECOREAPI_API unsigned short load_n_mfcc( char *fname )
{
   FILE  *fp;
   unsigned short n_mfcc = 0;
   
   fp = fopen(fname, "rb");
   if (!fp)
   {
      PRT_ERR(fprintf(stderr, "can't open %s\n", fname));
      return n_mfcc;
   }
   
   fread(&n_mfcc, sizeof(unsigned short), 1, fp);
   fclose( fp );
   return n_mfcc;
}



/*----------------------------------------------------------------*/
/*
 ** unpack mean or variance vectors
 */

TIesrEngineStatusType unpacking(short *base, ushort nbr, short n_mfcc,
FILE *fp, short scale[])
{
   int i;
   unsigned short tmp_sh[ MAX_DIM_MFCC16 ];
   short *vec;
   size_t nread;
   for (vec = base, i=0; i< nbr; i++, vec += n_mfcc * 2)
   {
      nread = fread(tmp_sh, sizeof(short), n_mfcc, fp);
      if( nread != (size_t)n_mfcc )
         return eTIesrEngineModelLoadFail;
      vector_unpacking(tmp_sh, vec, scale, n_mfcc);
   }
   //printf("----------n unpaced: %d\n", nbr);
   return eTIesrEngineSuccess;
}


/* ---------------------------------------------------------------------------
 load mu
 ---------------------------------------------------------------------- */
short *load_mu( char *fname,  gmhmm_type *gv, fileConfiguration_t*  fileConfig )
{
   FILE *fp;
   unsigned int vec_size = gv->n_mfcc;
   short *mu; /* array of mean vectors  */
   size_t nread;
   
   
   /* TIesr Engine wants mean vectors as interleaved static/dynamic
    features in bytes if BIT8MEAN set.  Otherwise, it wants the
    vectors as non-interleaved shorts.  vec_size is the number of
    shorts to allocate per vector, assuming a short is two bytes. */
   #ifndef BIT8MEAN
   /* TIesr Engine will store means as shorts */
   vec_size *= 2;
   #endif
   
   
   fp = fopen(fname, "rb");
   if( !fp )
      return NULL;
   
   
   /* Read number of mean vectors */
   nread = fread(&(gv->n_mu), sizeof(unsigned short), 1, fp);
   if( nread != 1 )
   {
      fclose( fp );
      return NULL;
   }
   
   
   
   /* Allocate space for mean vectors in either short or byte depending
    on the setting of BIT8MEAN */
   mu = (short *) malloc( gv->n_mu * vec_size * sizeof(short) );
   if( !mu )
   {
      fclose(fp);
      return NULL;
   }
   
   
   /* TIesr Engine wants mean vectors as interleaved static/dynamic
    features in bytes if BIT8MEAN set */
   #ifdef BIT8MEAN
   
   if( fileConfig->byteMu )
   {
      /* file mu is already in proper format, read in directly */
      nread = fread(mu, sizeof(short), gv->n_mu * vec_size, fp);
      fclose( fp );
      if( nread != gv->n_mu*vec_size )
      {
         free( mu );
         return NULL;
      }
   }
   else
   {
      /* The file mu data is in non-interleaved shorts.  TIesr Engine
       does not now have the code to convert from short to
       interleaved byte format.  It could be put in TIesr Engine.
       See TIesrFlex for the conversion code.  If that is done,
       then a call will be put here to convert. */
      fclose( fp );
      free( mu );
      return NULL;
   }
   
   
   /* TIesr Engine wants mean vectors as non-interleaved short features
    with static features first followed by dynamic features if
    BIT8MEAN is not set. */
   #else
   
   if( fileConfig->byteMu )
   {
      /* Input data is in interleaved static/dyanamic bytes.  Unpack these
       into non-interleaved short vectors */
      TIesrEngineStatusType status;
      status = unpacking(mu, gv->n_mu, gv->n_mfcc, fp, gv->scale_mu);
      fclose( fp );
      if( status != eTIesrEngineSuccess )
      {
         free( mu );
         return NULL;
      }
      
   }
   
   else
   {
      /* Input data is already in non-interleaved short features,
       read directly */
      nread = fread(mu, sizeof(short), gv->n_mu * vec_size, fp);
      fclose( fp );
      if( nread != gv->n_mu*vec_size )
      {
         free(mu);
         return NULL;
      }
   }
   
   #endif
   
   /* The loading was a success, return the mean vector location. */
   return mu;
}


/* ---------------------------------------------------------------------------
 load var
 ---------------------------------------------------------------------- */
static short *load_var( char *fname, gmhmm_type *gv, fileConfiguration_t *fileConfig )
{
   FILE               *fp;
   short *base_var; /* inverse of variance */
   size_t nread;
   
   /* Open binary inverse variances file or failure */
   fp = fopen(fname, "rb");
   if( !fp )
   {
      return NULL;
   }
   
   
   /* Load number of inverse variance vectors */
   nread = fread(&(gv->n_var), sizeof(unsigned short), 1, fp);
   if( nread != 1 )
   {
      fclose( fp );
      return NULL;
   }
   
   
   /* TIesr Engine wants inv variance vectors as interleaved static/dynamic
    features in bytes if BIT8VAR is set.  Otherwise, it wants the
    vectors as non-interleaved shorts.  Allocate space to hold variance
    based on the desired size, assuming short is two bytes. */
   #ifdef BIT8VAR
   base_var = (short *) malloc( gv->n_var * gv->n_mfcc * sizeof(short) );
   #else
   base_var = (short *) malloc( gv->n_var * 2 * gv->n_mfcc * sizeof(short) );
   #endif
   if( ! base_var )
   {
      fclose(fp);
      return NULL;
   }
   
   /* TIesr Engine wants inv variance vectors as interleaved static/dynamic
    features in bytes if BIT8VAR is set */
   #ifdef BIT8VAR
   
   if( fileConfig->byteVar )
   {
      /* Input file inv variance is already in byte format, read directly. */
      nread = fread(base_var, sizeof(short), gv->n_var * gv->n_mfcc, fp);
      fclose(fp);
      if( nread != (unsigned int)(gv->n_var*gv->n_mfcc) )
      {
         free(base_var);
         return NULL;
      }
   }
   else
   {
      /* The file inv variance data is in non-interleaved shorts.
       TIesr Engine does not now have the code to convert from short
       to interleaved byte format.  It could be put in TIesr Engine.
       See TIesrFlex for the conversion code.  If that is done, then
       a call will be put here to convert. */
      fclose( fp );
      free( base_var );
      return NULL;
   }
   
   
   /* TIesr Engine wants inv variance vectors as non-interleaved short
    features with static features first followed by dynamic features
    if BIT8VAR is not set. */
   #else
   
   if( fileConfig->byteVar )
   {
      /* Input data is in interleaved static/dyanamic bytes.  Unpack these
       into non-interleaved short vectors */
      TIesrEngineStatusType status;
      status = unpacking(base_var, gv->n_var, gv->n_mfcc, fp, gv->scale_var);
      fclose( fp );
      if( status != eTIesrEngineSuccess )
      {
         free( base_var );
         return NULL;
      }
   }
   
   else
   {
      /* Input data is already in non-interleaved short features,
       read directly */
      nread = fread(base_var, sizeof(short), gv->n_var * 2 * gv->n_mfcc, fp);
      fclose( fp );
      if( nread != (size_t)gv->n_var * 2 * gv->n_mfcc )
      {
         free(base_var);
         return NULL;
      }
   }
   
   #endif
   
   /* Loading of variance vectors was successful */
   return base_var;
}




/*--------------------------------*/
/* Testing only */
/*
 #define TESTIT
 */

#ifdef TESTIT
/*
 ** access to transition probabilities
 */
#define get_pi_gvn_trans(trans) ((short *)(trans+1))
#define nbr_states_gvn_trans(trans_ptr) (*(unsigned short *)(trans_ptr))
#define trans_size(trans_ptr) (nbr_states_gvn_trans(trans_ptr) *  nbr_states_gvn_trans(trans_ptr))

#define get_a_gvn_trans(trans)    (trans+nbr_states_gvn_trans(trans))
#define get_ai_gvn_trans(trans, i) ( get_a_gvn_trans(trans)+(nbr_states_gvn_trans(trans)*(i)))
#define get_aij_gvn_trans(trans, i, j) (get_ai_gvn_trans(trans, i)[j])

#define SCALE 2
#define MINV -32768

void trans_weight(short *trans_pool, unsigned short len)
{
   unsigned short n, nbr_stts, i, j;
   short *ptr;
   int  int_aij;
   
   for (n = 0, ptr = trans_pool; ptr < trans_pool + len; ptr += trans_size(ptr), n++)
   {
      nbr_stts = nbr_states_gvn_trans(ptr);
      printf("[%3d] %d\n", n, nbr_stts);
      /*     for (i=0; i<nbr_stts - 1; i++) printf("%5d ", get_pi_gvn_trans(ptr)[i]);  printf("\n"); */
      for (i=0; i<nbr_stts - 1; i++)
      {
         for (j=0; j<nbr_stts; j++)
         {
            printf("%5d ", get_aij_gvn_trans(ptr, i, j));
            int_aij =  SCALE * get_aij_gvn_trans(ptr, i, j);
            get_aij_gvn_trans(ptr, i, j) =  MAX(MINV, int_aij);
            printf("(%5d) ", get_aij_gvn_trans(ptr, i, j));
         }
         printf("\n");
      }
   }
}

#endif



/* ---------------------------------------------------------------------------
 load obs_scr
 ---------------------------------------------------------------------- */
/* -- used Gaussian level cache is enough. no need for pdf level

 static TIesrEngineStatusType
 load_obs_scr( char *fname, gmhmm_type *gv )
 {
 FILE               *fp;
 unsigned short     len;

 fp = fopen(fname, "rb");
 if( !fp )
 return eTIesrEngineModelLoadFail;

 fread(&len, sizeof(unsigned short), 1, fp);

 gv->obs_scr = (short *) malloc( len * sizeof(short) );
 if( ! gv->obs_scr )
 {
 fclose(fp);
 return eTIesrEngineModelLoadFail;
 }

 //   fread(obs_scr, sizeof(short), len, fp);


 fclose( fp );
 return eTIesrEngineSuccess;
 }
 */

/*--------------------------------*/
/*
 ** generic load
 */
TIESRENGINECOREAPI_API short *load_model_file(char fname[], unsigned short *len)
{
   FILE *fp;
   short *ptr;
   size_t nread;
   
   
   fp = fopen(fname, "rb");
   if( !fp )
      return NULL;
   
   fread(len, sizeof(unsigned short), 1, fp);
   
   ptr = (short *) malloc((*len) * sizeof(short) );
   if( !ptr )
   {
      fclose(fp);
      return NULL;
   }
   
   nread = fread(ptr, sizeof(short), *len, fp);
   fclose(fp);
   if( nread != *len )
   {
      free(ptr);
      return NULL;
   }
   
   return ( ptr );
}


/* ---------------------------------------------------------------------------
 load word list
 ---------------------------------------------------------------------- */
static TIesrEngineStatusType
load_word_list( char *fname, gmhmm_type *gv )
{
   FILE      *fp;
   short     i;
   int size;
   char *p_char;
   
   if ( gv->trans->n_word == 0)
      return eTIesrEngineSuccess;
   
   fp = fopen(fname, "r");
   if( !fp )
      return eTIesrEngineModelLoadFail;
   
   fseek(fp, 0, 2);
   
   size = ftell(fp); /* count */
   
   gv->vocabulary = (char **) malloc( gv->trans->n_word * sizeof(char *) );
   if( ! gv->vocabulary )
   {
      fclose(fp);
      return eTIesrEngineModelLoadFail;
   }
   
   p_char = (char *) malloc( size * sizeof(char));
   if( ! p_char )
   {
      free( gv->vocabulary );
      gv->vocabulary = NULL;
      fclose(fp);
      return eTIesrEngineModelLoadFail;
   }
   
   
   myrewind(fp);
   fread(p_char, sizeof(char), size, fp);
   fclose( fp );
   
   for (i = 0; i < gv->trans->n_word; i++)
   {
      gv->vocabulary[ i ] = p_char;
      while (*p_char != '\n') p_char++;
      *p_char = '\0'; /* replace line-feed by string end */
      p_char++;
   }
   
   return eTIesrEngineSuccess;
   
}


/*--------------------------------*/
TIESRENGINECOREAPI_API void free_scales(gmhmm_type *gv)
{
   if( gv->scale_mu )
      free(  gv->scale_mu );
   if( gv->scale_var )
      free(  gv->scale_var );
}


/*--------------------------------*/
TIESRENGINECOREAPI_API void free_models(gmhmm_type *gv)
{
   if( gv->base_net )
      free( gv->base_net );
   if( gv->base_hmms )
      free( gv->base_hmms );
   if( gv->base_mu )
      free( gv->base_mu );
   //
   if( gv->gauss_scr )
      free( gv->gauss_scr);
   if( gv->base_mu_orig )
      free( gv->base_mu_orig );
   if( gv->base_var )
      free( gv->base_var );
   #ifdef USE_SVA
   if( gv->base_var_orig )
      free( gv->base_var_orig );
   #endif
   if( gv->base_tran )
      free( gv->base_tran );
   //   if( gv->obs_scr )
   //      free( gv->obs_scr );
   if( gv->base_pdf )
      free( gv->base_pdf );
   if( gv->base_mixture )
      free( gv->base_mixture );
   if( gv->base_gconst )
      free( gv->base_gconst );
   
   free_scales(gv);
   
   if (gv->vocabulary)
   {
      if( gv->vocabulary[0] )
         free(gv->vocabulary[0]);
      free(gv->vocabulary);
   }
}


/*----------------------------------------------------------------
 load_model_config

 This function reads the configuration of the binary model data
 files.  In this way, the recognizer can adjust for various
 configurations of model data, such as means and/or variances in
 short or byte.

 ----------------------------------------------------------------*/
TIesrEngineStatusType load_model_config(char fname[],
fileConfiguration_t *fileConfig )
{
   FILE *fp;
   size_t nread;
   
   
   fp = fopen(fname, "rb");
   
   
   /* If configuration file is not available, assume the last assumption for
    creating models, that is, the models have byte mean and variance. */
   if( !fp )
   {
      fileConfig->byteMu = 1;
      fileConfig->byteVar = 1;
      
      return eTIesrEngineSuccess;
   }
   
   /* Read configuration parameters into configuration structure */
   nread = fread( &fileConfig->byteMu, sizeof(short), 1, fp);
   nread = fread( &fileConfig->byteVar, sizeof(short), 1, fp);
   fclose(fp);
   
   
   /* Check for enough data to read last parameter */
   if( nread != 1 )
      return eTIesrEngineModelLoadFail;
   
   return eTIesrEngineSuccess;
}

/* load mean vectors to either base mean for decoding or original mean for adaptation
 @param mod_dir pointer to model directory
 @param gv pointer to gmhmm_type
 @bLoadBaseMu TRUE if load to base mean, FALSE if load to original mean */
TIESRENGINECOREAPI_API
TIesrEngineStatusType load_mean_vec(char * mod_dir, gmhmm_type * gv ,
Boolean bLoadBaseMu )
{
   char fname[ MAX_STR ];
   TIesrEngineStatusType status;
   fileConfiguration_t fileConfig;
   
   /* Read the configuration of input file model data.  For now, if no
    configuration file is available, byte means and variances are
    assumed, since this was the last way we were assuming data was
    written to binary model files. */
   sprintf(fname, "%s/%s", mod_dir, file_names[11]);
   status = load_model_config( fname, &fileConfig );
   if( status != eTIesrEngineSuccess )
      return eTIesrEngineModelLoadFail;
   
   sprintf( fname, "%s/%s", mod_dir, file_names[3] );
   if (gv->base_mu == NULL && bLoadBaseMu)
   {
      gv->base_mu = load_mu( fname, gv, &fileConfig );
      if( ! gv->base_mu )
         return eTIesrEngineModelLoadFail;
   }
   
   /* load mu again for PMC */
   if (gv->base_mu_orig == NULL && bLoadBaseMu == FALSE)
   {
      gv->base_mu_orig = load_mu( fname, gv, &fileConfig );
      if( ! gv->base_mu_orig )
         return eTIesrEngineModelLoadFail;
   }
   return eTIesrEngineSuccess;
}


/*----------------------------------------------------------------*/
/*
 ** return the mfcc dimension of the feature, 0 if mfcc dimension file does not exist
 */
TIESRENGINECOREAPI_API
TIesrEngineStatusType load_models(char *mod_dir, gmhmm_type *gv, char bMonoNet,
char * network_file , char* word_list_file)
{
   char fname[ MAX_STR ];
   unsigned short len;
   TIesrEngineStatusType status;
   fileConfiguration_t fileConfig;
   
   
   /* Initialization of byte to short scale vectors */
   gv->scale_mu = NULL;
   gv->scale_var = NULL;
   
   /* read the dimension of mfcc */
   sprintf(fname, "%s/%s", mod_dir, file_names[9]);
   gv->n_mfcc = load_n_mfcc( fname );
   if (gv->n_mfcc == 0)
      return  eTIesrEngineModelLoadFail;
   
   
   /* Read the configuration of input file model data.  For now, if no
    configuration file is available, byte means and variances are
    assumed, since this was the last way we were assuming data was
    written to binary model files. */
   sprintf(fname, "%s/%s", mod_dir, file_names[11]);
   status = load_model_config( fname, &fileConfig );
   if( status != eTIesrEngineSuccess )
      return eTIesrEngineModelLoadFail;
   
   /* Read the scaling of mean and variance if mu.bin or var.bin in byte. */
   if( fileConfig.byteMu || fileConfig.byteVar )
   {
      sprintf(fname, "%s/%s", mod_dir, file_names[10]);
      status = load_scales( fname, gv );
      if( status != eTIesrEngineSuccess )
         return eTIesrEngineModelLoadFail;
   }
   else
   {
      gv->scale_mu = NULL;
      gv->scale_var = NULL;
   }
   
   /*
    ** read top level grammar
    */
   if (network_file)
   {
      gv->base_net =  load_model_file(network_file, &len);
      if( ! gv->base_net )
         return eTIesrEngineModelLoadFail;
      gv->trans = (TransType *) gv->base_net;
   }else
   {
      if (!bMonoNet)
      {
         sprintf(fname, "%s/%s", mod_dir, file_names[1]);
         gv->base_net =  load_model_file(fname, &len);
         if( ! gv->base_net )
            return eTIesrEngineModelLoadFail;
         gv->trans = (TransType *) gv->base_net;
      }else
      {
         gv->base_net= NULL;
         gv->trans = NULL;
      }
   }
   
   /* adjust address */
   /* C54 do it here, but cannot do it here for 32 bits CPU */
   
   /*
    ** load word list, after load_net()
    */
   if (word_list_file)
   {
      status = load_word_list( word_list_file, gv );
      if( status != eTIesrEngineSuccess )
         return eTIesrEngineModelLoadFail;
   }else if (!bMonoNet)
   {
      sprintf(fname, "%s/%s", mod_dir, file_names[0]);
      status = load_word_list( fname, gv );
      if( status != eTIesrEngineSuccess )
         return eTIesrEngineModelLoadFail;
   }else
   {
      gv->vocabulary = NULL;
   }
   
   /*
    ** read all HMM's used in the top level grammar
    */
   
   sprintf(fname, "%s/%s", mod_dir, file_names[2]);
   gv->base_hmms =  (unsigned short *)load_model_file(fname, &len);
   if( ! gv->base_hmms )
      return eTIesrEngineModelLoadFail;
   
   
   /* load mu, performing conversions as necessary */
   sprintf( fname, "%s/%s", mod_dir, file_names[3] );
   gv->base_mu = load_mu( fname, gv, &fileConfig );
   if( ! gv->base_mu )
      return eTIesrEngineModelLoadFail;
   
   // Gaussian cache
   gv->gauss_scr =(short *) calloc( gv->n_mu , sizeof(short) );
   if ( !gv->gauss_scr )
   {
      return eTIesrEngineModelLoadFail;
   }
   
   
   /* load original mu for PMC */
   #ifdef USE_16BITMEAN_DECOD
   gv->base_mu_orig = NULL;
   /*
    the following code has been commented. Loading the original mu has
    been a process in cluster-dependent JAC, inside function of
    rj_compensate(gmhmm_type * gv) */
   #else
   gv->base_mu_orig = load_mu( fname, gv, &fileConfig );
   if( ! gv->base_mu_orig )
      return eTIesrEngineModelLoadFail;
   #endif
   
   /* load inverse variance vectors  */
   sprintf( fname, "%s/%s", mod_dir, file_names[4] );
   gv->base_var = load_var( fname, gv, &fileConfig );
   if( ! gv->base_var )
      return eTIesrEngineModelLoadFail;
   
   #ifdef USE_SVA
   
   /* load var again for SVA */
   gv->base_var_orig = load_var( fname, gv, &fileConfig );
   if( ! gv->base_var_orig )
      return eTIesrEngineModelLoadFail;
   
   #endif
   
   
   /*
    ** transition matrix
    */
   sprintf(fname, "%s/%s", mod_dir, file_names[5]);
   gv->base_tran = load_model_file(fname, &len);
   if( ! gv->base_tran )
      return eTIesrEngineModelLoadFail;
   
   #ifdef TESTIT
   trans_weight(gv->base_tran, len);
   #endif
   
   /*
    ** PDF
    */
   sprintf(fname, "%s/%s", mod_dir, file_names[6]);
   gv->base_pdf = (unsigned short *)load_model_file(fname, &(gv->n_pdf));
   if( ! gv->base_pdf )
      return eTIesrEngineModelLoadFail;
   
   /*
    ** observation score buffer, only compute once per pdf per frame
    */
   /* Gaussian cache is better ,and this is redendant
    status = load_obs_scr( fname, gv );
    if( status != eTIesrEngineSuccess )
    return eTIesrEngineModelLoadFail;
    */
   
   /*
    ** mixture
    */
   sprintf(fname, "%s/%s", mod_dir, file_names[7]);
   gv->base_mixture = load_model_file(fname, &len);
   if( ! gv->base_mixture )
      return eTIesrEngineModelLoadFail;
   
   /*
    ** gconst
    */
   sprintf(fname, "%s/%s", mod_dir, file_names[8]);
   gv->base_gconst = load_model_file(fname, &len);
   if( ! gv->base_gconst )
      return eTIesrEngineModelLoadFail;
   
   return eTIesrEngineSuccess;
}

/*-----------------------------------------------------------
 mem_alloc

 Manage allocation of a memory area that is viewed as an
 array of short data.  This function has the capability to
 align data pointers within the memory to long, int, or short
 boundaries.  It assumes sizes of short, int, and long are
 powers of two.
 ------------------------------------------------------------*/
short *mem_alloc( short * base_mem, ushort * mem_count,
   ushort size_in_short, unsigned int max_mem_usage,
   unsigned short aAlign, char *mesg )
{
   unsigned int varLen;
   unsigned int shrtsPerVar;
   unsigned int shrtOffset;
   
   short *tmp =  base_mem + (*mem_count);
   
   
   /* Checks to ensure alignment of pointer is correct */
   if( aAlign == LONGALIGN || aAlign == INTALIGN )
   {
      varLen = (aAlign == LONGALIGN ) ?  sizeof(long) : sizeof(int);
      shrtsPerVar = varLen >> (SHORTLEN >> 1);
      shrtOffset = *mem_count & (shrtsPerVar - 1);

      /* Not aligned, align it to next short */
      if( shrtOffset )
      {
         tmp += shrtsPerVar - shrtOffset;
         *mem_count = (unsigned short)(tmp - base_mem);
      }
      
      /* Ensure request for exact number of shorts to fill all positions */
      shrtOffset = size_in_short & (shrtsPerVar-1);
      if( shrtOffset )
      {
         size_in_short += (shrtsPerVar - shrtOffset );
      }
   }
   
   (*mem_count) += size_in_short;
   PRT(printf("%5d words, %5d total/%d, in \"%s\"\n", size_in_short,
   *mem_count, max_mem_usage, mesg));
   
   if ((unsigned int) *mem_count > max_mem_usage)
   {
      PRT_ERR(printf("%5d words, %5d total/%d, in \"%s\"\n", size_in_short,
      *mem_count, max_mem_usage, mesg));
      PRT_ERR(fprintf(stderr, "maximum size capacity exceeded at \"%s\"\n", mesg));
      return NULL;
   }
   
   return tmp;
}

