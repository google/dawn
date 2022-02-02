void mix_1faeb1() {
  float4 res = lerp(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  mix_1faeb1();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  mix_1faeb1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_1faeb1();
  return;
}
