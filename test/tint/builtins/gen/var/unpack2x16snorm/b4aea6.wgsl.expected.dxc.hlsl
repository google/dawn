float2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  float2 res = tint_unpack2x16snorm(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack2x16snorm_b4aea6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack2x16snorm_b4aea6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16snorm_b4aea6();
  return;
}
