SKIP: FAILED

void trunc_cc2b0d() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t res = trunc(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  trunc_cc2b0d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  trunc_cc2b0d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_cc2b0d();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000023FD3F982D0(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000023FD3F982D0(2,13-17): error X3000: unrecognized identifier 'arg_0'

