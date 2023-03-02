void atan2_034ace() {
  float res = 0.78539818525314331055f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan2_034ace();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan2_034ace();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_034ace();
  return;
}
