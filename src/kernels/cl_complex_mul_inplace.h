unsigned char cl_complex_mul_inplace[] = {
  0x2f, 0x2f, 0x20, 0x77, 0x4d, 0x4e, 0x50, 0x20, 0x64, 0x65, 0x66, 0x69,
  0x6e, 0x65, 0x64, 0x20, 0x61, 0x74, 0x20, 0x63, 0x6f, 0x6d, 0x70, 0x69,
  0x6c, 0x65, 0x20, 0x74, 0x69, 0x6d, 0x65, 0x0a, 0x5f, 0x5f, 0x6b, 0x65,
  0x72, 0x6e, 0x65, 0x6c, 0x20, 0x76, 0x6f, 0x69, 0x64, 0x20, 0x63, 0x6c,
  0x5f, 0x63, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x78, 0x5f, 0x6d, 0x75, 0x6c,
  0x5f, 0x69, 0x6e, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x28, 0x63, 0x6f, 0x6e,
  0x73, 0x74, 0x20, 0x5f, 0x5f, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x2a, 0x20, 0x41, 0x2c, 0x0a, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x5f, 0x5f, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x20, 0x2a, 0x20, 0x42, 0x29, 0x0a, 0x7b, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x2f, 0x2f, 0x20, 0x42, 0x20, 0x3d, 0x20, 0x41, 0x2e, 0x2a,
  0x42, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x20,
  0x69, 0x6e, 0x74, 0x20, 0x69, 0x64, 0x20, 0x3d, 0x20, 0x67, 0x65, 0x74,
  0x5f, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x5f, 0x69, 0x64, 0x28, 0x30,
  0x29, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x69, 0x66, 0x28, 0x69, 0x64,
  0x20, 0x3c, 0x20, 0x63, 0x4d, 0x4e, 0x50, 0x29, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x7b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x66,
  0x6c, 0x6f, 0x61, 0x74, 0x20, 0x72, 0x65, 0x20, 0x3d, 0x20, 0x41, 0x5b,
  0x32, 0x2a, 0x69, 0x64, 0x5d, 0x2a, 0x42, 0x5b, 0x32, 0x2a, 0x69, 0x64,
  0x5d, 0x20, 0x2d, 0x20, 0x41, 0x5b, 0x32, 0x2a, 0x69, 0x64, 0x2b, 0x31,
  0x5d, 0x2a, 0x42, 0x5b, 0x32, 0x2a, 0x69, 0x64, 0x2b, 0x31, 0x5d, 0x3b,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x20, 0x69, 0x6d, 0x20, 0x3d, 0x20, 0x41, 0x5b, 0x32, 0x2a,
  0x69, 0x64, 0x5d, 0x2a, 0x42, 0x5b, 0x32, 0x2a, 0x69, 0x64, 0x2b, 0x31,
  0x5d, 0x20, 0x2b, 0x20, 0x41, 0x5b, 0x32, 0x2a, 0x69, 0x64, 0x2b, 0x31,
  0x5d, 0x2a, 0x42, 0x5b, 0x32, 0x2a, 0x69, 0x64, 0x5d, 0x3b, 0x0a, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x42, 0x5b, 0x32, 0x2a, 0x69,
  0x64, 0x5d, 0x3d, 0x72, 0x65, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x42, 0x5b, 0x32, 0x2a, 0x69, 0x64, 0x2b, 0x31, 0x5d,
  0x3d, 0x69, 0x6d, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x7d, 0x0a, 0x7d,
  0x0a
};
unsigned int cl_complex_mul_inplace_len = 397;
