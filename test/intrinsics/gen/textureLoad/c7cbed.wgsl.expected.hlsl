Texture1D<float4> arg_0 : register(t0, space1);

void textureLoad_c7cbed() {
  float4 res = arg_0.Load(int2(1, 0));
}

void vertex_main() {
  textureLoad_c7cbed();
  return;
}

void fragment_main() {
  textureLoad_c7cbed();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_c7cbed();
  return;
}

