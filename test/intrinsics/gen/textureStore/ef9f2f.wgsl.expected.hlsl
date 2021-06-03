RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_ef9f2f() {
  arg_0[int3(0, 0, 0)] = uint4(0u, 0u, 0u, 0u);
}

void vertex_main() {
  textureStore_ef9f2f();
  return;
}

void fragment_main() {
  textureStore_ef9f2f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_ef9f2f();
  return;
}

