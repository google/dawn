SKIP: FAILED

void step_cc6b61() {
  vector<float16_t, 3> res = step((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  step_cc6b61();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  step_cc6b61();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  step_cc6b61();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000015E30CB9FC0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

