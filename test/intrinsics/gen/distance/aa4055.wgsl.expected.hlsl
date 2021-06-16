void distance_aa4055() {
  float res = distance(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  distance_aa4055();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  distance_aa4055();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_aa4055();
  return;
}
