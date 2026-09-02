//
// fragment_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_3773f7() {
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_3773f7();
}

//
// compute_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_3773f7() {
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_3773f7();
}

