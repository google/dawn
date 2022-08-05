SKIP: FAILED

void select_a081f1() {
  vector<float16_t, 4> res = ((true).xxxx ? (float16_t(0.0h)).xxxx : (float16_t(0.0h)).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_a081f1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_a081f1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_a081f1();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000202B4A94A10(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

