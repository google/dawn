//
// fragment_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_45be8e() {
  uint v = uint(int(1));
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_45be8e();
}

//
// compute_main
//

RWTexture2DArray<float4> arg_0 : register(u0, space1);
void textureStore_45be8e() {
  uint v = uint(int(1));
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_45be8e();
}

