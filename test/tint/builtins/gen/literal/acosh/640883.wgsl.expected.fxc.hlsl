float2 tint_acosh(float2 x) {
  return log((x + sqrt(((x * x) - 1.0f))));
}

void acosh_640883() {
  float2 res = tint_acosh((1.0f).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acosh_640883();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acosh_640883();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acosh_640883();
  return;
}
