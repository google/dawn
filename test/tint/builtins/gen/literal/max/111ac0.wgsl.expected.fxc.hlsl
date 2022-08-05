SKIP: FAILED

void max_111ac0() {
  float16_t res = max(float16_t(0.0h), float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  max_111ac0();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  max_111ac0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_111ac0();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000215BA37C140(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000215BA37C140(2,13-15): error X3000: unrecognized identifier 'res'

