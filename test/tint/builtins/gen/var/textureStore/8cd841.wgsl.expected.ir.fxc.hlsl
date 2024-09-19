
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_8cd841() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint4 arg_3 = (1u).xxxx;
  RWTexture2DArray<uint4> v = arg_0;
  int2 v_1 = arg_1;
  uint4 v_2 = arg_3;
  v[int3(v_1, int(arg_2))] = v_2;
}

void fragment_main() {
  textureStore_8cd841();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_8cd841();
}

