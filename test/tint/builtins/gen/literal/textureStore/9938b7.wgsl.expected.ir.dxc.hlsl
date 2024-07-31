
RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_9938b7() {
  RWTexture2DArray<int4> v = arg_0;
  v[int3((1).xx, int(1u))] = (1).xxxx;
}

void fragment_main() {
  textureStore_9938b7();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9938b7();
}

