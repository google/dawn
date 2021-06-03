SKIP: FAILED



Validation Failure:
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_dde364() {
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
}

void vertex_main() {
  textureStore_dde364();
  return;
}

void fragment_main() {
  textureStore_dde364();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_dde364();
  return;
}


tint_0wYTiw:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
             ^


tint_0wYTiw:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
             ^


tint_0wYTiw:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = uint4(0u, 0u, 0u, 0u);
             ^

