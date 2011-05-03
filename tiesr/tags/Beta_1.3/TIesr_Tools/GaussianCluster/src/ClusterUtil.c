/*=======================================================================

 * 
 * ClusterUtil.c
 *
 * Utility object processing for clustering.
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
 
======================================================================*/

#include "ClusterUtil.h"

/*-------------------------- Create Vectors ------------------------------*/

extern double delta;
extern struct cluster Cluster[300];



Vector CreateVector(int dim)
{
   Vector v;
   v = (double *)calloc(dim, sizeof(double));
   return (v);
}

/*--------------------------- Free Vectors -------------------------------*/
void FreeVector(Vector v)
{
   free(v);
}

/*------------------------- Create Matrices ------------------------------*/
Matrix CreateMatrix(int n_row, int n_col)
{
   Matrix m;
   int i;
   m = (double **)calloc(n_row, sizeof(double *));
   for (i = 0; i < n_row; i++)
      m[i] = CreateVector(n_col);
   return (m);
}

/*-------------------------- Free Matrices -------------------------------*/
void FreeMatrix(Matrix m, int n_row)
{
   int i;
   for (i = 0; i < n_row; i++)
      FreeVector(m[i]);
   free(m);
}

/*-------------------------- Create Tensors ------------------------------*/
Tensor CreateTensor(int dim_x, int dim_y, int dim_z)
{
   Tensor t;
   int i;
   t = (double ***)calloc(dim_x, sizeof(double **));
   for (i = 0; i < dim_x; i++) {
      t[i] = CreateMatrix(dim_y, dim_z);
   }
   return (t);
}

/*--------------------------- Free Tensors -------------------------------*/
void FreeTensor(Tensor t, int dim_x, int dim_y)
{
   int i;
   for (i = 0; i < dim_x; i++) {
      FreeMatrix(t[i], dim_y);
   }
   free(t);
}

/* --------------------- Zero Vector/Matrix/Tensor -----------------------*/
void ZeroVector(Vector v, int dim) {
   int i;
   for (i=0; i<dim; i++)
      v[i] = 0.0;
}

void ZeroMatrix(Matrix m, int dim_x, int dim_y) {
   int i, j;
   for (i=0; i<dim_x; i++)
      for (j=0; j<dim_y; j++)
	 m[i][j] = 0.0;
}

void ZeroTensor(Tensor t, int dim_x, int dim_y, int dim_z) {
   int i, j, k;
   for (i=0; i<dim_x; i++)
      for (j=0; j<dim_y; j++)
	 for (k=0; k<dim_z; k++)
	    t[i][j][k] = 0.0;
}

/*--------------------------- Gauss Elimination --------------------------*/
void GaussEliminate(double **A, int dim, int *perm, int *sign) {
   double *s;    /* scale vector */
   int i, j, k, imax, li, lk;
   double r, rmax, smax, xmult, temp;

   *sign = 1;
   s = CreateVector(dim);
   /* initialize perm[], s[] */
   for (i=0; i<dim; i++) {
      perm[i] = i;
      smax = 0.0;
      for (j=0; j<dim; j++) {
	 if ((temp = fabs(A[i][j])) > smax) {
	    smax = temp;
	 }
      }
      if (smax == 0.0) {
	 printf("ERROR! Matrix is Singular!\n");
	 exit (-1);
      }
      s[i] = smax;
      /*printf("%d %f\n", perm[i], s[i]);*/
   } /* end initialization */

   for (k=0; k<dim-1; k++) {  /* index of Gauss Elimination procedure */
      rmax = 0.0;
      for (i=k; i<dim; i++) { /* choose pivot */
	 /*printf("start choosing pivot\n");*/
	 li = perm[i]; /*printf("li=%d", li);*/
	 r = fabs(A[li][k]/s[li]);
	 if (r > rmax) {
	    rmax = r;
	    j = i;    /* j indicates the pivot row */
	 }
	 if (rmax == 0.0) {
	    printf("\nERROR! rmax=0.0,Matrix is Singular!\n");
	    exit (-1);
	 }
      } /* end choosing pivot */
      imax = perm[j];
      perm[j] = perm[k];    /* interchange perm[j] & perm[k] */
      perm[k] = imax;
      if (((j-k)%2) == 1)
	 *sign = -(*sign);

      for (i=k+1; i<dim; i++) { /* for row i below row k */
	 lk = perm[k];
	 li = perm[i];
	 xmult = A[li][k]/A[lk][k];
	 A[li][k] = xmult;   /* store xmult in A[li][k] */
	 for (j=k+1; j<dim; j++) {  /* for columns in the right-side of k */
	    A[li][j] = A[li][j] - xmult*A[lk][j];
	 }
      } /* end elimination for row i */
   } /* end Gauss elimination */
} /* end of Gauss_eliminate */

/*------------------------- Solve Linear Equation ------------------------*/
void LinearSolve(double **A, int dim, int *perm, double *b, double *x) {
   int i, j, k, li, lk, ln;
   double sum;

   /* forward elimination of b[] */
   for (k=0; k<dim-1; k++) {
      lk = perm[k];
      for (i=k+1; i<dim; i++) { /* for the rows below k */
	 li = perm[i];
	 b[li] = b[li] - A[li][k]*b[lk];
      }
   }

   /* backward substraction */
   ln = perm[dim-1];
   x[dim-1] = b[ln]/A[ln][dim-1];

   for (i=dim-2; i>=0; i--) {
      li = perm[i];
      sum = b[li];
      for (j=i+1; j<dim; j++) {
	 sum = sum - A[li][j]*x[j];
      }
      x[i] = sum/A[li][i];
   }
}

/*---------------------------- Invert Matrix -----------------------------*/
double Invert(double **C, double **C_inv, int dim) {
   double **A;
   double col[100], x[100], xx;
   double log_det;
   int sign;
   int i, j, perm[100];
   int li;

   A = CreateMatrix(dim, dim);
   for (i=0; i<dim; i++)    /* make a copy of C */
      for (j=0; j<dim; j++)
	 A[i][j] = C[i][j];

   GaussEliminate(A, dim, perm, &sign);
   for (j=0; j<dim; j++) {
      for (i=0; i<dim; i++) { /* reset col[] */
	 col[i] = 0.0;
      }
      col[j] = 1.0;
      LinearSolve(A, dim, perm, col, x);   /* A*col'=col, solve column by column*/
      for (i=0; i<dim; i++) {
	 C_inv[i][j] = x[i];
      }
   }

   log_det = 0.0;  /* calculate log(det(C)) */
   for (i=0; i<dim; i++) {
      li = perm[i];
      xx = A[li][i];
      if (xx < 0.0)
	 sign = -sign;
      log_det += log(fabs(xx));
   }

   FreeMatrix(A, dim);
   return log_det;
}

/*-------------------------- Calculate bj(ot) ----------------------------*/
double CalStateLikeli(int LogKey, Vector data, vector *mean, vector *var, int M, Vector C)
{
   int m, i;
   double tmp1, tmp2, mix_likeli;
   double sum = 0.0;
   double state_likeli = MinProb;

   for (m=0; m < M; m++) {
      tmp1 = (-1.0)*mean[m][0];
      for (i=1; i<=FeaDim; i++) {
	 tmp2 = data[i-1] - mean[m][i];   /* o-mu */
	 tmp1 = tmp1 - (tmp2*tmp2)*var[m][i];
      }
      mix_likeli = exp((0.5)*tmp1);
      if (mix_likeli <= 0.0)
	 mix_likeli = 1e-30;
      sum += C[m]*mix_likeli;
   } /* end loop for mixture component */
   state_likeli = log(sum);
   if (LogKey == 0)
      return (sum);
   else
      return (state_likeli);
}

/*-------------------------- Calculate bjk(ot) ---------------------------*/
double CalMixLikeli(int LogKey, Vector data, vector mean, vector var)
{
   int i;
   double tmp1, tmp2;
   double sum = 0.0;
   double mix_likeli = MinProb;

   tmp1 = (-1.0) * mean[0];
   for (i=1; i<=FeaDim; i++) {
      tmp2 = data[i-1] - mean[i];
      tmp1 = tmp1 - (tmp2*tmp2)*var[i];
   }
   sum = exp((0.5)*tmp1);
   if (sum <= 0.0)
      sum = 1e-30;
   mix_likeli = log(sum);
   if (LogKey == 0)
      return (sum);
   else
      return (mix_likeli);
}
double CalEucleanDistance(Vector mean1, Vector mean2, int dim)
{
  int i;
   double distance = 0.0;
   double diff = 0.0;
   double term = 0.0;

   for (i=0; i<dim; i++) {
      diff = mean1[i] - mean2[i];
      term = diff * diff;
      distance += term;
   }

   return (distance);
}
double CalWeightEucleanDistance(Vector mean1, Vector invvar1, Vector mean2, int dim)
{
  int i;
   double distance = 0.0;
   double diff = 0.0;
   double term = 0.0;

   for (i=0; i<dim; i++) {
      diff = mean1[i] - mean2[i];
      term = diff * diff;
      term =term/ invvar1[i];
      distance += term;
   }

   return (distance);
}
double CalDivergence(Vector mean1, Vector sigma1, Vector mean2, Vector sigma2, int dim)
{
   int i;
   double divergence = 0.0;
   double diff = 0.0;
   double delta_kmi2 = 0.0;
   double term = 0.0;

   for (i=0; i<dim; i++) {
      diff = mean1[i] - mean2[i];
      delta_kmi2 = diff * diff;
      term = ((sigma1[i]+delta_kmi2)/sigma2[i])+((sigma2[i]+delta_kmi2)/sigma1[i]);
      divergence += term;
   }

   return (divergence);
}

double CalBhattaDistance(Vector mean1, Vector sigma1, Vector mean2, Vector sigma2, int dim)
{
   int i;
   double distance = 0.0;
   double diff = 0.0;
   double log_det_sigma1 = 0.0;
   double log_det_sigma2 = 0.0;
   double log_det_sigma1plus2 = 0.0;
   double term1 = 0.0;
   double term2 = 0.0;

   for (i=0; i<dim; i++) {
      diff = mean1[i] - mean2[i];



      term1 += diff * diff * (1.0)/(0.5*(sigma1[i]+sigma2[i]));
   }
   term1 *= (1.0)/(8.0);

   for (i=0; i<dim; i++) {
      log_det_sigma1 += log(sigma1[i]);
      log_det_sigma2 += log(sigma2[i]);
      log_det_sigma1plus2 += log(((sigma1[i]+sigma2[i])/2.0));
   }
   term2 = (0.5) * (log_det_sigma1plus2 - (0.5)*log_det_sigma1 - (0.5)*log_det_sigma2);

   distance = term1 + term2;
   return (distance);
}

void Abort(char *message) {
   fprintf(stderr, "\n%s\n", message);
   exit(-1);
}  
