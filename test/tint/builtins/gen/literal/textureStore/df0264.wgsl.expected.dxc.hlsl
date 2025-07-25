//
// fragment_main
//

RWTexture1D<float4> arg_0 : register(u0, space1);
void textureStore_df0264() {
  arg_0[int(1)] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_df0264();
}

//
// compute_main
//

RWTexture1D<float4> arg_0 : register(u0, space1);
void textureStore_df0264() {
  arg_0[int(1)] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_df0264();
}

