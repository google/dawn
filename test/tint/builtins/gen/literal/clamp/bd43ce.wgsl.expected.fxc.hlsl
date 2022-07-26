void clamp_bd43ce() {
  uint4 res = clamp((1u).xxxx, (1u).xxxx, (1u).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  clamp_bd43ce();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  clamp_bd43ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_bd43ce();
  return;
}
