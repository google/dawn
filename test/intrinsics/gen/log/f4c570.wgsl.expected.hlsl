void log_f4c570() {
  float3 res = log(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  log_f4c570();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  log_f4c570();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_f4c570();
  return;
}
