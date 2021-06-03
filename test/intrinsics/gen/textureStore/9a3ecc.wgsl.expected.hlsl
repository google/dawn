RWTexture3D<int4> arg_0 : register(u0, space1);

void textureStore_9a3ecc() {
  arg_0[int3(0, 0, 0)] = int4(0, 0, 0, 0);
}

void vertex_main() {
  textureStore_9a3ecc();
  return;
}

void fragment_main() {
  textureStore_9a3ecc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9a3ecc();
  return;
}

