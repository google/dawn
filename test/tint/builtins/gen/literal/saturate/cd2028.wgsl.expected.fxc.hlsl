SKIP: FAILED

void saturate_cd2028() {
  vector<float16_t, 2> res = saturate((float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  saturate_cd2028();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  saturate_cd2028();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  saturate_cd2028();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x00000147A2EDE4A0(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

