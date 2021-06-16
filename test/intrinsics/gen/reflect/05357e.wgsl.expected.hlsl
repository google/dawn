void reflect_05357e() {
  float4 res = reflect(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reflect_05357e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reflect_05357e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_05357e();
  return;
}
