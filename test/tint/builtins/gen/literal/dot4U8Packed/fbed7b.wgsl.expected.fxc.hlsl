SKIP: FAILED

uint tint_dot4U8Packed(uint param_0, uint param_1) {
  uint accumulator = 0u;
  return dot4add_u8packed(param_0, param_1, accumulator);
}

void dot4U8Packed_fbed7b() {
  uint res = tint_dot4U8Packed(1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot4U8Packed_fbed7b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot4U8Packed_fbed7b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot4U8Packed_fbed7b();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001D2469D6CD0(3,10-56): error X3004: undeclared identifier 'dot4add_u8packed'

