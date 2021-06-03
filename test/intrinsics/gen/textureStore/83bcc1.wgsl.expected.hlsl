RWTexture1D<uint4> arg_0 : register(u0, space1);

void textureStore_83bcc1() {
  arg_0[1] = uint4(0u, 0u, 0u, 0u);
}

void vertex_main() {
  textureStore_83bcc1();
  return;
}

void fragment_main() {
  textureStore_83bcc1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_83bcc1();
  return;
}

