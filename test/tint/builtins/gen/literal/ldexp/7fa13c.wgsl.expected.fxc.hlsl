SKIP: FAILED

void ldexp_7fa13c() {
  vector<float16_t, 4> res = ldexp((float16_t(0.0h)).xxxx, (1).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_7fa13c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_7fa13c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_7fa13c();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000018D79877AB0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

