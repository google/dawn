SKIP: FAILED



Validation Failure:
Texture2DArray<int4> arg_0 : register(t0, space1);

void textureLoad_79e697() {
  int4 res = arg_0.Load(int4(1, 0), 1);
}

void vertex_main() {
  textureLoad_79e697();
  return;
}

void fragment_main() {
  textureLoad_79e697();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_79e697();
  return;
}


tint_P9Drzs:4:30: error: too few elements in vector initialization (expected 4 elements, have 2)
  int4 res = arg_0.Load(int4(1, 0), 1);
                             ^


tint_P9Drzs:4:30: error: too few elements in vector initialization (expected 4 elements, have 2)
  int4 res = arg_0.Load(int4(1, 0), 1);
                             ^


tint_P9Drzs:4:30: error: too few elements in vector initialization (expected 4 elements, have 2)
  int4 res = arg_0.Load(int4(1, 0), 1);
                             ^

