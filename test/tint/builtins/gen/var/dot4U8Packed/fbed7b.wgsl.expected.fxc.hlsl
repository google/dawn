uint tint_dot4_u8_packed(uint a, uint b) {
  uint4 a_u8 = ((uint4((a).xxxx) >> uint4(24u, 16u, 8u, 0u)) & (255u).xxxx);
  uint4 b_u8 = ((uint4((b).xxxx) >> uint4(24u, 16u, 8u, 0u)) & (255u).xxxx);
  return dot(a_u8, b_u8);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint res = tint_dot4_u8_packed(arg_0, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot4U8Packed_fbed7b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot4U8Packed_fbed7b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot4U8Packed_fbed7b();
  return;
}
