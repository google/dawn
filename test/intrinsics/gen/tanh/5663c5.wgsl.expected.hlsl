void tanh_5663c5() {
  float4 res = tanh(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  tanh_5663c5();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  tanh_5663c5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_5663c5();
  return;
}
