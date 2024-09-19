
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_b286b4() {
  RWTexture2DArray<uint4> v = arg_0;
  v[int3((int(1)).xx, int(int(1)))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_b286b4();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_b286b4();
}

