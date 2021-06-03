SKIP: FAILED



Validation Failure:
Texture2D<float4> arg_0 : register(t0, space1);

void textureLoad_e893d7() {
  float4 res = arg_0.Load(int3(0));
}

void vertex_main() {
  textureLoad_e893d7();
  return;
}

void fragment_main() {
  textureLoad_e893d7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_e893d7();
  return;
}


tint_DMD0a9:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0));
                               ^


tint_DMD0a9:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0));
                               ^


tint_DMD0a9:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0));
                               ^

