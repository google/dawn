SKIP: FAILED



Validation Failure:
Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_1f2016() {
  float4 res = arg_0.Load(int4(0), 1);
}

void vertex_main() {
  textureLoad_1f2016();
  return;
}

void fragment_main() {
  textureLoad_1f2016();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_1f2016();
  return;
}


tint_jDpZEi:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0), 1);
                               ^


tint_jDpZEi:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0), 1);
                               ^


tint_jDpZEi:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0), 1);
                               ^

