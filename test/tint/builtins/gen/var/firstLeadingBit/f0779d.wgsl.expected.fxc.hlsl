uint tint_first_leading_bit(uint v) {
  uint x = v;
  const uint b16 = (bool((x & 4294901760u)) ? 16u : 0u);
  x = (x >> b16);
  const uint b8 = (bool((x & 65280u)) ? 8u : 0u);
  x = (x >> b8);
  const uint b4 = (bool((x & 240u)) ? 4u : 0u);
  x = (x >> b4);
  const uint b2 = (bool((x & 12u)) ? 2u : 0u);
  x = (x >> b2);
  const uint b1 = (bool((x & 2u)) ? 1u : 0u);
  const uint is_zero = ((x == 0u) ? 4294967295u : 0u);
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint res = tint_first_leading_bit(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  firstLeadingBit_f0779d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  firstLeadingBit_f0779d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  firstLeadingBit_f0779d();
  return;
}
