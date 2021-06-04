struct tint_symbol {
  float4 value : SV_Position;
};

void ceil_678655() {
  float res = ceil(1.0f);
}

tint_symbol vertex_main() {
  ceil_678655();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ceil_678655();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_678655();
  return;
}

