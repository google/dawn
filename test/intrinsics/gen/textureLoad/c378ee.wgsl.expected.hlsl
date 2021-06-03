SKIP: FAILED



Validation Failure:
Texture2DMS<uint4> arg_0 : register(t0, space1);

void textureLoad_c378ee() {
  uint4 res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_c378ee();
  return;
}

void fragment_main() {
  textureLoad_c378ee();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_c378ee();
  return;
}


tint_xcs7fj:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  uint4 res = arg_0.Load(int3(0), 1);
                              ^
tint_xcs7fj:4:26: warning: implicit truncation of vector type [-Wconversion]
  uint4 res = arg_0.Load(int3(0), 1);
                         ^


tint_xcs7fj:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  uint4 res = arg_0.Load(int3(0), 1);
                              ^
tint_xcs7fj:4:26: warning: implicit truncation of vector type [-Wconversion]
  uint4 res = arg_0.Load(int3(0), 1);
                         ^


tint_xcs7fj:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  uint4 res = arg_0.Load(int3(0), 1);
                              ^
tint_xcs7fj:4:26: warning: implicit truncation of vector type [-Wconversion]
  uint4 res = arg_0.Load(int3(0), 1);
                         ^

