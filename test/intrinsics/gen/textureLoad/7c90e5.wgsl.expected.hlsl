SKIP: FAILED



Validation Failure:
Texture2DArray<uint4> arg_0 : register(t0, space1);

void textureLoad_7c90e5() {
  uint4 res = arg_0.Load(int4(1, 0), 1);
}

void vertex_main() {
  textureLoad_7c90e5();
  return;
}

void fragment_main() {
  textureLoad_7c90e5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_7c90e5();
  return;
}


tint_BFevEy:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0), 1);
                              ^


tint_BFevEy:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0), 1);
                              ^


tint_BFevEy:4:31: error: too few elements in vector initialization (expected 4 elements, have 2)
  uint4 res = arg_0.Load(int4(1, 0), 1);
                              ^

