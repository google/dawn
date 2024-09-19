
RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_59a0ab() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  float4 arg_3 = (1.0f).xxxx;
  RWTexture2DArray<float4> v = arg_0;
  uint2 v_1 = arg_1;
  float4 v_2 = arg_3;
  v[uint3(v_1, uint(arg_2))] = v_2;
}

void fragment_main() {
  textureStore_59a0ab();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_59a0ab();
}

