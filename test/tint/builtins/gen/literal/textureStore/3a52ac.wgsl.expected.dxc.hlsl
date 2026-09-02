//
// fragment_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_3a52ac() {
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_3a52ac();
}

//
// compute_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_3a52ac() {
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_3a52ac();
}

