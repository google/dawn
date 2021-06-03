SKIP: FAILED



Validation Failure:
Texture3D<float4> arg_0 : register(t0, space1);

void textureLoad_a6a85a() {
  float4 res = arg_0.Load(int4(0));
}

void vertex_main() {
  textureLoad_a6a85a();
  return;
}

void fragment_main() {
  textureLoad_a6a85a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_a6a85a();
  return;
}


tint_5T6Aqu:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0));
                               ^


tint_5T6Aqu:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0));
                               ^


tint_5T6Aqu:4:32: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Load(int4(0));
                               ^

