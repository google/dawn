float2 tint_atanh(float2 x) {
  return (log(((1.0f + x) / (1.0f - x))) * 0.5f);
}

void atanh_c0e634() {
  float2 res = tint_atanh((1.0f).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_c0e634();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_c0e634();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_c0e634();
  return;
}
