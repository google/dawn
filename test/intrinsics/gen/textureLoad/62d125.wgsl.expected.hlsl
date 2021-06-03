SKIP: FAILED



Validation Failure:
Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_62d125() {
  float4 res = arg_0.Load(int4(0));
}

void vertex_main() {
  textureLoad_62d125();
  return;
}

void fragment_main() {
  textureLoad_62d125();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_62d125();
  return;
}


tint_Q3PltH:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0));
                               ^


tint_Q3PltH:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0));
                               ^


tint_Q3PltH:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0));
                               ^

