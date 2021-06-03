Texture2D<float4> arg_0 : register(t0, space1);

void textureLoad_9c2a14() {
  float4 res = arg_0.Load(int3(0, 0, 0));
}

void vertex_main() {
  textureLoad_9c2a14();
  return;
}

void fragment_main() {
  textureLoad_9c2a14();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_9c2a14();
  return;
}

