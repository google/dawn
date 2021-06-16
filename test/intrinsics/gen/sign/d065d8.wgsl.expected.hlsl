void sign_d065d8() {
  float2 res = sign(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sign_d065d8();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sign_d065d8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_d065d8();
  return;
}
