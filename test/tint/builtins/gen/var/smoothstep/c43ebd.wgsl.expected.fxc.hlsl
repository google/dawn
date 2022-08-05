SKIP: FAILED

void smoothstep_c43ebd() {
  vector<float16_t, 4> arg_0 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> arg_1 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> arg_2 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> res = smoothstep(arg_0, arg_1, arg_2);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  smoothstep_c43ebd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  smoothstep_c43ebd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_c43ebd();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001EE5BEB1170(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001EE5BEB1170(3,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001EE5BEB1170(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001EE5BEB1170(5,10-18): error X3000: syntax error: unexpected token 'float16_t'

