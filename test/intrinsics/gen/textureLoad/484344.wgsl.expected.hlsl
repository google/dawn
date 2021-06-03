SKIP: FAILED



Validation Failure:
Texture2D<float4> arg_0 : register(t0, space1);

void textureLoad_484344() {
  float4 res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_484344();
  return;
}

void fragment_main() {
  textureLoad_484344();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_484344();
  return;
}


tint_UQRhQC:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0), 1);
                               ^


tint_UQRhQC:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0), 1);
                               ^


tint_UQRhQC:4:32: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Load(int3(0), 1);
                               ^

