
RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_88ce7e() {
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_88ce7e();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_88ce7e();
}

