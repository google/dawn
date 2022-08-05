SKIP: FAILED

void sqrt_d9ab4d() {
  vector<float16_t, 2> res = sqrt((float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sqrt_d9ab4d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sqrt_d9ab4d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_d9ab4d();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000256F2C5EAF0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

