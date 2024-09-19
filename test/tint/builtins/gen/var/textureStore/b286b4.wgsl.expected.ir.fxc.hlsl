
RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_b286b4() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint4 arg_3 = (1u).xxxx;
  RWTexture2DArray<uint4> v = arg_0;
  int2 v_1 = arg_1;
  uint4 v_2 = arg_3;
  v[int3(v_1, int(arg_2))] = v_2;
}

void fragment_main() {
  textureStore_b286b4();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_b286b4();
}

