Texture2DMS<float4> arg_0 : register(t0, space1);

void textureLoad_a583c9() {
  float4 res = arg_0.Load(int3(0, 0, 0), 1);
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

