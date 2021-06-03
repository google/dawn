SKIP: FAILED



Validation Failure:
Texture2DArray<uint4> arg_0 : register(t0, space1);

void textureLoad_c40dcb() {
  uint4 res = arg_0.Load(int4(1, 0));
}

void vertex_main() {
  textureLoad_c40dcb();
  return;
}

void fragment_main() {
  textureLoad_c40dcb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_c40dcb();
  return;
}


tint_nOyzgu:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0));
                              ^


tint_nOyzgu:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0));
                              ^


tint_nOyzgu:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0));
                              ^

