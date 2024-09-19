
RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_b706b1() {
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_b706b1();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_b706b1();
}

