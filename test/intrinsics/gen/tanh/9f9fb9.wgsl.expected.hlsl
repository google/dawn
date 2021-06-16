void tanh_9f9fb9() {
  float3 res = tanh(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  tanh_9f9fb9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  tanh_9f9fb9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_9f9fb9();
  return;
}
