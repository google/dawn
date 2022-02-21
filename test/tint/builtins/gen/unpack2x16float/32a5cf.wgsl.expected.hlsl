float2 tint_unpack2x16float(uint param_0) {
  uint i = param_0;
  return f16tof32(uint2(i & 0xffff, i >> 16));
}

void unpack2x16float_32a5cf() {
  float2 res = tint_unpack2x16float(1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack2x16float_32a5cf();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack2x16float_32a5cf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16float_32a5cf();
  return;
}
