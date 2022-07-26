int2 tint_insert_bits(int2 v, int2 n, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
  return (((n << uint2((s).xx)) & int2((int(mask)).xx)) | (v & int2((int(~(mask))).xx)));
}

void insertBits_fe6ba6() {
  int2 res = tint_insert_bits((1).xx, (1).xx, 1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_fe6ba6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_fe6ba6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_fe6ba6();
  return;
}
