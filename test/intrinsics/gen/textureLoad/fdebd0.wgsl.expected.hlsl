SKIP: FAILED



Validation Failure:
Texture2DArray<uint4> arg_0 : register(t0, space1);

void textureLoad_fdebd0() {
  uint4 res = arg_0.Load(int4(1, 0));
}

void vertex_main() {
  textureLoad_fdebd0();
  return;
}

void fragment_main() {
  textureLoad_fdebd0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_fdebd0();
  return;
}


tint_cI4djR:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0));
                              ^


tint_cI4djR:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0));
                              ^


tint_cI4djR:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0));
                              ^

