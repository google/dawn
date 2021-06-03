RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_bbcb7f() {
  arg_0[int2(0, 0)] = int4(0, 0, 0, 0);
}

void vertex_main() {
  textureStore_bbcb7f();
  return;
}

void fragment_main() {
  textureStore_bbcb7f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_bbcb7f();
  return;
}

