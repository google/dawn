uint4 tint_unpack_4xu8(uint a) {
  uint4 a_vec4u = (uint4((a).xxxx) >> uint4(0u, 8u, 16u, 24u));
  return (a_vec4u & (255u).xxxx);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint4 res = tint_unpack_4xu8(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack4xU8_a5ea55();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack4xU8_a5ea55();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4xU8_a5ea55();
  return;
}
