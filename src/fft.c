#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fftw3.h>
#include <string.h>

// FFTW_MEASURE, FFTW_PATIENT or FFTW_EXHAUSTIVE
char * swf = NULL;
const unsigned int MYPLAN = FFTW_MEASURE;
//const unsigned int MYPLAN = FFTW_PATIENT;

void myfftw_start(int nThreads)
{  
//  printf("\t using %s with %d threads\n", fftwf_version, nThreads);
  fftwf_init_threads();
  fftwf_plan_with_nthreads(nThreads);
  swf = malloc(100*sizeof(char));
  sprintf(swf, "fftw_wisdom_float_threads_%d.dat", nThreads);
  fftwf_import_wisdom_from_filename(swf);
}

void myfftw_stop(void)
{
  fftwf_export_wisdom_to_filename(swf);
  free(swf);
  fftwf_cleanup_threads();
  fftwf_cleanup();
}


fftwf_complex * fft(float * in, int n1, int n2, int n3)
{
  int n3red = (n3+3)/2;
  fftwf_complex * out = fftwf_malloc(n1*n2*n3red*sizeof(fftwf_complex));
  memset(out, 0, n1*n2*n3red*sizeof(fftwf_complex));

  fftwf_plan p = fftwf_plan_dft_r2c_3d(n3, n2, n1, 
      in, // Float
      out, // fftwf_complex 
      MYPLAN);
  fftwf_execute(p); 
  fftwf_destroy_plan(p);
  return out;
}

void fft_mul(fftwf_complex * C, fftwf_complex * A, fftwf_complex * B, size_t n1, size_t n2, size_t n3)
{
  int n3red = (n3+3)/2;
  size_t N = n1*n2*n3red;
  // C = A*B
  for(size_t kk = 0; kk<N; kk++)
  {
    float a = A[kk][0]; float ac = A[kk][1];
    float b = B[kk][0]; float bc = B[kk][1];
    C[kk][0] = a*b - ac*bc;
    C[kk][1] = a*bc + b*ac;
  }
  return;
}

float * fft_convolve_cc(fftwf_complex * A, fftwf_complex * B, const int M, const int N, const int P)
{
  size_t n3red = (P+3)/2;
  fftwf_complex * C = fftwf_malloc(M*N*n3red*sizeof(fftwf_complex));
  fft_mul(C, A, B, M, N, P); 

  float * out = fftwf_malloc(M*N*P*sizeof(float));

  fftwf_plan p = fftwf_plan_dft_c2r_3d(P, N, M, 
      C, out, 
      MYPLAN);
  fftwf_execute(p);
  fftwf_destroy_plan(p);
  fftwf_free(C);

  for(size_t kk = 0; kk<M*N*P; kk++)
  {
    out[kk]/=(M*N*P);
  }
  return out;
}

void fft_train(size_t M, size_t N, size_t P, int verbosity)
{
  if(verbosity > 1){
    printf("fftw3 training ... \n"); fflush(stdout);
  }
  size_t MNP = M*N*P;
  fftwf_complex * C = fftwf_malloc(MNP*sizeof(fftwf_complex));
  float * R = fftwf_malloc(MNP*sizeof(float));

  fftwf_plan p0 = fftwf_plan_dft_c2r_3d(P, N, M, 
      C, R, MYPLAN | FFTW_WISDOM_ONLY);

  if(p0 == NULL)
  {    
    if(verbosity > 0)
    {
    printf("> generating c2r plan\n");
    }
  fftwf_plan p1 = fftwf_plan_dft_c2r_3d(P, N, M, 
      C, R, MYPLAN);
  fftwf_execute(p1);
  fftwf_destroy_plan(p1);
  } else {
    if(verbosity > 1)
    {
      printf("\tc2r -- ok\n");
    }
  }
fftwf_destroy_plan(p0);

    p0 = fftwf_plan_dft_r2c_3d(P, N, M, 
      R, C, MYPLAN | FFTW_WISDOM_ONLY);

if(p0 == NULL)
{
  if(verbosity > 0){
    printf("> generating r2c plan \n");
  }
  fftwf_plan p2 = fftwf_plan_dft_r2c_3d(P, N, M, 
      R, C, MYPLAN);
  fftwf_execute(p2);
  fftwf_destroy_plan(p2);
} else {
  if(verbosity > 1)
  {
  printf("\tr2c -- ok\n");
  }
  }
fftwf_destroy_plan(p0);

  fftwf_free(R);
  fftwf_free(C);
  fftwf_export_wisdom_to_filename(swf);

  return;
}
