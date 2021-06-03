SKIP: FAILED



Validation Failure:
Texture3D<int4> arg_0 : register(t0, space1);

void textureLoad_3d001b() {
  int4 res = arg_0.Load(int4(0));
}

void vertex_main() {
  textureLoad_3d001b();
  return;
}

void fragment_main() {
  textureLoad_3d001b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_3d001b();
  return;
}


tint_7HF8Og:4:30: error: too few elements in vector initialization (expected 4 elements, have 1)
  int4 res = arg_0.Load(int4(0));
                             ^


tint_7HF8Og:4:30: error: too few elements in vector initialization (expected 4 elements, have 1)
  int4 res = arg_0.Load(int4(0));
                             ^


tint_7HF8Og:4:30: error: too few elements in vector initialization (expected 4 elements, have 1)
  int4 res = arg_0.Load(int4(0));
                             ^

