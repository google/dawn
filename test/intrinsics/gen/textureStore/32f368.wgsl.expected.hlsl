SKIP: FAILED



Validation Failure:
RWTexture2DArray<float4> arg_0 : register(u0, space1);

void textureStore_32f368() {
  arg_0[int3(1)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

void vertex_main() {
  textureStore_32f368();
  return;
}

void fragment_main() {
  textureStore_32f368();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_32f368();
  return;
}


tint_CQZOh9:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
             ^


tint_CQZOh9:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
             ^


tint_CQZOh9:4:14: error: too few elements in vector initialization (expected 3 elements, have 1)
  arg_0[int3(1)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
             ^

