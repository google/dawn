SKIP: FAILED

void max_e14f2b() {
  vector<float16_t, 4> arg_0 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> arg_1 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> res = max(arg_0, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  max_e14f2b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  max_e14f2b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_e14f2b();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000026D512195F0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000026D512195F0(3,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000026D512195F0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'

