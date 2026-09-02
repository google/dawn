//
// fragment_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_9938b7() {
  int v = int(1u);
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_9938b7();
}

//
// compute_main
//

RWTexture2DArray<int4> arg_0 : register(u0, space1);
void textureStore_9938b7() {
  int v = int(1u);
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9938b7();
}

