#version 310 es
precision mediump float;

layout(location = 0) out vec4 out_var_SV_TARGET_1_1;
vec4 out_var_SV_TARGET = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  out_var_SV_TARGET = vec4(0.0f /* inf */);
  return;
}

struct main_out {
  vec4 out_var_SV_TARGET_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(out_var_SV_TARGET);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  out_var_SV_TARGET_1_1 = inner_result.out_var_SV_TARGET_1;
  return;
}
