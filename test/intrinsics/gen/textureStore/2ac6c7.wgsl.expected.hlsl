RWTexture1D<float4> arg_0 : register(u0, space1);

void textureStore_2ac6c7() {
  arg_0[1] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

void vertex_main() {
  textureStore_2ac6c7();
  return;
}

void fragment_main() {
  textureStore_2ac6c7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2ac6c7();
  return;
}

