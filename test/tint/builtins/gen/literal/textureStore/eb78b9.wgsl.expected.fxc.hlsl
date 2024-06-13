RWTexture3D<int4> arg_0 : register(u0, space1);

void textureStore_eb78b9() {
  arg_0[(1).xxx] = (1).xxxx;
}

void fragment_main() {
  textureStore_eb78b9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_eb78b9();
  return;
}
