SKIP: FAILED

float16_t tint_atanh(float16_t x) {
  return (log(((float16_t(1.0h) + x) / (float16_t(1.0h) - x))) * float16_t(0.5h));
}

void atanh_d2d8cd() {
  float16_t res = tint_atanh(float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_d2d8cd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_d2d8cd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_d2d8cd();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001760B391820(1,1-9): error X3000: unrecognized identifier 'float16_t'

