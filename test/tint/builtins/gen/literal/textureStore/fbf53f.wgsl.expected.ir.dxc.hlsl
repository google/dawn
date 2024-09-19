
RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_fbf53f() {
  RWTexture2DArray<int4> v = arg_0;
  v[int3((int(1)).xx, int(int(1)))] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_fbf53f();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_fbf53f();
}

