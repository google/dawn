//
// fragment_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_602dfa() {
  arg_0[int(1)] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_602dfa();
}

//
// compute_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_602dfa() {
  arg_0[int(1)] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_602dfa();
}

