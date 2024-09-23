
RWTexture3D<uint4> arg_0 : register(u0, space1);
void textureStore_ab03b6() {
  arg_0[(int(1)).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_ab03b6();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_ab03b6();
}

