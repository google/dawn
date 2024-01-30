uint tint_pack_4xu8_clamp(uint4 a) {
  uint4 a_clamp = clamp(a, (0u).xxxx, (255u).xxxx);
  uint4 a_u8 = uint4((a_clamp << uint4(0u, 8u, 16u, 24u)));
  return dot(a_u8, (1u).xxxx);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void pack4xU8Clamp_6b8c1b() {
  uint4 arg_0 = (1u).xxxx;
  uint res = tint_pack_4xu8_clamp(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pack4xU8Clamp_6b8c1b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pack4xU8Clamp_6b8c1b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack4xU8Clamp_6b8c1b();
  return;
}
