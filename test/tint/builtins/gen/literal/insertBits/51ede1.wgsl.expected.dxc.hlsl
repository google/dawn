uint4 tint_insert_bits(uint4 v, uint4 n, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
  return (((n << uint4((s).xxxx)) & uint4((mask).xxxx)) | (v & uint4((~(mask)).xxxx)));
}

void insertBits_51ede1() {
  uint4 res = tint_insert_bits((1u).xxxx, (1u).xxxx, 1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_51ede1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_51ede1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_51ede1();
  return;
}
