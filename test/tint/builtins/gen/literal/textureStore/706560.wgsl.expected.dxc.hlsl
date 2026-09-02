//
// fragment_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_706560() {
  int v = int(1u);
  arg_0[(int(1)).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_706560();
}

//
// compute_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_706560() {
  int v = int(1u);
  arg_0[(int(1)).xxx] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_706560();
}

