SKIP: FAILED

void dot_d0d179() {
  float16_t res = dot((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot_d0d179();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot_d0d179();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_d0d179();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001E77FD80D40(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001E77FD80D40(2,13-15): error X3000: unrecognized identifier 'res'

