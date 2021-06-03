SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);

void textureLoad_87be85() {
  float4 res = arg_0.Load(int4(1, 0), 1);
}

void vertex_main() {
  textureLoad_87be85();
  return;
}

void fragment_main() {
  textureLoad_87be85();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_87be85();
  return;
}


tint_vATPFA:4:32: error: too few elements in vector initialization (expected 4 elements, have 2)
  float4 res = arg_0.Load(int4(1, 0), 1);
                               ^


tint_vATPFA:4:32: error: too few elements in vector initialization (expected 4 elements, have 2)
  float4 res = arg_0.Load(int4(1, 0), 1);
                               ^


tint_vATPFA:4:32: error: too few elements in vector initialization (expected 4 elements, have 2)
  float4 res = arg_0.Load(int4(1, 0), 1);
                               ^

