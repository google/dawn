uint2 tint_first_leading_bit(uint2 v) {
  uint2 x = v;
  const uint2 b16 = (bool2((x & (4294901760u).xx)) ? (16u).xx : (0u).xx);
  x = (x >> b16);
  const uint2 b8 = (bool2((x & (65280u).xx)) ? (8u).xx : (0u).xx);
  x = (x >> b8);
  const uint2 b4 = (bool2((x & (240u).xx)) ? (4u).xx : (0u).xx);
  x = (x >> b4);
  const uint2 b2 = (bool2((x & (12u).xx)) ? (2u).xx : (0u).xx);
  x = (x >> b2);
  const uint2 b1 = (bool2((x & (2u).xx)) ? (1u).xx : (0u).xx);
  const uint2 is_zero = ((x == (0u).xx) ? (4294967295u).xx : (0u).xx);
  return uint2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void firstLeadingBit_6fe804() {
  uint2 arg_0 = (1u).xx;
  uint2 res = tint_first_leading_bit(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  firstLeadingBit_6fe804();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  firstLeadingBit_6fe804();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  firstLeadingBit_6fe804();
  return;
}
