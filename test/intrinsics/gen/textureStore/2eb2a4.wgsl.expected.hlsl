RWTexture1D<uint4> arg_0 : register(u0, space1);

void textureStore_2eb2a4() {
  arg_0[1] = uint4(0u, 0u, 0u, 0u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureStore_2eb2a4();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureStore_2eb2a4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2eb2a4();
  return;
}
