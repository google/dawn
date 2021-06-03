Texture1D<float4> arg_0 : register(t0, space1);

void textureLoad_81c381() {
  float4 res = arg_0.Load(int2(1, 0), 1);
}

void vertex_main() {
  textureLoad_81c381();
  return;
}

void fragment_main() {
  textureLoad_81c381();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_81c381();
  return;
}

