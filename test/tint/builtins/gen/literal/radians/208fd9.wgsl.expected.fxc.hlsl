SKIP: FAILED

float16_t tint_radians(float16_t param_0) {
  return param_0 * 0.017453292519943295474;
}

void radians_208fd9() {
  float16_t res = tint_radians(float16_t(0.0h));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_208fd9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_208fd9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_208fd9();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001B5647B0440(1,1-9): error X3000: unrecognized identifier 'float16_t'

