void atan2_ae713e() {
  float4 res = atan2((1.0f).xxxx, (1.0f).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan2_ae713e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan2_ae713e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_ae713e();
  return;
}
