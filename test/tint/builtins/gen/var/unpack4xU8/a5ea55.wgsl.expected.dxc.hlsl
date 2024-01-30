RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint4 res = unpack_u8u32(uint8_t4_packed(arg_0));
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
