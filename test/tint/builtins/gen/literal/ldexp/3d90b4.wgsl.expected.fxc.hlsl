SKIP: FAILED

void ldexp_3d90b4() {
  vector<float16_t, 2> res = (float16_t(2.0h)).xx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_3d90b4();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_3d90b4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_3d90b4();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x000001C8EE651B30(2,10-18): error X3000: syntax error: unexpected token 'float16_t'

