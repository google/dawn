//
// fragment_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_fd0b2d() {
  uint v = uint(int(1));
  arg_0[(1u).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_fd0b2d();
}

//
// compute_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_fd0b2d() {
  uint v = uint(int(1));
  arg_0[(1u).xxx] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_fd0b2d();
}

