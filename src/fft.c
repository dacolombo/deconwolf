#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fftw3.h>
#include <string.h>

const char swf[] = "fftw_wisdom_float_threads.dat";
const unsigned int PREF_FFT_PLAN = FFTW_MEASURE;

void myfftw_start(void)
{
  int nThreads = 8;
  fftwf_init_threads();
  fftwf_plan_with_nthreads(nThreads);
  fftwf_import_wisdom_from_filename(swf);
}

void myfftw_stop(void)
{
  fftwf_export_wisdom_to_filename(swf);
  fftwf_cleanup_threads();
  fftwf_cleanup();
}

void dim3_real_float_inverse(fftwf_complex * in, float * out,
    const int n1, const int n2, const int n3)
{
  myfftw_start();

  fftwf_plan p;

  p = fftwf_plan_dft_c2r_3d(n3, n2, n1, 
      in, out, 
      FFTW_MEASURE);
  fftwf_execute(p);
  fftwf_destroy_plan(p);

  myfftw_stop();
}

void dim3_real_float(float * in, fftwf_complex* out,
    const int n1, const int n2, const int n3)
{

  if(0){
  FILE * fh = fopen("/tmp/log.txt", "w");
  fprintf(fh, "fftwf_version=%s\n", fftwf_version);
  fclose(fh);
  }

  myfftw_start();

  fftwf_plan p = fftwf_plan_dft_r2c_3d(n3, n2, n1, 
      in, // Float
      out, // fftwf_complex 
      FFTW_MEASURE);
  fftwf_execute(p); 
  fftwf_destroy_plan(p);

  myfftw_stop();
}

fftwf_complex * fft(float * in, int n1, int n2, int n3)
{
  fftwf_complex * out = fftwf_malloc(n1*n2*n3*sizeof(fftwf_complex));
  memset(out, 0, n1*n2*n3*sizeof(fftwf_complex));

fftwf_plan p = fftwf_plan_dft_r2c_3d(n3, n2, n1, 
      in, // Float
      out, // fftwf_complex 
      FFTW_MEASURE);
  fftwf_execute(p); 
  fftwf_destroy_plan(p);
  return out;
}

void fft_mul(fftwf_complex * C, fftwf_complex * A, fftwf_complex * B, size_t N)
{
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

float * fft_convolve_cc(fftwf_complex * A, fftwf_complex * B, int M, int N, int P)
{
  fftwf_complex * C = fftwf_malloc(M*N*P*sizeof(fftwf_complex));
  fft_mul(C, A, B, M*N*P); 

  float * out = fftwf_malloc(M*N*P*sizeof(float));
 
  fftwf_plan p = fftwf_plan_dft_c2r_3d(P, N, M, 
      C, out, 
      FFTW_MEASURE);
  fftwf_execute(p);
  fftwf_destroy_plan(p);
  fftwf_free(C);
  return out;
}

void fft_train(size_t M, size_t N, size_t P)
{
  printf("fftw3 training ... \n"); fflush(stdout);
  size_t MNP = M*N*P;
  fftwf_complex * C = fftwf_malloc(MNP*sizeof(fftwf_complex));
  float * R = fftwf_malloc(MNP*sizeof(float));

  fftwf_plan p1 = fftwf_plan_dft_c2r_3d(P, N, M, 
      C, R, FFTW_MEASURE);
    fftwf_execute(p1);
  fftwf_destroy_plan(p1);

fftwf_plan p2 = fftwf_plan_dft_r2c_3d(P, N, M, 
      R, C, FFTW_MEASURE);
    fftwf_execute(p2);
  fftwf_destroy_plan(p2);

  fftwf_free(R);
  fftwf_free(C);
  fftwf_export_wisdom_to_filename(swf);

  return;
}
