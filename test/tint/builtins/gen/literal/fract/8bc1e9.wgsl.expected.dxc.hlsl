RWByteAddressBuffer prevent_dce : register(u0, space2);

void fract_8bc1e9() {
  float4 res = (0.25f).xxxx;
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fract_8bc1e9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fract_8bc1e9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_8bc1e9();
  return;
}
