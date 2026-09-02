//
// fragment_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_c32905() {
  uint v = uint(int(1));
  arg_0[(1u).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_c32905();
}

//
// compute_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_c32905() {
  uint v = uint(int(1));
  arg_0[(1u).xxx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_c32905();
}

