//
// fragment_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_28ff45() {
  arg_0[(int(1)).xxx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_28ff45();
}

//
// compute_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_28ff45() {
  arg_0[(int(1)).xxx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_28ff45();
}

