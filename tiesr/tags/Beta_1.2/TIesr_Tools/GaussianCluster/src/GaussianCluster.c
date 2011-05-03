/*=======================================================================
 GaussianCluster.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
======================================================================*/

#include "ClusterUtil.h"

/* Global variables */
extern void FreeMatrix(Matrix m, int n_row);
extern void FreeVector(Vector v);
static double delta;
struct HMM_struct triphone[MaxGaussian];
struct cluster Cluster[MaxCluster];
struct cluster ReferenceCluster[MaxGaussian];
struct gaussian Gaussian[MaxGaussian]; 
int TotalGauss;
int TotalReference;
int TotalCDHMM;
int TotalCIHMM;

unsigned int nMean = 0;
unsigned int nVar = 0;
void *mean_base = NULL;
void *var_base = NULL;
short *p_mu_base = NULL;
short *p_var_base = NULL;
short *scale_base = NULL; /* the first m_inputFeatDim is form mean scaling,
			     the last m_inputFeatDim is for variance scaling*/
long ave_cov[ MAX_DIM_MFCC16 ];
float f_ave_cov[ MAX_DIM_MFCC16 ];

struct{
  int m_inputFeatDim ;

      // Static feature vector dimensions
  int m_inputStaticDim;

  // Features are in 16-bit shorts
  int m_bInputByteMeans;

  int m_bInputByteVars;

  // Features are CMN mean normalized, and need bias added.
  // Bias is added as part of TIesrFlex converting means
  // to bytes.
  int m_bInputCMN;

  // Features are not little endian
  int m_bInputLittleEndian ;

  // Male and female sets, denoted by :f and :m
  int m_inputSets ;

  // Monophone transitions are set ("gender") independent
  int m_bInputGDTrans ;

  // Acoustic decision trees are set ("gender") dependent
  int m_bInputGDTrees;
}st_config; 
 


/*--------------------------------
  read_config

  This function reads the configuration file that defines the type of
  information in the input fixed point tree and HMM files.
  Originally, Yuhung assumed one exact format for the files.  Now
  multiple types of files can be supported.  These file types allow
  options for much more compact data storage.
  --------------------------------*/
int read_config( char *fname )
{
   FILE *fp;
   size_t nread = 0;
   
   // Attempt to open the configuration file.  If it can't be opened,
   // use defaults.
   fp = fopen(fname, "rb");
   if ( fp == NULL ) 
   {
     // Default assumptions made by Yuhung for input trees and HMMs
     
     // Feature vector dimensions
     st_config.m_inputFeatDim = 20;
     
     // Static feature vector dimensions
     st_config.m_inputStaticDim = 10;
     
     // Features are in 16-bit shorts
     st_config.m_bInputByteMeans = FALSE;
     
     st_config.m_bInputByteVars = FALSE;
     
     // Features are CMN mean normalized, and need bias added.
     // Bias is added as part of TIesrFlex converting means
     // to bytes.
     st_config.m_bInputCMN = TRUE;
     
     // Features are not little endian
     st_config.m_bInputLittleEndian = FALSE;
     
     // Male and female sets, denoted by :f and :m
     st_config.m_inputSets = 1;

     // Monophone transitions are set ("gender") independent
     st_config.m_bInputGDTrans = FALSE;
     
     // Acoustic decision trees are set ("gender") dependent
     st_config.m_bInputGDTrees = TRUE;
     
     return OK;
   }
   

   // Number of features in input mean and variance vectors
   // This may include delta and accel. in addition to static
   fread( &st_config.m_inputFeatDim, sizeof(int), 1, fp );

   // Static feature dimensions in input mean and variance vectors
   // This is only the number of static features.  If delta and acc 
   // features are in the input, then delta and acc must also have
   // this number of dimensions.
   fread( &st_config.m_inputStaticDim, sizeof(int), 1, fp );

   // Flag, input feature means are in bytes
   fread( &st_config.m_bInputByteMeans, sizeof(int), 1, fp );

   // Flag, input feature inverse variances are in bytes
   fread( &st_config.m_bInputByteVars, sizeof(int), 1, fp );

   // Flag, data is mean normalized and needs mean added
   fread( &st_config.m_bInputCMN, sizeof(int), 1, fp );
   
   // Flag, data is in little endian format
   fread( &st_config.m_bInputLittleEndian, sizeof(int), 1, fp );

   // Number of "genders" or hmm model sets in data
   fread( &st_config.m_inputSets, sizeof(int), 1, fp );

   // Flag, gender-dependent transitions
   fread( &st_config.m_bInputGDTrans, sizeof(int), 1, fp );
   
   // Flag, gender-dependent trees
   nread = fread( &st_config.m_bInputGDTrees, sizeof(int), 1, fp );
   
   fclose( fp );

   if( nread != 1 )
   {
      return FAIL;
   }

   if (st_config.m_bInputByteMeans){
     printf(" -------------- GaussianCluster Error --------------\n");
     printf(" Cluster resolution not acceptable using ByteMean as\n");
     printf(" input for vector quantization.\n");
     printf(" Must use short mean vectors as input.\n");
     printf(" ----------------------------------------------------\n");
     return FAIL;
   }
   return OK;
}



void DECODE_STDY(short tmp_sh, short *mu_static, short *mu_dynamic){
    *mu_static  = (tmp_sh & 0xff00);
    *mu_dynamic = (tmp_sh) << 8;
}

void vector_unpacking(const unsigned short v_in[], short v_out[], const short scale[], short D)
{
  short d;

  for (d = 0; d < D ; d++) {
    DECODE_STDY(v_in[d], &v_out[d], &v_out[d + D]);
    v_out[d] >>= scale[d];
    v_out[d+D] >>= scale[d+D];
  }
}

void unpacking(short *base, unsigned short nbr, short n_mfcc,
	       FILE *fp, short scale[])
{
  int i;
  unsigned short tmp_sh[ MAX_DIM_MFCC16 ];
  short *vec;
  short nread;
  for (vec = base, i=0; i< nbr; i++, vec += n_mfcc * 2) {
    nread = fread(tmp_sh, sizeof(short), n_mfcc, fp);
    if( nread != n_mfcc )
      return ; 
    vector_unpacking(tmp_sh, vec, scale, n_mfcc);
  }
}

int read_scale( char *fname )
{
   FILE *fp;
   size_t nread = 0;
   size_t scaleSize;


   // The number of scales should be twice the number of features,
   // since we are getting the scales for mean and variance.  The
   // scales are in signed short.
   scaleSize =  2 * st_config.m_inputFeatDim * sizeof(short);
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

   nread = fread( scale_base, sizeof(short), 2*st_config.m_inputFeatDim, fp );
   fclose( fp );

   if( nread != (size_t)(2*st_config.m_inputFeatDim) )
   {
      return FAIL;
   }

   return OK;
}


void add_mean(short * vec, int n_mfcc){
  short d; 
  short *pv = vec; 
  long tmp; 
  for (d=0; d<n_mfcc; d++) {
    tmp = (long) pv[d] + (long) mean_bias[d];
    pv[d] = LONG2SHORT(tmp);
  }
}

int read_mean(char *fname)
{
   FILE  *fp;
   size_t elementSize;
   short vectorSize;
   unsigned int nRead;
   unsigned int i; 
   short * mu8, *mu16; 

   // if mean data exists, free it
   if( mean_base != NULL )
      free( mean_base );
	
   // Determine sizes of input and output vectors
   elementSize = st_config.m_bInputByteMeans ? sizeof( unsigned char ) :
      sizeof( short );
   
   vectorSize = elementSize * st_config.m_inputFeatDim;
	
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
   nRead = fread( mean_base, vectorSize, nMean, fp );

   fclose( fp );

   if (st_config.m_bInputByteMeans){
     p_mu_base = (short*)malloc( sizeof(short) * nMean * st_config.m_inputFeatDim ); 
     mu8= (short*)mean_base; mu16 = p_mu_base; 
     for (i=0;i<nMean;i++){
       vector_unpacking((unsigned short*) mu8, mu16, scale_base, st_config.m_inputStaticDim); 
       mu8 += st_config.m_inputStaticDim;
       mu16 += st_config.m_inputFeatDim; 
     }
     
     free(mean_base); 
   }else p_mu_base = (short*) mean_base ; 

   if (st_config.m_bInputCMN){
     /* need add bias */
     mu16 = p_mu_base ; 
     for (i=0;i<nMean;i++){
       add_mean(mu16, st_config.m_inputStaticDim);
       mu16 += st_config.m_inputFeatDim ; 
     }
   }

   return OK;
}

void convert_to_short_var()
{
  short *scale_var_base = &scale_base[st_config.m_inputFeatDim];
  short *var8, *var16;
  unsigned int i;

  if (st_config.m_bInputByteVars){
    p_var_base = (short*)malloc( sizeof(short) * 
				 nVar * st_config.m_inputFeatDim ); 
    var8= (short*)var_base; var16 = p_var_base; 
    for (i=0;i<nVar;i++){
      vector_unpacking((unsigned short*) var8, var16, scale_var_base, st_config.m_inputStaticDim); 
      var8 += st_config.m_inputStaticDim;
      var16 += st_config.m_inputFeatDim; 
    }
     
    free(var_base); 
   }else p_var_base = (short*) var_base ; 

}

void get_global_var()
{
  int i, j; 
  short *inv = p_var_base; 

  for (j=st_config.m_inputFeatDim-1;j>=0;j--) f_ave_cov[j] = 0;
  for (i=0;i<nVar;i++){
    for (j=st_config.m_inputFeatDim-1;j>=0;j--){
      f_ave_cov[j] += Q9/((float)inv[j]); /* Q6 */
    }
    inv += st_config.m_inputFeatDim; 
  }
  for (j=st_config.m_inputFeatDim-1;j>=0;j--){
    f_ave_cov[j] /= nVar; 
  }

}

int read_var(char *fname)
{
   FILE  *fp;
   size_t elementSize;
   int vectorSize;

   // if inverse variance data exists, free it
   if( var_base != NULL )
      free( var_base );

   // Determine sizes of input and output vectors
   elementSize = st_config.m_bInputByteVars ? sizeof( unsigned char ) :
      sizeof( short );
   
   vectorSize = elementSize * st_config.m_inputFeatDim;
	
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

   convert_to_short_var();
   get_global_var();

   return OK;
}

void save_var(char *fname)
{
  FILE *fp;
  short i; 

  fp = fopen(fname, "wt");
  for(i=0;i<st_config.m_inputFeatDim;i++)
    fprintf(fp, "%2.4f\n", f_ave_cov[i]);
  fclose(fp);

}

int main(int argc,char *argv[]){
  FILE *f_triphone,*f_monophone,*f_variance;
  unsigned int i;
  int j;
  int temp,counter=0;
  int phone_index_previous=0,state_index_previous=0,mixture_index_previous=0;
  int phone_index_follow=0,state_index_follow=0,mixture_index_follow=0;
  int Number_classes;
  float gg;
  char fname[256]; 
  short bSwap = 0;
  int islittleendian;

  if(argc!=7)
    {
      printf("GaussianCluster Path SavedMeanFile SavedVarFile SavedInfo NumClusters LittleEndian\n");
      printf(" Path          : Directory containing fixed point models and fxconfig.bin\n");
      printf(" SavedMeanFile : saved ASCII Mean vectors in Q11 for training\n");
      printf(" SavedVarFile  : saved ASCII variance vector in Q0\n");
      printf(" SavedInfo     : saved auxiliary information - not currently used\n");
      printf(" Num_clusters  : number of clusters, e.g., 128 \n");
      printf(" LittleEndian  : 1/0 : 1 => output Little Endian, 0 => output Big Endian\n");
      exit(-1);
    }


  /* determine endianness of the machine, and if byte swap on output needed. */
  i = 0x1;
  islittleendian =  *( (char*)&i ) == 0x1;
  bSwap = islittleendian != atoi( argv[6]);

  /* read the fixed-point configuration */
  strcpy(fname, argv[1]);
  strcat(fname, "/fxconfig.bin");
  if (read_config(fname) == FAIL) exit(1); 

  if (st_config.m_bInputLittleEndian != islittleendian ) {
    printf("GaussianCluster: fixed point data and machine endianness conflict\n");
    exit(1);
  }

  /* read the scaling vector */
  strcpy(fname, argv[1]);
  strcat(fname, "/scale.bin");
  read_scale(fname);

  /* read the fixed-point mean */
  strcpy(fname, argv[1]);
  strcat(fname, "/fxmean.cb");
  if (read_mean(fname)){
    /* the data is before TIesrFlex */
    f_triphone = fopen(argv[2], "wt");
    for (i=0;i<nMean;i++){
      fprintf(f_triphone, "%d %d %d ", i, 0, 0);
      for (j=0;j<st_config.m_inputFeatDim;j++) 
	fprintf(f_triphone, "%d ", p_mu_base[i*st_config.m_inputFeatDim + j]); 
      fprintf(f_triphone , "\n");
    }
    fclose(f_triphone);
    
    free(p_mu_base);
  }else
    return -1; 

  /* read and write the fixed-point mean */
  strcpy(fname, argv[1]);
  strcat(fname, "/fxvar.cb");

  if (read_var(fname)) {
    /* save variance */
    save_var(argv[3]);
    free( p_var_base );
    if (scale_base) free(scale_base);
  }else
    return -1; 

  f_triphone=fopen(argv[2],"r");
  if(f_triphone==NULL){
    printf("GaussianCluster: Open f_triphone ERROR\n");
    exit(-1);
  }

  /*  bSwap = atoi(argv[6]); */
  f_variance=fopen(argv[3],"r");
  if( f_variance == NULL ){
      printf( "GaussianCluster: Open f_variance ERROR\n");
      exit(-1);
  }

  Number_classes=atoi(argv[5]);

  fscanf(f_triphone,"%d ",&phone_index_previous);
  while (!feof(f_triphone)){
    triphone[phone_index_previous].phone_name=phone_index_previous;
    fscanf(f_triphone,"%d %d",&state_index_previous,&mixture_index_previous);
    if (mixture_index_previous==0 && counter!=0)
      triphone[phone_index_follow].mix_no[state_index_follow]=(mixture_index_follow+1);
    if (mixture_index_previous==0 && state_index_previous==0 && counter!=0)
      triphone[phone_index_follow].state_no=(state_index_follow+1);
    for (i=0;i<st_config.m_inputFeatDim;i++)
      {
	fscanf(f_triphone,"%d ", &temp);		
	triphone[phone_index_previous].mean[state_index_previous][mixture_index_previous][i]=(double)(temp)/Q11;
	fscanf(f_variance,"%f ", &gg);
	triphone[phone_index_previous].var[state_index_previous][mixture_index_previous][i]=gg;
      }
    rewind(f_variance);
    counter++;
    
    phone_index_follow=phone_index_previous;
    state_index_follow=state_index_previous;
    mixture_index_follow=mixture_index_previous;
    fscanf(f_triphone,"%d ",&phone_index_previous);
  }
  triphone[phone_index_follow].mix_no[state_index_follow]=(mixture_index_follow+1);
  triphone[phone_index_follow].state_no=(state_index_follow+1);
  TotalCDHMM=phone_index_follow+1;
  TotalGauss=counter;
  printf("%d Gaussians\n",TotalGauss);
  fclose(f_variance);
  fclose(f_triphone);
  VQ(Number_classes,0, bSwap);

  f_monophone = fopen(argv[2], "r");
  if (f_monophone == NULL) {
    printf("GaussianCluster: Open f_monophone ERROR\n");
    exit(-1);
  }
  f_variance=fopen(argv[3],"r");
  if (f_variance == NULL) {
    printf("GaussianCluster: Open f_variance ERROR\n");
    exit(-1);
  }
  counter=0;
  fscanf(f_monophone,"%d ",&phone_index_previous);
  while (!feof(f_monophone)){
    triphone[phone_index_previous].phone_name=phone_index_previous;
    fscanf(f_monophone,"%d %d",&state_index_previous,&mixture_index_previous);
    if (mixture_index_previous==0 && counter!=0)
      triphone[phone_index_follow].mix_no[state_index_follow]=(mixture_index_follow+1);
    if (mixture_index_previous==0 && state_index_previous==0 && counter!=0)
      triphone[phone_index_follow].state_no=(state_index_follow+1);
    for (i=0;i<st_config.m_inputFeatDim;i++)
      {
	fscanf(f_monophone,"%d ", &temp);	
	triphone[phone_index_previous].mean[state_index_previous][mixture_index_previous][i]=(double)(temp)/Q11;
	fscanf(f_variance,"%f ", &gg);
	triphone[phone_index_previous].var[state_index_previous][mixture_index_previous][i]=gg;
	
      }
    rewind(f_variance);
    counter++;
    
    phone_index_follow=phone_index_previous;
    state_index_follow=state_index_previous;
    mixture_index_follow=mixture_index_previous;
    fscanf(f_monophone,"%d ",&phone_index_previous);
  }
  triphone[phone_index_follow].mix_no[state_index_follow]=(mixture_index_follow+1);
  triphone[phone_index_follow].state_no=(state_index_follow+1);
  TotalCIHMM=phone_index_follow+1;
  TotalReference=counter;
  printf("%d Gaussians\n",TotalReference);
  Classify(Number_classes,triphone,0);
  fclose(f_variance);
  fclose(f_monophone);

  exit(0);
}

void FreeClusterMemberTable(int clu)
{
   free(Cluster[clu].pdf_member);
}


int Min(double *D, int cs){
  
  int min_id,i;
  double min;
  
  min=D[0];
  min_id=0;
  for(i=1;i<cs;i++){
    
    if(D[i]<min){
      min=D[i];
      min_id=i;
    }
    
  }

  return min_id;
}

int * Ranking(double *D, int cs, int pick){
  int min_id,i,j;
  double min;
  int * rank;
  rank=(int *)calloc(cs, sizeof(int));
  min=D[0];
  min_id=0;
  for (j=0;j<pick;j++){
    for(i=0;i<cs;i++){	  
      if(D[i]<min){
	min=D[i];
	D[i]=D[i]+MaxProb;
	rank[j]=i;
	min_id=i;
      }
    }
  }
  return rank;
}

double pow_Yu(double base, int cs){
  
  int i;
  double result;
  result=base;
  if (cs==0)
    result=1;
  else{
    for(i=1;i<cs;i++){
      result=result*base;
    }
  }
  return result;
}

int ceil_Yu(double input){
  int ref;
  double output;
  ref=(int)(input);
  if(ref<input)
    output=ref+1;
  else 
    output=ref;
  return (int)(output);
}
int Layer_no(int base,int pdfs){
  int value,i;
  value=1;
  if (pdfs==1)
	  i=0;
  else
  {
	for (i=1;i<100;i++)
	{ 
		value=value*base;
		if (value>=pdfs)
		{
			break;
		}
  
	}
  }
  return i;
}

int Max_clu(int cs){

  int max_id,i;
  double max;
  max=Cluster[0].pdf_no;
  max_id=0;
  for(i=1;i<cs;i++){
    if(Cluster[i].pdf_no>max){
      max=Cluster[i].pdf_no;
      max_id=i;
    }
  }
  return max_id;
}

void copyHMM2Gauss(struct HMM_struct *triphone){

  int model_index, state_index, mix_index;
  int gaussian_index=0;
  int i;
  
  printf("Copying Hmm to Gaussian...\n");
  for (model_index=0; model_index<TotalCDHMM; model_index++) {
    for (state_index=0; state_index<triphone[model_index].state_no; state_index++) {
      for (mix_index=0; mix_index<triphone[model_index].mix_no[state_index]; mix_index++) {
	Gaussian[gaussian_index].phone_name= triphone[model_index].phone_name;
	Gaussian[gaussian_index].state_no = state_index;
	Gaussian[gaussian_index].mix_no = mix_index;
	Gaussian[gaussian_index].pdf_id = gaussian_index;
	for (i=0; i<st_config.m_inputFeatDim; i++) {
	  Gaussian[gaussian_index].mean[i] = triphone[model_index].mean[state_index][mix_index][i];
	  Gaussian[gaussian_index].var[i] = triphone[model_index].var[state_index][mix_index][i]; 
	  
	}
	gaussian_index++;
      }
    } 
  }
  if (gaussian_index!=TotalGauss)
    printf("GaussianCluster: Mismatched number of Gaussian mixtures\n");
}
void ComputeWeightCentroid( int clu_index){

   int i,k,now_pdf;
   Vector denom_acc;
   Vector mean_numer_acc;
   Vector var_numer_acc;
   Vector tmpvector;
   denom_acc = CreateVector(FeaDim); //Cluster[clu_index].pdf_no;
   ZeroVector(denom_acc, FeaDim);
   mean_numer_acc = CreateVector(FeaDim);
   ZeroVector(mean_numer_acc, FeaDim);
   var_numer_acc = CreateVector(FeaDim);
   ZeroVector(var_numer_acc, FeaDim);
   tmpvector = CreateVector(FeaDim);
   for (i=0; i<Cluster[clu_index].pdf_no; i++){
     now_pdf=Cluster[clu_index].pdf_member[i];
     for (k=0; k<st_config.m_inputFeatDim; k++)
       {
	 mean_numer_acc[k] +=Gaussian[now_pdf].mean[k] / pow(Gaussian[now_pdf].var[k],0.5);
	 denom_acc[k] += 1/pow(Gaussian[now_pdf].var[k],0.5); 
	 var_numer_acc[k] += (Gaussian[now_pdf].mean[k]*Gaussian[now_pdf].mean[k]/ Gaussian[now_pdf].var[k]);
       }
   }
   
   for (k=0; k<st_config.m_inputFeatDim; k++) {
     Cluster[clu_index].MeanCentroid[k] = mean_numer_acc[k] / denom_acc[k];
     tmpvector[k] = Cluster[clu_index].MeanCentroid[k]; 
     Cluster[clu_index].VarCentroid[k] = 
       (var_numer_acc[k]/ denom_acc[k]) - (tmpvector[k]*tmpvector[k]);
   }
   FreeVector(mean_numer_acc);
   FreeVector(var_numer_acc);
   FreeVector(tmpvector);
   FreeVector(denom_acc);
}

void ComputeCentroid( int clu_index){

   int i,k,now_pdf;
   double denom_acc;
   Vector mean_numer_acc, var_numer_acc;
   Vector tmpvector;
   denom_acc = Cluster[clu_index].pdf_no;
   mean_numer_acc = CreateVector(st_config.m_inputFeatDim);
   ZeroVector(mean_numer_acc, st_config.m_inputFeatDim);
   var_numer_acc = CreateVector(st_config.m_inputFeatDim);
   ZeroVector(var_numer_acc, st_config.m_inputFeatDim);
   tmpvector = CreateVector(st_config.m_inputFeatDim);
   for (i=0; i<Cluster[clu_index].pdf_no; i++){
     now_pdf=Cluster[clu_index].pdf_member[i];
     for (k=0; k<st_config.m_inputFeatDim; k++)
       {
	 mean_numer_acc[k] +=Gaussian[now_pdf].mean[k];
	 
	 var_numer_acc[k] += (Gaussian[now_pdf].var[k] + Gaussian[now_pdf].mean[k]*Gaussian[now_pdf].mean[k]);
       }
   }
  
   for (k=0; k<st_config.m_inputFeatDim; k++) {
     Cluster[clu_index].MeanCentroid[k] = mean_numer_acc[k] / denom_acc;
     tmpvector[k] = mean_numer_acc[k] / denom_acc;
     Cluster[clu_index].VarCentroid[k] = (var_numer_acc[k]/denom_acc ) - (tmpvector[k]*tmpvector[k]);
   }
   
   FreeVector(mean_numer_acc);
   FreeVector(var_numer_acc);
   FreeVector(tmpvector);
}

static int
vq_init_comp(const void*p1, const void* p2)
{
  double *f1 = (double*)p1, *f2 = (double*)p2; 
  double fr1 = f1[1], fr2 = f2[1]; 
  if (fr1 > fr2)
      return 1;
  else if ( fr1 == fr2 )
      return 0;
  else
      return -1;
 }

static double* Init_cb_ave(){

  double ave_mean[MAX_DIM_MFCC16 ];
  short i, j; 
  double *p_idx_dis; 

  for(i=0;i<st_config.m_inputFeatDim;i++)
    ave_mean[i] = 0;
  
  for (i=nMean-1;i>=0;i--){
    for (j=st_config.m_inputFeatDim-1;j>=0;j--){
      ave_mean[j] += Gaussian[i].mean[j]; 
    }
  }
  for (j=st_config.m_inputFeatDim-1;j>=0;j--)
    ave_mean[j] /= nMean; 

  p_idx_dis  = (double*)malloc(sizeof(double)*2*nMean);
  for (i=nMean-1;i>=0;i--){
    p_idx_dis[2*i] = i;
    p_idx_dis[2*i+1] = CalWeightEucleanDistance(Gaussian[i].mean,Gaussian[i].var, ave_mean, st_config.m_inputFeatDim);
  }

  qsort((void*)p_idx_dis, nMean, sizeof(double)*2, vq_init_comp);

           return p_idx_dis;
}
  

void Initial_codebook(int cs){
  int  i,j, k;
  Matrix select;
  int *count;
  int rat;
  double * p_sorted_dist; 
  select=CreateMatrix(cs,TotalGauss);
  count=(int *)calloc(cs ,sizeof(int));
   
  p_sorted_dist = Init_cb_ave();

  for(i=0;i<cs;i++)
    count[i]=0;
  
  rat=(TotalGauss/cs) ;
  if (rat * cs < TotalGauss) rat ++ ; 
  k = 0;
  for(i=0;i<cs;i++){
    for(j=i*rat;j<(i*rat+rat) && j < TotalGauss;j++){
      select[i][count[i]]=p_sorted_dist[2*k]; 
      count[i]++;
      k++;
    }
  }
  free(p_sorted_dist);

  for(i=0;i<cs;i++){
    Cluster[i].pdf_no=count[i];
    AllocateClusterMemberTable(i,Cluster[i].pdf_no);   
    for(j=0;j<count[i];j++){
      Cluster[i].pdf_member[j]=(int)(select[i][j]);
    }
  }
	
  

  for(i=0;i<cs;i++)
    ComputeCentroid(i);
  printf("Complete Initialize Codebook \n");
  FreeMatrix(select,cs);
  free(count);
  
}	

void SFB_Initial_codebook(int cs){
  int  i,j;
  Matrix select;
  int *count;
  int rat;
  select=CreateMatrix(cs,TotalGauss);
  count=(int *)calloc(cs ,sizeof(int));
   
  for(i=0;i<cs;i++)
    count[i]=0;
  
  rat=(TotalGauss/cs);
  for(i=0;i<cs;i++){
    for(j=i*rat;j<(i*rat+rat);j++){
      select[i][count[i]]=j;	
      count[i]++;
    }
  }
  

  for(i=0;i<cs;i++){
    Cluster[i].pdf_no=rat;
    AllocateClusterMemberTable(i,Cluster[i].pdf_no);   
    for(j=0;j<rat;j++){
      Cluster[i].pdf_member[j]=(int)(select[i][j]);
    }
  }
	
  

  for(i=0;i<cs;i++)
    ComputeCentroid(i);
  printf("Complete Initialize Codebook \n");
  FreeMatrix(select,cs);
  free(count);
  
}	

void CodebookVQ(int cs, int No_Gaussian){
  struct Node *node;       
  int i,j,k,l,now_pdf,clu,nopdf_acc,maxclu,node_no,parent_id,flag;
  int begin_node,end_node,layer;
  double epsilon;
  int *count,nopdf[1000];
  double D, D_prime;
  Matrix distance_table;
  Matrix tmp;
  count=(int *)calloc(cs, sizeof(int));
  distance_table = CreateMatrix(No_Gaussian, cs);
  tmp=CreateMatrix(cs,No_Gaussian );
  D_prime = 0;
  D = 0;
  do {
    D_prime = D;

    /* If a cluster has Gaussians mapped to it, clear the mapping
     * in preparation for finding the new mapping */
    for (i=0; i<cs; i++){
      if(Cluster[i].pdf_no!=0){
	D = 0.0;
	count[i]=0;
	Cluster[i].pdf_no=0;
	FreeClusterMemberTable(i);
      }
    }  

    /* Calculate distance from all Gaussians to cluster centroids */
    for (i=0; i<cs; i++) {
      for (j=0; j<No_Gaussian; j++) { 
	now_pdf = j;
	distance_table[j][i]=CalWeightEucleanDistance(Gaussian[now_pdf].mean,Gaussian[now_pdf].var,Cluster[i].MeanCentroid,st_config.m_inputFeatDim);
      }
    }

    /* Find the indices (mapping) of Gaussians that are closest to each
     * cluster centroid, and accumulated distance from all Gaussians
     *  to closest centroid. */
    for (j=0; j<No_Gaussian; j++) {
      clu=Min(distance_table[j],cs);
      tmp[clu][count[clu]] = j;
      count[clu]++;
      Cluster[clu].pdf_no += 1;
      D += distance_table[j][clu];
    }
    for (i=0; i<cs; i++) {
      AllocateClusterMemberTable(i,Cluster[i].pdf_no);
      for (j=0; j<Cluster[i].pdf_no; j++) {
	Cluster[i].pdf_member[j] = (int)(tmp[i][j]);
      }
    }

    /* Determine clusters that have no Gaussians mapped to them, and the total
     * number of such clusters */
    node_no=0;
    nopdf_acc=0;
    j=0;
    for(i=0;i<cs;i++){
      if(Cluster[i].pdf_no==0){
	nopdf_acc++;
	nopdf[j]=i;
	j++;
      }
    }	  


    /* Determine cluster that has most Gaussians mapped to it */
    maxclu=Max_clu(cs);
    

    /* If there are clusters with no Gaussians mapped to them, then reassign the
     * centroid means and variances of these clusters to locations close to the
     * mean and variance of the cluster that has most mapped Gaussians.
     * Develop the new means and variances using a binary tree to perturb
     * the mean and variance of the cluster that has most mapped Gaussians. */
    if(nopdf_acc!=0){
      /* Number of levels in the binary tree, and number of tree nodes so that
         * bottom layer has enough leaf nodes with perturbed centroids to
         * provide for the clusters with no mapped Gaussians */
      layer=(int)(ceil((log10(nopdf_acc)/log10(2))));
      for(l=0;l<=layer;l++)
	node_no+=(int)(pow(2,l));

      /* Construct array representing binary tree of perturbed centroid nodes */
      node=(struct Node *)calloc(node_no,sizeof(struct Node ));
      if(node==NULL){
	printf("GaussianCluster: Allocate error in Node \n");
	exit(-1);
      }

      /* Assign top node the centroid mean and variance corresponding to
       * the cluster that has most Gaussians mapped to it.  */
      for(k=0;k<st_config.m_inputFeatDim;k++){
	node[0].MeanCentroid[k]=Cluster[maxclu].MeanCentroid[k];
	node[0].VarCentroid[k]=Cluster[maxclu].VarCentroid[k];		 
      }

      /* Perturb the top node centroid to produce tree of centroids */
      epsilon = 0.05;
      for(i=1;i<=layer;i++){
	begin_node = (int)(pow(2, i))-1;
	end_node = (int)(pow(2, i+1))-2;
	for (j = begin_node; j <= end_node; j++){
	  
	  parent_id = ((j%2)==1) ? (j/2) : (j/2-1);
	  if((j%2)==0){
	    for(k=0;k<st_config.m_inputFeatDim;k++){
	      node[j].MeanCentroid[k]=node[parent_id].MeanCentroid[k] +
                      epsilon * node[0].MeanCentroid[k];
	      node[j].VarCentroid[k]=node[parent_id].VarCentroid[k] +
                      epsilon * node[0].VarCentroid[k];
	      //printf("%f:%f\n", node[0].MeanCentroid[k], node[0].VarCentroid[k]);
	      
	    }// k index	  
	  }		  
	  else{	
	    for(k=0;k<st_config.m_inputFeatDim;k++){
	      node[j].MeanCentroid[k]=node[parent_id].MeanCentroid[k] -
                      epsilon * node[0].MeanCentroid[k];;
	      node[j].VarCentroid[k]=node[parent_id].VarCentroid[k] -
                      epsilon * node[0].MeanCentroid[k];
	    }//k index 	 
	  }
	}  // j index

        /* Reduce perturbation for next tree level */
        epsilon /= 2;
      } //i index
      
    }//if index


    /* Assign perturbed centroids of leaves to clusters that had no Gaussians
     * mapped to them */
    if(nopdf_acc!=0){
      begin_node = (int)(pow(2,layer))-1;
      for(i=0;i<nopdf_acc;i++){
	for(k=0;k<st_config.m_inputFeatDim;k++){
	  Cluster[nopdf[i]].MeanCentroid[k]=node[i+begin_node].MeanCentroid[k];
	  Cluster[nopdf[i]].VarCentroid[k]=node[i+begin_node].VarCentroid[k];
	}
      }
    }
    if (nopdf_acc!=0) free(node);

    /* Calculate centroid for clusters that had Guassians mapped to them */
    for (i=0; i<cs; i++){
      flag=0;
      for(j=0;j<nopdf_acc;j++){
	if(nopdf[j]==i){
	  flag=1;
	  break;
	}
      }
      if(flag!=1)
	ComputeCentroid(i);
	  // ComputeWeightCentroid(i);

    }

    /* Output results of this clustering pass */
    printf("Divided to %d set: \n",cs);
    for (i=0; i<cs; i++) printf("<%d> ",Cluster[i].pdf_no);
    printf("\nClassfication Result: [Distortion = %f]\n",D );   
  }while (fabs(D -D_prime) > (delta * D_prime));

  printf("Converged...\n");
  FreeMatrix(distance_table, No_Gaussian);
  FreeMatrix(tmp, cs);
  free(count);
}

//......................................................................

int VQ(int size,double del, short bSwap){
  int codebook_size ;
  codebook_size=size;
  delta=del;
  copyHMM2Gauss(triphone);
  Initial_codebook(codebook_size);
  printf("Performing  VQ...\n");
  CodebookVQ(codebook_size,TotalGauss);

  printf("VQ complete\n");
  printf("Writing results for %d clusters...\n",codebook_size);
  Writecluster1(codebook_size, bSwap);

  return(1);
}
void Initial_Classy(int cs,struct HMM_struct *triphone){
   ComputeCentroidClassify(triphone);
}	

void ComputeCentroidClassify( struct HMM_struct *triphone){

   int k,now_pdf=0;
   int model_index,state_index,mix_index;
   for (model_index=0; model_index<TotalCIHMM; model_index++){
      for (state_index=0; state_index<triphone[model_index].state_no; state_index++) {
		  for (mix_index=0; mix_index<triphone[model_index].mix_no[state_index]; mix_index++) {
			for (k=0; k<st_config.m_inputFeatDim; k++) {
				ReferenceCluster[now_pdf].MeanCentroid[k] = triphone[model_index].mean[state_index][mix_index][k];
				ReferenceCluster[now_pdf].VarCentroid[k] = triphone[model_index].var[state_index][mix_index][k];
			}
		   now_pdf++;
		  }
	  }
   }
   
	if (now_pdf!=TotalReference)
	{
		printf("GaussianCluster: Mismatched Monophone and Cluster numbers");
		exit(-1);
	}
}
void CodebookClassify(int cs,double del){
 
 FILE *f_clu;
#ifdef  Binary_OUTPUT
 FILE *f_clu_b;
 short Temp_s;
 int TempMean;
 short CentroidMean_s;
 char name_CDHMM[100];
 char name_CDHMMbin[100];
#endif
 int clu,d;
 int i,j,now_pdf;
 int *count;
 double D, D_prime;
 Matrix distance_table;
 count=(int *)calloc(TotalReference, sizeof(int));
 distance_table = CreateMatrix(cs,TotalReference);
 D_prime = 1.0;
 D = 0.0;

 do {	    
   sprintf(name_CDHMM,"cls_centr_fromROM.list" );
   f_clu=fopen(name_CDHMM,"w");
#ifdef  Binary_OUTPUT
   sprintf(name_CDHMMbin,"cls_centr_fromROM.bin");
   f_clu_b=fopen(name_CDHMMbin,"wb");
   Temp_s=(short)(cs);
   // swap16(Temp_s);
   fwrite(&Temp_s,sizeof(short),1,f_clu_b);
   Temp_s=(short)(st_config.m_inputFeatDim);
   // swap16(Temp_s);
   fwrite(&Temp_s,sizeof(short),1,f_clu_b);
#endif
   D_prime = D;
   for (i=0; i<TotalReference; i++){
     if(ReferenceCluster[i].pdf_no!=0){
       D = 0.0;
       count[i]=0;
       ReferenceCluster[i].pdf_no=0;
     }
   }
   /* --- Compute Distance --- */    
   for (i=0; i<TotalReference; i++) {
     for (j=0; j<cs; j++) { 
       now_pdf = i;
       // distance_table[j][i] =  CalBhattaDistance(Gaussian[now_pdf].mean, Gaussian[now_pdf].var, Cluster[i].MeanCentroid, Cluster[i].VarCentroid,st_config.m_inputFeatDim);
       distance_table[j][i] =  CalWeightEucleanDistance(Cluster[j].MeanCentroid,ReferenceCluster[now_pdf].VarCentroid, ReferenceCluster[now_pdf].MeanCentroid, st_config.m_inputFeatDim);
     }
   }
   for (j=0; j<cs; j++) {
     clu=Min(distance_table[j],TotalReference);
     count[clu]++;
     fprintf(f_clu,"%d \n",clu);
#ifdef  Binary_OUTPUT
     for (d=0;d<st_config.m_inputFeatDim;d++)  
       {
	 TempMean=(int)(ReferenceCluster[clu].MeanCentroid[d]*2048);
	 CentroidMean_s=(short)(TempMean);
	 // swap16(CentroidMean_s);
	 fwrite(&CentroidMean_s,sizeof(short),1,f_clu_b);
}
     /*
       clu_s=short(clu);
       swap16(clu_s);
       fwrite(&clu_s,sizeof(short),1,f_clu_b);
     */
#endif
     ReferenceCluster[clu].pdf_no += 1;
     D += distance_table[j][clu];
   }
   
   printf("\nClassfication Result: [Distortion = %f]\n",D );  
   fclose(f_clu);
#ifdef  Binary_OUTPUT
   fclose(f_clu_b);
#endif
 }while (fabs(D -D_prime) > (del * D_prime));
 
 printf("Converged...\n");
 FreeMatrix(distance_table, cs);
 free(count);
 
}

void Writecluster1(int cs, short bSwap){

  FILE *f_clu,*f_mean,*f_var;
  char name_clu[100];
  char name_mean[100];
#ifdef  Binary_OUTPUT
  FILE *f_clu_b,*f_mean_b,*f_var_b;
  short clu_s;
  int TempMean;
  short Temp_s;
  short CentroidMean_s;
  char name_clu_bin[100];
  char name_mean_bin[100];
#endif
  int i,j,d,now_pdf,k; 
  int model_index,state_index,mix_index; 
  float ClusterVar[FeaDim];
  
  sprintf(name_clu,"ROM2cls.list");
  f_clu=fopen(name_clu,"w");
  sprintf(name_mean,"cls_centr.list");
  f_mean=fopen(name_mean,"w");
  f_var=fopen("Centroid_VAR.list","w");
#ifdef  Binary_OUTPUT
  sprintf(name_clu_bin,"ROM2cls.bin");
  f_clu_b=fopen(name_clu_bin,"wb");
  sprintf(name_mean_bin,"cls_centr.bin");
  f_mean_b=fopen(name_mean_bin,"wb");
  f_var_b=fopen("Centroid_VAR.bin","wb");
#ifdef  Binary_OUTPUT
  Temp_s=(short)(cs);
  if (bSwap) swap16(Temp_s);
  fwrite(&Temp_s,sizeof(short),1,f_mean_b);
  Temp_s=(short)(st_config.m_inputFeatDim);
  if (bSwap) swap16(Temp_s);
  fwrite(&Temp_s,sizeof(short),1,f_mean_b);
  Temp_s=(short)(TotalCDHMM);
  if (bSwap) swap16(Temp_s);
  fwrite(&Temp_s,sizeof(short),1,f_clu_b);
  Temp_s=(short)(cs);
  if (bSwap) swap16(Temp_s);
  fwrite(&Temp_s,sizeof(short),1,f_clu_b);
#endif
  
  
  
#endif
  printf("Preparing for classify\n");
  for(i=0;i<cs;i++){
    for (d=0;d<st_config.m_inputFeatDim;d++)  
      {
	fprintf(f_mean,"%f ",Cluster[i].MeanCentroid[d]);
#ifdef  Binary_OUTPUT
        TempMean=(int)(Cluster[i].MeanCentroid[d]*2048);
	CentroidMean_s=(short)(TempMean);
	if (bSwap) swap16(CentroidMean_s);
	fwrite(&CentroidMean_s,sizeof(short),1,f_mean_b);
#endif
      }
    fprintf(f_mean,"\n");
    for(j=0;j<Cluster[i].pdf_no;j++){
      now_pdf=Cluster[i].pdf_member[j];
      triphone[Gaussian[now_pdf].phone_name].cluster[Gaussian[now_pdf].state_no][Gaussian[now_pdf].mix_no]=i;
    }
  }
  for (model_index=0; model_index<TotalCDHMM; model_index++) {
    for (state_index=0; state_index<triphone[model_index].state_no; state_index++) {
      for (mix_index=0; mix_index<triphone[model_index].mix_no[state_index]; mix_index++) {
	fprintf(f_clu,"%d\n",triphone[model_index].cluster[state_index][mix_index]);
#ifdef  Binary_OUTPUT
	clu_s=(short)(triphone[model_index].cluster[state_index][mix_index]);
	if (bSwap) swap16(clu_s);
	fwrite(&clu_s,sizeof(short),1,f_clu_b);
#endif
      }
    }
  }
  for (k=0;k<st_config.m_inputFeatDim;k++)    ClusterVar[k]=0.0;
  for (i=0;i<cs;i++)
    {
      
      for (k=0;k<st_config.m_inputFeatDim;k++)
	{
	  ClusterVar[k]=ClusterVar[k]+(float)(Cluster[i].VarCentroid[k])/cs;
	}
    }
  for (k=0;k<st_config.m_inputFeatDim;k++) fprintf(f_var,"%f\n",ClusterVar[k]);
  for (i=0;i<cs;i++)
    {
      
      for (k=0;k<st_config.m_inputFeatDim;k++)
	{
	  Cluster[i].VarCentroid[k]=ClusterVar[k];
	}
    }
  fclose(f_clu);
  fclose(f_mean);
  fclose(f_var);
#ifdef  Binary_OUTPUT
  fclose(f_clu_b);
  fclose(f_mean_b);
  fclose(f_var_b);
#endif
}

void AllocateClusterMemberTable(int clu, int number)
{
   Cluster[clu].pdf_member = (int *)calloc(number, sizeof(int));
}

void AllocateClusterMemberTableClassify(int clu, int number)
{
   ReferenceCluster[clu].pdf_member = (int *)calloc(number, sizeof(int));
}


//......................................................................

int Classify(int size,struct HMM_struct *triphone,double del){
  int codebook_size ;
  codebook_size=size;
  Initial_Classy(codebook_size,triphone);
  printf("Performing  Classification...\n");
  CodebookClassify(codebook_size,del);
  printf("Classify complete\n");
  return(1);
}
