SKIP: FAILED

void transpose_7be8b2() {
  matrix<float16_t, 2, 2> arg_0 = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  matrix<float16_t, 2, 2> res = transpose(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_7be8b2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_7be8b2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_7be8b2();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000015E28C70990(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x0000015E28C70990(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

