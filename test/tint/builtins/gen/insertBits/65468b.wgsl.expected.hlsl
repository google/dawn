int tint_insert_bits(int v, int n, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
  return (((n << s) & int(mask)) | (v & int(~(mask))));
}

void insertBits_65468b() {
  int res = tint_insert_bits(1, 1, 1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_65468b();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_65468b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_65468b();
  return;
}
