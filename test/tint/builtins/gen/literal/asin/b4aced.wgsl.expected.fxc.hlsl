SKIP: FAILED

void asin_b4aced() {
  vector<float16_t, 2> res = asin((float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  asin_b4aced();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  asin_b4aced();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_b4aced();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000138E6B7E9E0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

