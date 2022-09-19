__kernel void cl_complex_mul_conj(__global float *A, __global float * B, __global float * C)
{
    // C = A.*B
    const int id = get_global_id(0);

    float re = A[2*id]*B[2*id] + A[2*id+1]*B[2*id+1];
    float im = A[2*id]*B[2*id+1] - A[2*id+1]*B[2*id];
    C[2*id]=re;
    C[2*id+1]=im;

}
