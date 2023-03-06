SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void fract_eb38ce() {
  float16_t arg_0 = float16_t(1.25h);
  float16_t res = frac(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fract_eb38ce();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fract_eb38ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_eb38ce();
  return;
}
