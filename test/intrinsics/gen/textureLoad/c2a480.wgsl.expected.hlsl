SKIP: FAILED



Validation Failure:
Texture2D<int4> arg_0 : register(t0, space1);

void textureLoad_c2a480() {
  int4 res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_c2a480();
  return;
}

void fragment_main() {
  textureLoad_c2a480();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_c2a480();
  return;
}


tint_iO1fuJ:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0), 1);
                             ^


tint_iO1fuJ:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0), 1);
                             ^


tint_iO1fuJ:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0), 1);
                             ^

