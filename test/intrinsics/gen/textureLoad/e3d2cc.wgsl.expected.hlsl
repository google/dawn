SKIP: FAILED



Validation Failure:
Texture2DMS<int4> arg_0 : register(t0, space1);

void textureLoad_e3d2cc() {
  int4 res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_e3d2cc();
  return;
}

void fragment_main() {
  textureLoad_e3d2cc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_e3d2cc();
  return;
}


tint_ANIEwS:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0), 1);
                             ^
tint_ANIEwS:4:25: warning: implicit truncation of vector type [-Wconversion]
  int4 res = arg_0.Load(int3(0), 1);
                        ^


tint_ANIEwS:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0), 1);
                             ^
tint_ANIEwS:4:25: warning: implicit truncation of vector type [-Wconversion]
  int4 res = arg_0.Load(int3(0), 1);
                        ^


tint_ANIEwS:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0), 1);
                             ^
tint_ANIEwS:4:25: warning: implicit truncation of vector type [-Wconversion]
  int4 res = arg_0.Load(int3(0), 1);
                        ^

