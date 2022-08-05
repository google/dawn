SKIP: FAILED

void tan_d4d491() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t res = tan(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tan_d4d491();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tan_d4d491();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_d4d491();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000026911B52E20(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000026911B52E20(2,13-17): error X3000: unrecognized identifier 'arg_0'

