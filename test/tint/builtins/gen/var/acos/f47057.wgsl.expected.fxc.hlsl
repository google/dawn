SKIP: FAILED

void acos_f47057() {
  vector<float16_t, 3> arg_0 = (float16_t(0.0h)).xxx;
  vector<float16_t, 3> res = acos(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acos_f47057();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acos_f47057();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_f47057();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000287C9AECA00(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000287C9AECA00(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

