int tint_extract_bits(int v, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint shl = (32u - e);
  const uint shr = (shl + s);
  const int shl_result = ((shl < 32u) ? (v << shl) : 0);
  return ((shr < 32u) ? (shl_result >> shr) : ((shl_result >> 31u) >> 1u));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void extractBits_249874() {
  int arg_0 = 1;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int res = tint_extract_bits(arg_0, arg_1, arg_2);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  extractBits_249874();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  extractBits_249874();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  extractBits_249874();
  return;
}
