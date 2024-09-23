
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_158cf0() {
  arg_0[uint3((1u).xx, uint(1u))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_158cf0();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_158cf0();
}

