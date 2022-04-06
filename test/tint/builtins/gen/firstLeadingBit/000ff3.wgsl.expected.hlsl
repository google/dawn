uint4 tint_first_leading_bit(uint4 v) {
  uint4 x = v;
  const uint4 b16 = (bool4((x & uint4((4294901760u).xxxx))) ? uint4((16u).xxxx) : uint4((0u).xxxx));
  x = (x >> b16);
  const uint4 b8 = (bool4((x & uint4((65280u).xxxx))) ? uint4((8u).xxxx) : uint4((0u).xxxx));
  x = (x >> b8);
  const uint4 b4 = (bool4((x & uint4((240u).xxxx))) ? uint4((4u).xxxx) : uint4((0u).xxxx));
  x = (x >> b4);
  const uint4 b2 = (bool4((x & uint4((12u).xxxx))) ? uint4((2u).xxxx) : uint4((0u).xxxx));
  x = (x >> b2);
  const uint4 b1 = (bool4((x & uint4((2u).xxxx))) ? uint4((1u).xxxx) : uint4((0u).xxxx));
  const uint4 is_zero = ((x == uint4((0u).xxxx)) ? uint4((4294967295u).xxxx) : uint4((0u).xxxx));
  return uint4((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstLeadingBit_000ff3() {
  uint4 res = tint_first_leading_bit(uint4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  firstLeadingBit_000ff3();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  firstLeadingBit_000ff3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  firstLeadingBit_000ff3();
  return;
}
