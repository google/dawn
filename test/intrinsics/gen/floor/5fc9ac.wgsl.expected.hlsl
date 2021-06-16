void floor_5fc9ac() {
  float2 res = floor(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  floor_5fc9ac();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  floor_5fc9ac();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  floor_5fc9ac();
  return;
}
