RWByteAddressBuffer prevent_dce : register(u0, space2);

void countLeadingZeros_f70103() {
  uint4 res = (31u).xxxx;
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  countLeadingZeros_f70103();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  countLeadingZeros_f70103();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countLeadingZeros_f70103();
  return;
}
