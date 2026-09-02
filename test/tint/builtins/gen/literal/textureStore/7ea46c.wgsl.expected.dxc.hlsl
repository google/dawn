//
// fragment_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_7ea46c() {
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_7ea46c();
}

//
// compute_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_7ea46c() {
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_7ea46c();
}

