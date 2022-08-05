SKIP: FAILED

void mix_38cbbb() {
  float16_t res = lerp(float16_t(0.0h), float16_t(0.0h), float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  mix_38cbbb();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  mix_38cbbb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_38cbbb();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001E08ADE7D50(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001E08ADE7D50(2,13-15): error X3000: unrecognized identifier 'res'

