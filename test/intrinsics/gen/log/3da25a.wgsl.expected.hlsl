void log_3da25a() {
  float4 res = log(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  log_3da25a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  log_3da25a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_3da25a();
  return;
}
