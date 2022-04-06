uint2 tint_first_trailing_bit(uint2 v) {
  uint2 x = uint2(v);
  const uint2 b16 = (bool2((x & uint2((65535u).xx))) ? uint2((0u).xx) : uint2((16u).xx));
  x = (x >> b16);
  const uint2 b8 = (bool2((x & uint2((255u).xx))) ? uint2((0u).xx) : uint2((8u).xx));
  x = (x >> b8);
  const uint2 b4 = (bool2((x & uint2((15u).xx))) ? uint2((0u).xx) : uint2((4u).xx));
  x = (x >> b4);
  const uint2 b2 = (bool2((x & uint2((3u).xx))) ? uint2((0u).xx) : uint2((2u).xx));
  x = (x >> b2);
  const uint2 b1 = (bool2((x & uint2((1u).xx))) ? uint2((0u).xx) : uint2((1u).xx));
  const uint2 is_zero = ((x == uint2((0u).xx)) ? uint2((4294967295u).xx) : uint2((0u).xx));
  return uint2((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

void firstTrailingBit_45eb10() {
  uint2 res = tint_first_trailing_bit(uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  firstTrailingBit_45eb10();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  firstTrailingBit_45eb10();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  firstTrailingBit_45eb10();
  return;
}
