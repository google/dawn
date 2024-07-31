
RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_7cec8d() {
  int2 arg_1 = (1).xx;
  int arg_2 = 1;
  int4 arg_3 = (1).xxxx;
  RWTexture2DArray<int4> v = arg_0;
  int2 v_1 = arg_1;
  int4 v_2 = arg_3;
  v[int3(v_1, int(arg_2))] = v_2;
}

void fragment_main() {
  textureStore_7cec8d();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_7cec8d();
}

