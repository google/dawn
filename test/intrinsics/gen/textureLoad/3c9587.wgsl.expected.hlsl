Texture2D<float4> arg_0 : register(t0, space1);

void textureLoad_3c9587() {
  float4 res = arg_0.Load(int3(0, 0, 0));
}

void vertex_main() {
  textureLoad_3c9587();
  return;
}

void fragment_main() {
  textureLoad_3c9587();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_3c9587();
  return;
}

