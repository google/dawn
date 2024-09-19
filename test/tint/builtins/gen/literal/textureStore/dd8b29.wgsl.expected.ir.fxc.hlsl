
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_dd8b29() {
  RWTexture2DArray<uint4> v = arg_0;
  v[int3((int(1)).xx, int(int(1)))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_dd8b29();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_dd8b29();
}

