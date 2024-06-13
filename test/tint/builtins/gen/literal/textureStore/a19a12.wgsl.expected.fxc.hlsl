RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_a19a12() {
  arg_0[(1).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_a19a12();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_a19a12();
  return;
}
