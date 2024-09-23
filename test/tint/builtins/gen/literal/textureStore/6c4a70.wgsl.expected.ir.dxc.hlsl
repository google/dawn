
RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_6c4a70() {
  arg_0[(1u).xx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_6c4a70();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_6c4a70();
}

