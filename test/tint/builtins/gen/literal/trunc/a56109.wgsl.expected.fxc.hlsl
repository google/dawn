SKIP: FAILED

void trunc_a56109() {
  vector<float16_t, 2> res = trunc((float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  trunc_a56109();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  trunc_a56109();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_a56109();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000295C5ED1620(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

