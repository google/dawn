void tan_ae26ae() {
  float3 res = (1.557407737f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tan_ae26ae();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tan_ae26ae();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_ae26ae();
  return;
}
