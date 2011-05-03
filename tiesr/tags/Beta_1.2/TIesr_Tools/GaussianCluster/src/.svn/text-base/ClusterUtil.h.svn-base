/*=======================================================================
 ClusterUtil.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
======================================================================*/

#ifndef CLUSTERUTIL_H
#define CLUSTERUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>




/* Macro functions */
#define swap16(x) x = ((x << 8 & 0xff00) | (x >> 8 & 0xff))

#define SQUARE(A)   ((A)*(A))

#define MIN(a,b)  ((a) < (b) ? (a): (b))

#define MAX(a,b)  ((a) > (b) ? (a): (b))

#define LONG2SHORT(x) (short)(MIN(MAX(x, -32768),32767))


/* Q point constants */
#define Q11 2048
#define Q9 512

/* Logical constants */
#define FALSE 0
#define TRUE 1
#define OK 1
#define FAIL 0

/* symbolic constants related to features */
#define FeaDim 20         /* dimension of feature vector */
#define MAX_DIM_MFCC16 32

/* Global mfcc mean bias applied if input means are CMN */
static const short mean_bias[] = {14951, -2789, -811, 1563, -3013, -1346, 973, -884, 355, -496, 0, 0, 0, 0, 0, 0};

/* symbolic constants related to models - Maximum sizes allowed */
#define StateNum 10          /* number of states per model */
#define MaxMixNum 10         /* maximum number of mixtures per state */
#define VectorSize 32        /* dimension of mean and covariance */
#define N_dim 32


/* symbolic constants for prob calculation */
#define MaxProb  1.0e32
#define MinProb  -1.0e32
#define LSMALL   -0.5e10      /* log values < LSMALL are set to LZERO */
#define LZERO    -1.0e10       /* ~log(0) */
#define MaxHMM 7000
#define MaxCluster 1000
#define MaxGaussian 20000

/* Output binary versions of clustered data */
#define Binary_OUTPUT


/*
 * #define MLLR_FLAG 0
 * #define MAP_FLAG 0
 */

/*---------------------- global data setting -----------------------------*/
typedef double vector[N_dim];   /* define "vector", mean[0] for GConst */
typedef double *Vector;
typedef double **Matrix;
typedef double ***Tensor;
typedef double LogDouble;  /* just to signal that the value is from log */

struct HMM_struct {    /* for reading-in HTK model    */
   int phone_ID;
   int state_no;
   int  phone_name;
   int mix_no[StateNum];
   double mixweight[StateNum][MaxMixNum];
   int cluster[StateNum][MaxMixNum];
   double mean[StateNum][MaxMixNum][VectorSize];
   double var[StateNum][MaxMixNum][VectorSize];
   double transP[StateNum][StateNum];
   double Gconst[StateNum][MaxMixNum];
};


struct gaussian {
   int phone_name;
   int state_no,mix_no,pdf_id;
   double mean[FeaDim];
   double var[FeaDim];
   int pseudo;
};


struct cluster{
	int reference;
	int *pdf_member;
	int pdf_no;
	double MeanCentroid[FeaDim];
    double VarCentroid[FeaDim];
};


struct Node {
    double MeanCentroid[FeaDim];
    double VarCentroid[FeaDim];
};


void WriteNewModel(char *new_path,struct HMM_struct *triphone );
void FreeClusterMemberTable(int clu);
int Min(double *D, int cs);
int Max_clu(int cs);
void copyHMM2Gauss(struct HMM_struct *triphone);
void ComputeCentroid( int clu_index);
void Initial_codebook(int cs);
void CodebookVQ(int cs, int TotalGauss);
void CodebookClassify(int cs,double del);
int VQ(int size,double del, short bSwap);
int Classify(int size,struct HMM_struct *monophone,double del);
void ComputeCentroidClassify( struct HMM_struct *monophone);
void Abort(char *message);
void AllocateClusterMemberTableClassify(int clu, int number);
Matrix CreateMatrix(int n_row, int n_col);
void AllocateMemSentence(void);
Vector CreateVector(int dim);
void ZeroVector(Vector v, int dim);
void ZeroMatrix(Matrix m, int dim_x, int dim_y);
void ZeroTensor(Tensor t, int dim_x, int dim_y, int dim_z);
void GaussEliminate(double **A, int dim, int *perm, int *sign);
void LinearSolve(double **A, int dim, int *perm, double *b, double *x);
double Invert(double **C, double **C_inv, int dim);
double CalStateLikeli(int LogKey, Vector data, vector *mean, vector *var, int M, Vector C);
double CalMixLikeli(int LogKey, Vector data, vector mean, vector var);
double CalEucleanDistance(Vector mean1, Vector mean2, int dim);
double CalDivergence(Vector mean1, Vector sigma1, Vector mean2, Vector sigma2, int dim);
double CalBhattaDistance(Vector mean1, Vector sigma1, Vector mean2, Vector sigma2, int dim);
void FreeVector(Vector v);
void FreeMatrix(Matrix m, int n_row);
Tensor CreateTensor(int dim_x, int dim_y, int dim_z);
void FreeTensor(Tensor t, int dim_x, int dim_y);
extern void FreeTensor(Tensor t, int dim_x, int dim_y);
extern void FreeVector(Vector v);
extern void FreeMatrix(Matrix m, int n_row);
void AllocateClusterMemberTable(int clu, int number);
double pow_Yu(double base, int cs);
int ceil_Yu(double input);
int * Ranking(double *D, int cs, int pick);
int Layer_no(int base,int pdfs);
double CalWeightEucleanDistance(Vector mean1, Vector invvar1, Vector mean2, int dim);
void ComputeWeightCentroid( int clu_index);
void Writecluster1(int cs, short bSwap);
// void Writecluster2(int cs);

#endif
