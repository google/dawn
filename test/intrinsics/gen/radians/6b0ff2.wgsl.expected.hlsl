float tint_radians(float param_0) {
  return param_0 * 0.017453292519943295474;
}

void radians_6b0ff2() {
  float res = tint_radians(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_6b0ff2();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_6b0ff2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_6b0ff2();
  return;
}
