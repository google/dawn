SKIP: FAILED

void atan_a5f421() {
  vector<float16_t, 3> res = atan((float16_t(0.0h)).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan_a5f421();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan_a5f421();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan_a5f421();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000186B9618C70(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

