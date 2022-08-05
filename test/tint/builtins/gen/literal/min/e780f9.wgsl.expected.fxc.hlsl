SKIP: FAILED

void min_e780f9() {
  vector<float16_t, 2> res = min((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  min_e780f9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  min_e780f9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_e780f9();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001FEDC6E7DE0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

