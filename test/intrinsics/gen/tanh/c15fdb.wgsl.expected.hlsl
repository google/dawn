void tanh_c15fdb() {
  float res = tanh(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  tanh_c15fdb();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  tanh_c15fdb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_c15fdb();
  return;
}
