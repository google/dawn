//
// fragment_main
//

RWTexture3D<uint4> arg_0 : register(u0, space1);
void textureStore_53a68b() {
  int3 arg_1 = (int(1)).xxx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_53a68b();
}

//
// compute_main
//

RWTexture3D<uint4> arg_0 : register(u0, space1);
void textureStore_53a68b() {
  int3 arg_1 = (int(1)).xxx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_53a68b();
}

