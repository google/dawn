//
// fragment_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_da530c() {
  uint2 arg_1 = (1u).xx;
  int4 arg_2 = (int(1)).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_da530c();
}

//
// compute_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_da530c() {
  uint2 arg_1 = (1u).xx;
  int4 arg_2 = (int(1)).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_da530c();
}

