int2 tint_extract_bits(int2 v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  return ((v << uint2((shl).xx)) >> uint2((shr).xx));
}

void extractBits_a99a8d() {
  int2 res = tint_extract_bits((1).xx, 1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  extractBits_a99a8d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  extractBits_a99a8d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  extractBits_a99a8d();
  return;
}
