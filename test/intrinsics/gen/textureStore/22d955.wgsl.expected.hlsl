RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_22d955() {
  arg_0[int3(0, 0, 1)] = uint4(0u, 0u, 0u, 0u);
}

void vertex_main() {
  textureStore_22d955();
  return;
}

void fragment_main() {
  textureStore_22d955();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_22d955();
  return;
}

