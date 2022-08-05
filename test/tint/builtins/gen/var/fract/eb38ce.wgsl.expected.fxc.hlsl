SKIP: FAILED

void fract_eb38ce() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t res = frac(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fract_eb38ce();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fract_eb38ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_eb38ce();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000024B27352E20(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000024B27352E20(2,13-17): error X3000: unrecognized identifier 'arg_0'

