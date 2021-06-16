void reflect_f47fdb() {
  float3 res = reflect(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reflect_f47fdb();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reflect_f47fdb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_f47fdb();
  return;
}
