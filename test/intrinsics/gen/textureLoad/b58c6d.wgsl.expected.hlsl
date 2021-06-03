Texture2DArray<float4> arg_0 : register(t0, space1);

void textureLoad_b58c6d() {
  float4 res = arg_0.Load(int4(0, 0, 1, 0));
}

void vertex_main() {
  textureLoad_b58c6d();
  return;
}

void fragment_main() {
  textureLoad_b58c6d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_b58c6d();
  return;
}

