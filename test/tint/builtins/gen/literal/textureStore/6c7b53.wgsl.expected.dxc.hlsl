//
// fragment_main
//

RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_6c7b53() {
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_6c7b53();
}

//
// compute_main
//

RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_6c7b53() {
  arg_0[(int(1)).xxx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_6c7b53();
}

