int4 tint_extract_bits(int4 v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  return ((v << uint4((shl).xxxx)) >> uint4((shr).xxxx));
}

void extractBits_fb850f() {
  int4 arg_0 = (1).xxxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int4 res = tint_extract_bits(arg_0, arg_1, arg_2);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  extractBits_fb850f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  extractBits_fb850f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  extractBits_fb850f();
  return;
}
