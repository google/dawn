SKIP: FAILED



Validation Failure:
Texture2DArray<int4> arg_0 : register(t0, space1);

void textureLoad_582015() {
  int4 res = arg_0.Load(int4(1, 0));
}

void vertex_main() {
  textureLoad_582015();
  return;
}

void fragment_main() {
  textureLoad_582015();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_582015();
  return;
}


tint_zwnAkS:4:30: error: too few elements in vector initialization (expected 4 elements, have 2)
  int4 res = arg_0.Load(int4(1, 0));
                             ^


tint_zwnAkS:4:30: error: too few elements in vector initialization (expected 4 elements, have 2)
  int4 res = arg_0.Load(int4(1, 0));
                             ^


tint_zwnAkS:4:30: error: too few elements in vector initialization (expected 4 elements, have 2)
  int4 res = arg_0.Load(int4(1, 0));
                             ^

