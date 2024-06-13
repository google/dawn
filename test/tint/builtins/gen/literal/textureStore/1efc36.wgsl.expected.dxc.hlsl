RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_1efc36() {
  arg_0[(1u).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_1efc36();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_1efc36();
  return;
}
