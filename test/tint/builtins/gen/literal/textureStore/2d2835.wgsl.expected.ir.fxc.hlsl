
RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_2d2835() {
  arg_0[uint3((1u).xx, uint(1u))] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_2d2835();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2d2835();
}

