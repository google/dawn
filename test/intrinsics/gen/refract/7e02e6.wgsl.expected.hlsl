void refract_7e02e6() {
  float4 res = refract(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  refract_7e02e6();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  refract_7e02e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  refract_7e02e6();
  return;
}
