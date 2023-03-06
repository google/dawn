uint tint_count_leading_zeros(uint v) {
  uint x = uint(v);
  const uint b16 = ((x <= 65535u) ? 16u : 0u);
  x = (x << b16);
  const uint b8 = ((x <= 16777215u) ? 8u : 0u);
  x = (x << b8);
  const uint b4 = ((x <= 268435455u) ? 4u : 0u);
  x = (x << b4);
  const uint b2 = ((x <= 1073741823u) ? 2u : 0u);
  x = (x << b2);
  const uint b1 = ((x <= 2147483647u) ? 1u : 0u);
  const uint is_zero = ((x == 0u) ? 1u : 0u);
  return uint((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint res = tint_count_leading_zeros(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  countLeadingZeros_208d46();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  countLeadingZeros_208d46();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countLeadingZeros_208d46();
  return;
}
