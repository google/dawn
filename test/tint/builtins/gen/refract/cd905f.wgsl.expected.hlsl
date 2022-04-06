void refract_cd905f() {
  float2 res = refract(float2(0.0f, 0.0f), float2(0.0f, 0.0f), 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  refract_cd905f();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  refract_cd905f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  refract_cd905f();
  return;
}
