RWTexture1D<uint4> arg_0 : register(u0, space1);

void textureStore_2eb2a4() {
  arg_0[1] = uint4(0u, 0u, 0u, 0u);
}

void vertex_main() {
  textureStore_2eb2a4();
  return;
}

void fragment_main() {
  textureStore_2eb2a4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2eb2a4();
  return;
}

