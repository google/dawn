
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_9f7cea() {
  RWTexture2DArray<uint4> v = arg_0;
  v[int3((1).xx, int(1u))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_9f7cea();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9f7cea();
}

