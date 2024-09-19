
RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_30b0b0() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  int4 arg_3 = (int(1)).xxxx;
  RWTexture2DArray<int4> v = arg_0;
  int2 v_1 = arg_1;
  int4 v_2 = arg_3;
  v[int3(v_1, int(arg_2))] = v_2;
}

void fragment_main() {
  textureStore_30b0b0();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_30b0b0();
}

