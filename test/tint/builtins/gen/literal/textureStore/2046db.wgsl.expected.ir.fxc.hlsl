
RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_2046db() {
  RWTexture2DArray<float4> v = arg_0;
  v[uint3((1u).xx, uint(int(1)))] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_2046db();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2046db();
}

