void mix_275cac() {
  float4 res = (1.0f).xxxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  mix_275cac();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  mix_275cac();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_275cac();
  return;
}
