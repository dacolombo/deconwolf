__kernel void cl_complex_mul(const __global float * A,
                             const __global float * B,
                             __global float * C,
                             const __global size_t * complex_size)
{
    // C = A.*B
    const int id = get_global_id(0);

    if(id < complex_size[0])
    {
        float re = A[2*id]*B[2*id] - A[2*id+1]*B[2*id+1];
        float im = A[2*id]*B[2*id+1] + A[2*id+1]*B[2*id];
        C[2*id]=re;
        C[2*id+1]=im;
    }
}
