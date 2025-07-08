//
// fragment_main
//

RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_f29ce4() {
  arg_0[(1u).xxx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_f29ce4();
}

//
// compute_main
//

RWTexture3D<int4> arg_0 : register(u0, space1);
void textureStore_f29ce4() {
  arg_0[(1u).xxx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_f29ce4();
}

