void length_7b4741() {
  float res = 0.0f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  length_7b4741();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  length_7b4741();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_7b4741();
  return;
}
