int tint_dot4_i8_packed(uint a, uint b) {
  int4 a_i8 = (asint((uint4((a).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx);
  int4 b_i8 = (asint((uint4((b).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx);
  return dot(a_i8, b_i8);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4_i8_packed(arg_0, arg_1);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot4I8Packed_881e62();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot4I8Packed_881e62();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot4I8Packed_881e62();
  return;
}
