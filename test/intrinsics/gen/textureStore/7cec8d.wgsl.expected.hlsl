RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_7cec8d() {
  arg_0[int3(0, 0, 1)] = int4(0, 0, 0, 0);
}

void vertex_main() {
  textureStore_7cec8d();
  return;
}

void fragment_main() {
  textureStore_7cec8d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_7cec8d();
  return;
}

