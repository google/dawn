RWTexture2D<float4> arg_0 : register(u0, space1);

void textureStore_05ce15() {
  arg_0[int2(0, 0)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

void vertex_main() {
  textureStore_05ce15();
  return;
}

void fragment_main() {
  textureStore_05ce15();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_05ce15();
  return;
}

