//
// fragment_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_f9f90e() {
  arg_0[uint3((1u).xx, uint(int(1)))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_f9f90e();
}

//
// compute_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_f9f90e() {
  arg_0[uint3((1u).xx, uint(int(1)))] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_f9f90e();
}

