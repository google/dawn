//
// fragment_main
//

RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_df0c51() {
  uint3 arg_1 = (1u).xxx;
  int4 arg_2 = (int(1)).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_df0c51();
}

//
// compute_main
//

RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_df0c51() {
  uint3 arg_1 = (1u).xxx;
  int4 arg_2 = (int(1)).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_df0c51();
}

