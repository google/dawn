SKIP: FAILED



Validation Failure:
Texture2DMS<float4> arg_0 : register(t0, space1);

void textureLoad_a583c9() {
  float4 res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_a583c9();
  return;
}

void fragment_main() {
  textureLoad_a583c9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_a583c9();
  return;
}


tint_0qET7a:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0), 1);
                               ^
tint_0qET7a:4:27: warning: implicit truncation of vector type [-Wconversion]
  float4 res = arg_0.Load(int3(0), 1);
                          ^


tint_0qET7a:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0), 1);
                               ^
tint_0qET7a:4:27: warning: implicit truncation of vector type [-Wconversion]
  float4 res = arg_0.Load(int3(0), 1);
                          ^


tint_0qET7a:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0), 1);
                               ^
tint_0qET7a:4:27: warning: implicit truncation of vector type [-Wconversion]
  float4 res = arg_0.Load(int3(0), 1);
                          ^

