void tan_7ea104() {
  float3 res = tan(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  tan_7ea104();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  tan_7ea104();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_7ea104();
  return;
}
