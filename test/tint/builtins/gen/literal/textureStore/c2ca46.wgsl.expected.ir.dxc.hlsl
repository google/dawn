
RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_c2ca46() {
  arg_0[uint3((1u).xx, uint(1u))] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_c2ca46();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_c2ca46();
}

