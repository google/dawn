SKIP: FAILED

void transpose_b9ad1f() {
  matrix<float16_t, 3, 2> arg_0 = matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  matrix<float16_t, 2, 3> res = transpose(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  transpose_b9ad1f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  transpose_b9ad1f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_b9ad1f();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001BE022AEC60(2,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001BE022AEC60(3,10-18): error X3000: syntax error: unexpected token 'float16_t'

