SKIP: FAILED



Validation Failure:
Texture3D<uint4> arg_0 : register(t0, space1);

void textureLoad_f56e6f() {
  uint4 res = arg_0.Load(int4(0));
}

void vertex_main() {
  textureLoad_f56e6f();
  return;
}

void fragment_main() {
  textureLoad_f56e6f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_f56e6f();
  return;
}


tint_qJ0MK4:4:31: error: too few elements in vector initialization (expected 4 elements, have 1)
  uint4 res = arg_0.Load(int4(0));
                              ^


tint_qJ0MK4:4:31: error: too few elements in vector initialization (expected 4 elements, have 1)
  uint4 res = arg_0.Load(int4(0));
                              ^


tint_qJ0MK4:4:31: error: too few elements in vector initialization (expected 4 elements, have 1)
  uint4 res = arg_0.Load(int4(0));
                              ^

