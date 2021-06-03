SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);

void textureLoad_1a062f() {
  float4 res = arg_0.Load(int4(1, 0));
}

void vertex_main() {
  textureLoad_1a062f();
  return;
}

void fragment_main() {
  textureLoad_1a062f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_1a062f();
  return;
}


tint_ONx53X:4:32: error: too few elements in vector initialization (expected 4 elements, have 2)
  float4 res = arg_0.Load(int4(1, 0));
                               ^


tint_ONx53X:4:32: error: too few elements in vector initialization (expected 4 elements, have 2)
  float4 res = arg_0.Load(int4(1, 0));
                               ^


tint_ONx53X:4:32: error: too few elements in vector initialization (expected 4 elements, have 2)
  float4 res = arg_0.Load(int4(1, 0));
                               ^

