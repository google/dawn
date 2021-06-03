SKIP: FAILED



Validation Failure:
Texture2D<int4> arg_0 : register(t0, space1);

void textureLoad_dbd554() {
  int4 res = arg_0.Load(int3(0));
}

void vertex_main() {
  textureLoad_dbd554();
  return;
}

void fragment_main() {
  textureLoad_dbd554();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_dbd554();
  return;
}


tint_SLZaZo:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0));
                             ^


tint_SLZaZo:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0));
                             ^


tint_SLZaZo:4:30: error: too few elements in vector initialization (expected 3 elements, have 1)
  int4 res = arg_0.Load(int3(0));
                             ^

