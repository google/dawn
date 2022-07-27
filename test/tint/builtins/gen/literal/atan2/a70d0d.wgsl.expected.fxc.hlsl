void atan2_a70d0d() {
  float3 res = (0.785398185f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan2_a70d0d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan2_a70d0d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_a70d0d();
  return;
}
