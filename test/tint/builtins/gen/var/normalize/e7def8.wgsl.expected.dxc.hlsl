void normalize_e7def8() {
  float3 res = (0.57735025882720947266f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  normalize_e7def8();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  normalize_e7def8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_e7def8();
  return;
}
