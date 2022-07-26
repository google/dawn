float4 tint_degrees(float4 param_0) {
  return param_0 * 57.295779513082322865;
}

void degrees_0d170c() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = tint_degrees(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  degrees_0d170c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  degrees_0d170c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  degrees_0d170c();
  return;
}
