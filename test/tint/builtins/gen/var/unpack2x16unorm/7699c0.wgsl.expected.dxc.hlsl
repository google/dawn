float2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack2x16unorm_7699c0() {
  uint arg_0 = 1u;
  float2 res = tint_unpack2x16unorm(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack2x16unorm_7699c0();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack2x16unorm_7699c0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16unorm_7699c0();
  return;
}
