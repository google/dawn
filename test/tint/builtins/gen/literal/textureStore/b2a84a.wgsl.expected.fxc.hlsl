//
// fragment_main
//

RWTexture2D<float4> arg_0 : register(u0, space1);
void textureStore_b2a84a() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_b2a84a();
}

//
// compute_main
//

RWTexture2D<float4> arg_0 : register(u0, space1);
void textureStore_b2a84a() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_b2a84a();
}

