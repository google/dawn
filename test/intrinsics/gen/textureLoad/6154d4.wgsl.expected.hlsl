SKIP: FAILED



Validation Failure:
Texture2D<uint4> arg_0 : register(t0, space1);

void textureLoad_6154d4() {
  uint4 res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_6154d4();
  return;
}

void fragment_main() {
  textureLoad_6154d4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_6154d4();
  return;
}


tint_QZYT9D:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  uint4 res = arg_0.Load(int3(0), 1);
                              ^


tint_QZYT9D:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  uint4 res = arg_0.Load(int3(0), 1);
                              ^


tint_QZYT9D:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  uint4 res = arg_0.Load(int3(0), 1);
                              ^

