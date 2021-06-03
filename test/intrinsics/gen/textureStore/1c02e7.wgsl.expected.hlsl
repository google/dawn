SKIP: FAILED



Validation Failure:
RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_1c02e7() {
  arg_0[int3(1)] = int4(0, 0, 0, 0);
}

void vertex_main() {
  textureStore_1c02e7();
  return;
}

void fragment_main() {
  textureStore_1c02e7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_1c02e7();
  return;
}


tint_9SEdo4:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = int4(0, 0, 0, 0);
             ^


tint_9SEdo4:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = int4(0, 0, 0, 0);
             ^


tint_9SEdo4:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = int4(0, 0, 0, 0);
             ^

