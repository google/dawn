//
// fragment_main
//

RWTexture3D<float4> arg_0 : register(u0, space1);
void textureStore_331aee() {
  int3 arg_1 = (int(1)).xxx;
  float4 arg_2 = (1.0f).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_331aee();
}

//
// compute_main
//

RWTexture3D<float4> arg_0 : register(u0, space1);
void textureStore_331aee() {
  int3 arg_1 = (int(1)).xxx;
  float4 arg_2 = (1.0f).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_331aee();
}

