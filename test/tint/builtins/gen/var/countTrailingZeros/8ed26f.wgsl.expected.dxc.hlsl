uint3 tint_count_trailing_zeros(uint3 v) {
  uint3 x = uint3(v);
  const uint3 b16 = (bool3((x & (65535u).xxx)) ? (0u).xxx : (16u).xxx);
  x = (x >> b16);
  const uint3 b8 = (bool3((x & (255u).xxx)) ? (0u).xxx : (8u).xxx);
  x = (x >> b8);
  const uint3 b4 = (bool3((x & (15u).xxx)) ? (0u).xxx : (4u).xxx);
  x = (x >> b4);
  const uint3 b2 = (bool3((x & (3u).xxx)) ? (0u).xxx : (2u).xxx);
  x = (x >> b2);
  const uint3 b1 = (bool3((x & (1u).xxx)) ? (0u).xxx : (1u).xxx);
  const uint3 is_zero = ((x == (0u).xxx) ? (1u).xxx : (0u).xxx);
  return uint3((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void countTrailingZeros_8ed26f() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = tint_count_trailing_zeros(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  countTrailingZeros_8ed26f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  countTrailingZeros_8ed26f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countTrailingZeros_8ed26f();
  return;
}
