int3 tint_extract_bits(int3 v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  return ((v << uint3((shl).xxx)) >> uint3((shr).xxx));
}

void extractBits_e04f5d() {
  int3 res = tint_extract_bits((1).xxx, 1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  extractBits_e04f5d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  extractBits_e04f5d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  extractBits_e04f5d();
  return;
}
