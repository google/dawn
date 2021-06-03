SKIP: FAILED



Validation Failure:
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_22d955() {
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
}

void vertex_main() {
  textureStore_22d955();
  return;
}

void fragment_main() {
  textureStore_22d955();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_22d955();
  return;
}


tint_Qx2ZJJ:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
             ^


tint_Qx2ZJJ:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
             ^


tint_Qx2ZJJ:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
             ^

