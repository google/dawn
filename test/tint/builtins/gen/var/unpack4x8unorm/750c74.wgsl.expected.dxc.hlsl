float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack4x8unorm_750c74() {
  uint arg_0 = 1u;
  float4 res = tint_unpack4x8unorm(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack4x8unorm_750c74();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack4x8unorm_750c74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4x8unorm_750c74();
  return;
}
