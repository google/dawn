void tanh_5724b3() {
  float2 res = tanh(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  tanh_5724b3();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  tanh_5724b3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_5724b3();
  return;
}
