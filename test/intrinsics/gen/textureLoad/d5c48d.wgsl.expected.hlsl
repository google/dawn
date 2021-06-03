SKIP: FAILED



Validation Failure:
Texture2D<float4> arg_0 : register(t0, space1);

void textureLoad_d5c48d() {
  float4 res = arg_0.Load(int3(0));
}

void vertex_main() {
  textureLoad_d5c48d();
  return;
}

void fragment_main() {
  textureLoad_d5c48d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_d5c48d();
  return;
}


tint_UeYBhz:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0));
                               ^


tint_UeYBhz:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0));
                               ^


tint_UeYBhz:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0));
                               ^

