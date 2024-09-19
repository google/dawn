
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_22d955() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint4 arg_3 = (1u).xxxx;
  int2 v = arg_1;
  uint4 v_1 = arg_3;
  arg_0[int3(v, int(arg_2))] = v_1;
}

void fragment_main() {
  textureStore_22d955();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_22d955();
}

