
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_8a16b0() {
  RWTexture2DArray<uint4> v = arg_0;
  v[int3((1).xx, int(1))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_8a16b0();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_8a16b0();
}

