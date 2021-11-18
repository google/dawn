#version 310 es
precision mediump float;

vec4 out_var_SV_TARGET = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  out_var_SV_TARGET = vec4(uintBitsToFloat(0x7fc00000u), uintBitsToFloat(0x7fc00000u), uintBitsToFloat(0x7fc00000u), uintBitsToFloat(0x7fc00000u));
  return;
}

struct main_out {
  vec4 out_var_SV_TARGET_1;
};
struct tint_symbol_1 {
  vec4 out_var_SV_TARGET_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(out_var_SV_TARGET);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.out_var_SV_TARGET_1 = inner_result.out_var_SV_TARGET_1;
  return wrapper_result;
}
out vec4 out_var_SV_TARGET_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  out_var_SV_TARGET_1 = outputs.out_var_SV_TARGET_1;
}


